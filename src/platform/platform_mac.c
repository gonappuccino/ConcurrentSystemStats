#include "platform.h"
#include "common.h"

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <sys/proc_info.h>
#include <sys/types.h>
#include <mach/processor_info.h>
#include <mach/mach_types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

// Store previous CPU data for delta calculation
static host_cpu_load_info_data_t prev_cpu_load;
static int prev_cpu_load_initialized = 0;

/**
 * Implementation of sysinfo function (compatible with Linux's sysinfo)
 * 
 * Provides the same functionality as Linux's sysinfo function on macOS.
 * Collects system uptime, memory information, swap information, etc.
 * 
 * @param info Pointer to sysinfo structure
 * @return 0 on success, -1 on failure
 */
int sysinfo(struct sysinfo *info) {
    if (info == NULL) {
        return -1;
    }
    
    int64_t time_now;
    
    // Initialize the structure
    memset(info, 0, sizeof(struct sysinfo));
    
    // Get uptime information
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    
    if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
        perror("sysctl failed");
        return -1;
    }
    
    time_now = time(NULL);
    
    // Calculate uptime
    info->uptime = time_now - boottime.tv_sec;
    
    // Get load averages
    double load_avg[3];
    if (getloadavg(load_avg, 3) < 0) {
        perror("getloadavg failed");
        return -1;
    }
    
    // Convert to Linux format (load averages are 1-min, 5-min, 15-min)
    info->loads[0] = (unsigned long)(load_avg[0] * 65536.0);
    info->loads[1] = (unsigned long)(load_avg[1] * 65536.0);
    info->loads[2] = (unsigned long)(load_avg[2] * 65536.0);
    
    // Get VM statistics
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    kern_return_t ret;
    
    ret = host_statistics64(mach_host_self(), HOST_VM_INFO64, 
                          (host_info64_t)&vm_stats, &count);
    if (ret != KERN_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mach_error_string(ret));
        return -1;
    }
    
    // Get page size
    int mib_page[2] = { CTL_HW, HW_PAGESIZE };
    int64_t pagesize;
    len = sizeof(pagesize);
    
    if (sysctl(mib_page, 2, &pagesize, &len, NULL, 0) < 0) {
        perror("sysctl pagesize failed");
        return -1;
    }
    
    // Get physical memory size
    int mib_mem[2] = { CTL_HW, HW_MEMSIZE };
    uint64_t physical_memory;
    len = sizeof(physical_memory);
    
    if (sysctl(mib_mem, 2, &physical_memory, &len, NULL, 0) < 0) {
        perror("sysctl memsize failed");
        return -1;
    }
    
    // Get swap information
    struct xsw_usage swap_usage;
    len = sizeof(swap_usage);
    
    int mib_swap[2] = { CTL_VM, VM_SWAPUSAGE };
    
    if (sysctl(mib_swap, 2, &swap_usage, &len, NULL, 0) < 0) {
        perror("sysctl swapusage failed");
        return -1;
    }
    
    // Fill in the sysinfo structure
    info->totalram = physical_memory;
    info->freeram = vm_stats.free_count * pagesize;
    info->totalswap = swap_usage.xsu_total;
    info->freeswap = swap_usage.xsu_avail;
    info->mem_unit = 1;
    
    return 0;
}

/**
 * CPU usage information collection function
 * 
 * Collects CPU usage information on macOS and converts it to a Linux-compatible format.
 * Uses delta calculation between current and previous readings to get accurate usage.
 * 
 * @param cpu_usage Array to store the collected CPU usage values
 */
void get_cpu_stats(unsigned long cpu_usage[7]) {
    host_cpu_load_info_data_t cpu_load;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    
    // Initialize the output array
    memset(cpu_usage, 0, 7 * sizeof(unsigned long));
    
    // Get CPU statistics
    kern_return_t ret = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, 
                                      (host_info_t)&cpu_load, &count);
    
    if (ret != KERN_SUCCESS) {
        fprintf(stderr, "Error getting CPU stats: %s\n", mach_error_string(ret));
        return;
    }
    
    // If this is the first call, initialize the previous values and return zeros
    if (!prev_cpu_load_initialized) {
        memcpy(&prev_cpu_load, &cpu_load, sizeof(host_cpu_load_info_data_t));
        prev_cpu_load_initialized = 1;
        
        // Set initial values - small non-zero values to avoid division by zero
        cpu_usage[0] = 10; // USER
        cpu_usage[1] = 1;  // NICE
        cpu_usage[2] = 1;  // SYSTEM
        cpu_usage[3] = 100; // IDLE (to make initial CPU usage low)
        cpu_usage[4] = 0;  // IOWAIT (not available on macOS)
        cpu_usage[5] = 0;  // IRQ (not available on macOS)
        cpu_usage[6] = 0;  // SOFTIRQ (not available on macOS)
        
        printf("First CPU reading - initialized baseline values\n");
        return;
    }
    
    // Calculate deltas for each state
    unsigned long user_diff = cpu_load.cpu_ticks[CPU_STATE_USER] - prev_cpu_load.cpu_ticks[CPU_STATE_USER];
    unsigned long nice_diff = cpu_load.cpu_ticks[CPU_STATE_NICE] - prev_cpu_load.cpu_ticks[CPU_STATE_NICE];
    unsigned long system_diff = cpu_load.cpu_ticks[CPU_STATE_SYSTEM] - prev_cpu_load.cpu_ticks[CPU_STATE_SYSTEM];
    unsigned long idle_diff = cpu_load.cpu_ticks[CPU_STATE_IDLE] - prev_cpu_load.cpu_ticks[CPU_STATE_IDLE];
    
    // Calculate total difference
    unsigned long total_diff = user_diff + nice_diff + system_diff + idle_diff;
    
    // If no time has passed between calls, use the current values directly
    if (total_diff == 0) {
        printf("Warning: No CPU time difference detected\n");
        // Set some reasonable values to avoid division by zero
        total_diff = 100;
        idle_diff = 95; // Assume 5% usage when no difference
    }
    
    // Update the previous values for next call
    memcpy(&prev_cpu_load, &cpu_load, sizeof(host_cpu_load_info_data_t));
    
    // Fill the output array with delta values
    cpu_usage[0] = user_diff;    // USER
    cpu_usage[1] = nice_diff;    // NICE
    cpu_usage[2] = system_diff;  // SYSTEM
    cpu_usage[3] = idle_diff;    // IDLE
    cpu_usage[4] = 0;            // IOWAIT (not available on macOS)
    cpu_usage[5] = 0;            // IRQ (not available on macOS)
    cpu_usage[6] = 0;            // SOFTIRQ (not available on macOS)
    
    // Print debug information
    double user_pct = (double)user_diff * 100.0 / total_diff;
    double system_pct = (double)system_diff * 100.0 / total_diff;
    double idle_pct = (double)idle_diff * 100.0 / total_diff;
    
    printf("CPU Usage - User: %.1f%%, System: %.1f%%, Idle: %.1f%%\n", 
           user_pct, system_pct, idle_pct);
    printf("Raw values - User: %lu, System: %lu, Idle: %lu, Total: %lu\n",
           user_diff, system_diff, idle_diff, total_diff);
}

