#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

/**
 * CPU state information structure
 * 
 * Structure that stores various CPU state information.
 * This structure is used to calculate CPU usage in platform-specific implementations.
 */
typedef struct {
    unsigned long user;    // User mode CPU time
    unsigned long system;  // System mode CPU time
    unsigned long idle;    // Idle CPU time
    unsigned long nice;    // Nice (low priority) CPU time
} cpu_stats_t;

/**
 * Memory usage calculation function
 * 
 * Calculates memory usage in GB for the current platform.
 * 
 * @return Memory usage (GB)
 */
double calculate_memory_usage(void);

/**
 * Total memory capacity calculation function
 * 
 * Calculates total memory capacity in GB for the current platform.
 * 
 * @return Total memory capacity (GB)
 */
double calculate_memory_total(void);

/**
 * Swap usage calculation function
 * 
 * Calculates swap memory usage in GB for the current platform.
 * 
 * @return Swap usage (GB)
 */
double calculate_swap_usage(void);

/**
 * Total swap capacity calculation function
 * 
 * Calculates total swap memory capacity in GB for the current platform.
 * 
 * @return Total swap capacity (GB)
 */
double calculate_swap_total(void);

/**
 * CPU state information collection function
 * 
 * Collects CPU state information for the current platform.
 * 
 * @param cpu_usage Array to store CPU usage information
 */
void get_cpu_stats(unsigned long cpu_usage[7]);

/**
 * System uptime information collection function
 * 
 * Collects system uptime on the current platform.
 * 
 * @param days Pointer to store uptime days
 * @param hours Pointer to store uptime hours
 * @param minutes Pointer to store uptime minutes
 * @param seconds Pointer to store uptime seconds
 */
void get_system_uptime(int *days, int *hours, int *minutes, int *seconds);

/**
 * Structure compatible with Linux's sysinfo structure
 * 
 * Provides a unified system information interface across different platforms.
 * Defined in the same format as Linux's sysinfo structure.
 */
struct sysinfo {
    long uptime;             /* System uptime in seconds */
    unsigned long loads[3];  /* 1, 5, 15 minute load averages */
    unsigned long totalram;  /* Total usable main memory */
    unsigned long freeram;   /* Available memory */
    unsigned long sharedram; /* Amount of shared memory */
    unsigned long bufferram; /* Memory used by buffers */
    unsigned long totalswap; /* Total swap space */
    unsigned long freeswap;  /* Swap space available */
    unsigned short procs;    /* Number of processes */
    unsigned long totalhigh; /* High memory size */
    unsigned long freehigh;  /* Available high memory */
    unsigned int mem_unit;   /* Memory unit size in bytes */
};

/**
 * Function compatible with Linux's sysinfo function
 * 
 * Provides a unified system information interface across different platforms.
 * Offers the same interface as Linux's sysinfo function.
 * 
 * @param info Pointer to sysinfo structure
 * @return 0 on success, -1 on failure
 */
int sysinfo(struct sysinfo *info);

#endif // PLATFORM_H 