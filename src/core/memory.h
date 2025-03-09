#ifndef MEMORY_H
#define MEMORY_H

#include "../utils/common.h"

/**
 * Memory information collection and storage function
 * 
 * Collects current system memory usage information and sends it through a pipe.
 * This function is called from a child process to periodically monitor memory status.
 * 
 * @param tdelay Sampling interval (seconds)
 * @param samples Number of samples to collect
 * @param memFD Pipe file descriptor for sending memory information
 */
void storeMemoryInfo(int tdelay, int samples, int *memFD);

// Memory information output function
void printMemoryInfo(int sequential, int samples, char memArr[][MAX_MEMORY_BUFFER], int iteration, int memFD[2]);

// Memory graphics creation function
void createMemoryGraphics(double virtual_used_gb, double *prev_used_gb, char memArr[][MAX_MEMORY_BUFFER], int iteration);

/**
 * Virtual memory usage return function
 * 
 * Calculates and returns the current system's virtual memory usage in GB.
 * 
 * @return Virtual memory usage (GB)
 */
double getVirtualMemoryUsage(void);

// Reserve empty space function
void reserveSpace(int samples);

/**
 * Physical memory usage return function
 * 
 * Calculates and returns the current system's physical memory usage in GB.
 * 
 * @return Physical memory usage (GB)
 */
double getPhysicalMemoryUsage(void);

/**
 * Total memory capacity return function
 * 
 * Calculates and returns the system's total memory capacity in GB.
 * 
 * @return Total memory capacity (GB)
 */
double getTotalMemory(void);

#endif // MEMORY_H 