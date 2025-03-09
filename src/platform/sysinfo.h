#ifndef SYSINFO_H
#define SYSINFO_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <sys/mount.h>
#include <string.h>

// Linux sysinfo structure compatibility for macOS
struct sysinfo {
    long uptime;             /* Time in seconds since boot */
    unsigned long loads[3];  /* 1, 5, 15 minute load averages */
    unsigned long totalram;  /* Total usable main memory size */
    unsigned long freeram;   /* Available memory size */
    unsigned long sharedram; /* Amount of shared memory */
    unsigned long bufferram; /* Memory used by buffers */
    unsigned long totalswap; /* Total swap space size */
    unsigned long freeswap;  /* Available swap space */
    unsigned short procs;    /* Number of current processes */
    unsigned long totalhigh; /* Total high memory size */
    unsigned long freehigh;  /* Available high memory size */
    unsigned int mem_unit;   /* Memory unit size in bytes */
    char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
};

// Function to get system information on macOS (changed to static inline)
static inline int sysinfo(struct sysinfo *info) {
    if (info == NULL) {
        return -1;
    }
    
    // Initialize structure
    memset(info, 0, sizeof(struct sysinfo));
    
    // Get system boot time
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    
    if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
        return -1;
    }
    
    time_t now = time(NULL);
    info->uptime = now - boottime.tv_sec;
    
    // Get memory information
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
    
    // Get total physical memory
    int64_t physical_memory;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    len = sizeof(physical_memory);
    if (sysctl(mib, 2, &physical_memory, &len, NULL, 0) < 0) {
        return -1;
    }
    
    // Set values in sysinfo structure
    info->totalram = physical_memory;
    info->freeram = vm_stats.free_count * page_size;
    
    // Get swap information (represented by xsw_usage structure on macOS)
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
    
    // Set memory unit (standard is bytes)
    info->mem_unit = 1;
    
    return 0;
}

#endif // SYSINFO_H 