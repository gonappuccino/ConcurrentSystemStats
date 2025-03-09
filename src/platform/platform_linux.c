#ifndef __APPLE__

#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

/**
 * CPU statistics collection function
 * @param cpu_usage Array to store CPU usage
 */
void get_cpu_stats(unsigned long cpu_usage[7]) {
    FILE *fp = fopen("/proc/stat", "r");
    
    if (!fp) {
        memset(cpu_usage, 0, 7 * sizeof(unsigned long));
        return;
    }
    
    if (fscanf(fp, "cpu %lu %lu %lu %lu %lu %lu %lu", 
              &cpu_usage[0], &cpu_usage[1], &cpu_usage[2], &cpu_usage[3], 
              &cpu_usage[4], &cpu_usage[5], &cpu_usage[6]) != 7) {
        memset(cpu_usage, 0, 7 * sizeof(unsigned long));
    }
    
    fclose(fp);
}

/**
 * Memory usage calculation function
 * @return Virtual memory usage (GB)
 */
double calculate_memory_usage(void) {
    struct sysinfo sys_info;
    
    if (sysinfo(&sys_info) != 0) {
        return 0.0;
    }
    
    double phys_total_gb = (double)sys_info.totalram / (1024 * 1024 * 1024); 
    double phys_free_gb = (double)sys_info.freeram / (1024 * 1024 * 1024);
    double phys_used_gb = phys_total_gb - phys_free_gb;
    double swap_total_gb = (double)sys_info.totalswap / (1024 * 1024 * 1024);
    double swap_free_gb = (double)sys_info.freeswap / (1024 * 1024 * 1024);
    
    double virtual_used_gb = phys_used_gb + (swap_total_gb - swap_free_gb);
    
    return virtual_used_gb;
}

/**
 * Get system uptime information
 * @param days Pointer to store days
 * @param hours Pointer to store hours
 * @param minutes Pointer to store minutes
 * @param seconds Pointer to store seconds
 */
void get_system_uptime(int *days, int *hours, int *minutes, int *seconds) {
    FILE *uptime_file;
    double uptime_secs = 0.0;
    
    uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file != NULL) {
        fscanf(uptime_file, "%lf", &uptime_secs);
        fclose(uptime_file);
    }
    
    // Convert to days, hours, minutes, seconds
    *days = uptime_secs / (24 * 3600);
    uptime_secs = uptime_secs - (*days * 24 * 3600);
    *hours = uptime_secs / 3600;
    uptime_secs = uptime_secs - (*hours * 3600);
    *minutes = uptime_secs / 60;
    *seconds = (int)uptime_secs % 60;
}

#endif // !__APPLE__ 