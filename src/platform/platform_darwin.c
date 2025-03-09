#ifdef __APPLE__

#include "platform.h"
#include <string.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <sys/mount.h>

/**
 * sysinfo 함수 구현 - Linux의 sysinfo와 호환
 * @param info 시스템 정보를 저장할 구조체 포인터
 * @return 성공 시 0, 실패 시 -1
 */
int sysinfo(struct sysinfo *info) {
    if (info == NULL) {
        return -1;
    }
    
    // 구조체 초기화
    memset(info, 0, sizeof(struct sysinfo));
    
    // 시스템 가동 시간 가져오기
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    
    if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
        return -1;
    }
    
    time_t now = time(NULL);
    info->uptime = now - boottime.tv_sec;
    
    // 메모리 정보 가져오기
    vm_size_t page_size;
    mach_port_t mach_port = mach_host_self();
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);
    
    if (host_page_size(mach_port, &page_size) != KERN_SUCCESS) {
        return -1;
    }
    
    if (host_statistics64(mach_port, HOST_VM_INFO64, (host_info64_t)&vm_stats, &count) != KERN_SUCCESS) {
        return -1;
    }
    
    // 총 물리 메모리 가져오기
    int64_t physical_memory;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    len = sizeof(physical_memory);
    if (sysctl(mib, 2, &physical_memory, &len, NULL, 0) < 0) {
        return -1;
    }
    
    // sysinfo 구조체에 값 설정
    info->totalram = physical_memory;
    info->freeram = vm_stats.free_count * page_size;
    
    // 스왑 정보 가져오기 (macOS에서는 xsw_usage 구조체로 표현)
    struct xsw_usage swap_usage;
    len = sizeof(swap_usage);
    mib[0] = CTL_VM;
    mib[1] = VM_SWAPUSAGE;
    if (sysctl(mib, 2, &swap_usage, &len, NULL, 0) < 0) {
        info->totalswap = 0;
        info->freeswap = 0;
    } else {
        info->totalswap = swap_usage.xsu_total;
        info->freeswap = swap_usage.xsu_avail;
    }
    
    // 메모리 단위 설정 (표준은 바이트)
    info->mem_unit = 1;
    
    return 0;
}

/**
 * CPU 통계 정보 수집 함수
 * @param cpu_usage CPU 사용량을 저장할 배열
 */
void get_cpu_stats(unsigned long cpu_usage[7]) {
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS) {
        // macOS의 CPU 통계를 Linux 형식으로 변환
        cpu_usage[0] = cpuinfo.cpu_ticks[CPU_STATE_USER];     // user
        cpu_usage[1] = cpuinfo.cpu_ticks[CPU_STATE_NICE];     // nice
        cpu_usage[2] = cpuinfo.cpu_ticks[CPU_STATE_SYSTEM];   // system
        cpu_usage[3] = cpuinfo.cpu_ticks[CPU_STATE_IDLE];     // idle
        cpu_usage[4] = 0;  // iowait (macOS에 없음)
        cpu_usage[5] = 0;  // irq (macOS에 없음)
        cpu_usage[6] = 0;  // softirq (macOS에 없음)
    } else {
        memset(cpu_usage, 0, 7 * sizeof(unsigned long));
    }
}

/**
 * 메모리 사용량 계산 함수
 * @return 가상 메모리 사용량 (GB)
 */
double calculate_memory_usage(void) {
    // 물리 메모리 정보 가져오기
    int64_t physical_memory;
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    size_t len = sizeof(physical_memory);
    
    if (sysctl(mib, 2, &physical_memory, &len, NULL, 0) < 0) {
        return 0.0;
    }
    
    // 메모리 통계 가져오기
    vm_size_t page_size;
    mach_port_t mach_port = mach_host_self();
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);
    
    if (host_page_size(mach_port, &page_size) != KERN_SUCCESS) {
        return 0.0;
    }
    
    if (host_statistics64(mach_port, HOST_VM_INFO64, (host_info64_t)&vm_stats, &count) != KERN_SUCCESS) {
        return 0.0;
    }
    
    // 물리 메모리 사용량 계산
    double phys_total_gb = (double)physical_memory / (1024 * 1024 * 1024);
    double phys_free_gb = (double)(vm_stats.free_count * page_size) / (1024 * 1024 * 1024);
    double phys_used_gb = phys_total_gb - phys_free_gb;
    
    // 스왑 정보 가져오기
    struct xsw_usage swap_usage;
    len = sizeof(swap_usage);
    mib[0] = CTL_VM;
    mib[1] = VM_SWAPUSAGE;
    
    double swap_used_gb = 0.0;
    if (sysctl(mib, 2, &swap_usage, &len, NULL, 0) == 0) {
        swap_used_gb = (double)(swap_usage.xsu_total - swap_usage.xsu_avail) / (1024 * 1024 * 1024);
    }
    
    // 가상 메모리 사용량 = 물리 메모리 사용량 + 스왑 사용량
    double virtual_used_gb = phys_used_gb + swap_used_gb;
    
    return virtual_used_gb;
}

/**
 * 시스템 가동 시간 정보 가져오기
 * @param days 일 수를 저장할 포인터
 * @param hours 시간을 저장할 포인터
 * @param minutes 분을 저장할 포인터
 * @param seconds 초를 저장할 포인터
 */
void get_system_uptime(int *days, int *hours, int *minutes, int *seconds) {
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    
    if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
        *days = *hours = *minutes = *seconds = 0;
        return;
    }
    
    time_t now = time(NULL);
    double uptime_secs = difftime(now, boottime.tv_sec);
    
    // 일, 시간, 분, 초로 변환
    *days = uptime_secs / (24 * 3600);
    uptime_secs = uptime_secs - (*days * 24 * 3600);
    *hours = uptime_secs / 3600;
    uptime_secs = uptime_secs - (*hours * 3600);
    *minutes = uptime_secs / 60;
    *seconds = (int)uptime_secs % 60;
}

#endif // __APPLE__ 