/**
 * System uptime calculation function
 * 
 * Calculates the system uptime on macOS and converts it to days, hours, minutes, and seconds.
 * 
 * @param days Pointer to store the number of days
 * @param hours Pointer to store the number of hours
 * @param minutes Pointer to store the number of minutes
 * @param seconds Pointer to store the number of seconds
 */
void get_system_uptime(int *days, int *hours, int *minutes, int *seconds) {
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    
    if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
        // Handle error - set all values to 0
        *days = 0;
        *hours = 0;
        *minutes = 0;
        *seconds = 0;
        return;
    }
    
    time_t now = time(NULL);
    time_t uptime = now - boottime.tv_sec;
    
    // Convert to days, hours, minutes, seconds
    *days = uptime / (24 * 3600);
    uptime = uptime % (24 * 3600);
    *hours = uptime / 3600;
    uptime = uptime % 3600;
    *minutes = uptime / 60;
    *seconds = uptime % 60;
}

/**
 * Current memory usage calculation function
 * 
 * Calculates the current memory usage on macOS in GB.
 * 
 * @return Memory usage in GB
 */
double calculate_memory_usage(void) {
    // Get VM statistics
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    kern_return_t ret;
    
    ret = host_statistics64(mach_host_self(), HOST_VM_INFO64, 
                          (host_info64_t)&vm_stats, &count);
    if (ret != KERN_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mach_error_string(ret));
        return 0.0;
    }
    
    // Get page size
    int mib[2] = { CTL_HW, HW_PAGESIZE };
    int64_t pagesize;
    size_t len = sizeof(pagesize);
    
    if (sysctl(mib, 2, &pagesize, &len, NULL, 0) < 0) {
        perror("sysctl pagesize failed");
        return 0.0;
    }
    
    // Calculate used memory (Active + Wired)
    uint64_t used_memory = (vm_stats.active_count + 
                          vm_stats.wire_count) * pagesize;
    
    // Convert to GB
    double used_memory_gb = (double)used_memory / (1024 * 1024 * 1024);
    
    return used_memory_gb;
}

/**
 * Total memory capacity calculation function
 * 
 * Calculates the total memory capacity on macOS in GB.
 * 
 * @return Total memory capacity in GB
 */
double calculate_memory_total(void) {
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    int64_t physical_memory;
    size_t len = sizeof(physical_memory);
    
    if (sysctl(mib, 2, &physical_memory, &len, NULL, 0) < 0) {
        perror("sysctl memsize failed");
        return 0.0;
    }
    
    // Convert to GB
    double total_memory_gb = (double)physical_memory / (1024 * 1024 * 1024);
    
    return total_memory_gb;
}

/**
 * Swap usage calculation function
 * 
 * Calculates the swap memory usage on macOS in GB.
 * 
 * @return Swap usage in GB
 */
double calculate_swap_usage(void) {
    struct xsw_usage swap_usage;
    size_t len = sizeof(swap_usage);
    
    int mib[2] = { CTL_VM, VM_SWAPUSAGE };
    
    if (sysctl(mib, 2, &swap_usage, &len, NULL, 0) < 0) {
        perror("sysctl swapusage failed");
        return 0.0;
    }
    
    // Calculate used swap (total - available)
    uint64_t used_swap = swap_usage.xsu_total - swap_usage.xsu_avail;
    
    // Convert to GB
    double used_swap_gb = (double)used_swap / (1024 * 1024 * 1024);
    
    return used_swap_gb;
}

/**
 * Total swap capacity calculation function
 * 
 * Calculates the total swap memory capacity on macOS in GB.
 * 
 * @return Total swap capacity in GB
 */
double calculate_swap_total(void) {
    struct xsw_usage swap_usage;
    size_t len = sizeof(swap_usage);
    
    int mib[2] = { CTL_VM, VM_SWAPUSAGE };
    
    if (sysctl(mib, 2, &swap_usage, &len, NULL, 0) < 0) {
        perror("sysctl swapusage failed");
        return 0.0;
    }
    
    // Convert to GB
    double total_swap_gb = (double)swap_usage.xsu_total / (1024 * 1024 * 1024);
    
    return total_swap_gb;
}

#endif // __APPLE__ 