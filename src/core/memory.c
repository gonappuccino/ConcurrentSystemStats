#include "memory.h"
#include "../platform/platform.h"
#include <signal.h>
#include <math.h>

/**
 * Function to collect memory information and send through pipe
 * @param tdelay Sampling interval (seconds)
 * @param samples Number of samples to collect
 * @param memFD Pipe file descriptor for sending memory information
 */
void storeMemoryInfo(int tdelay, int samples, int *memFD) {
    for (int i = 0; i < samples; i++) {
        char memBuffer[MAX_MEMORY_BUFFER] = {0};
        struct sysinfo sys_info;
        
        if (sysinfo(&sys_info) != 0) {
            perror("Error getting system info");
            snprintf(memBuffer, sizeof(memBuffer), "Error getting system info");
        } else {
            double phys_total_gb = (double)sys_info.totalram / (1024 * 1024 * 1024); 
            double phys_free_gb = (double)sys_info.freeram / (1024 * 1024 * 1024);
            double phys_used_gb = phys_total_gb - phys_free_gb;
            double swap_total_gb = (double)sys_info.totalswap / (1024 * 1024 * 1024);
            double swap_free_gb = (double)sys_info.freeswap / (1024 * 1024 * 1024);

            double virtual_used_gb = phys_used_gb + (swap_total_gb - swap_free_gb);
            double virtual_total_gb = phys_total_gb + swap_total_gb;
            
            snprintf(memBuffer, sizeof(memBuffer), "%.2f GB / %.2f GB  -- %.2f GB / %.2f GB",
                    phys_used_gb, phys_total_gb, virtual_used_gb, virtual_total_gb);
        }

        // Safe pipe writing - including null terminator
        size_t len = strlen(memBuffer) + 1;
        
        // Send buffer length first
        if (write(memFD[1], &len, sizeof(len)) == -1) {
            perror("Error writing length to pipe");
            kill(getpid(), SIGTERM);
            return;
        }
        
        // Then send buffer contents
        if (write(memFD[1], memBuffer, len) == -1) {
            perror("Error writing data to pipe");
            kill(getpid(), SIGTERM);
            return;
        }

        sleep(tdelay);
    }
}

/**
 * Memory information output function
 * @param sequential Whether sequential mode is enabled
 * @param samples Number of samples to collect
 * @param memArr Memory information array
 * @param iteration Current iteration index
 * @param memFD Memory pipe file descriptor (unused)
 */
void printMemoryInfo(int sequential, int samples, char memArr[][MAX_MEMORY_BUFFER], int iteration, int memFD[2]) {
    (void)memFD; // Suppress unused parameter warning
    
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    
    if (sequential) {
        for (int k = 0; k < samples; k++) {
            if (k == iteration) {
                printf("%s\n", memArr[k]);
            } else {
                printf("\n");
            }
        }
    } else {
        for (int j = 0; j <= iteration; j++) {
            printf("%s\n", memArr[j]);
        }
    }
}

/**
 * Memory graphics creation function
 * @param virtual_used_gb Virtual memory usage (GB)
 * @param prev_used_gb Pointer to previous virtual memory usage (GB)
 * @param memArr Memory information array
 * @param iteration Current iteration index
 */
void createMemoryGraphics(double virtual_used_gb, double *prev_used_gb, char memArr[][MAX_MEMORY_BUFFER], int iteration) {
    double difference = virtual_used_gb - *prev_used_gb;
    char graphicsStr[MAX_MEMORY_BUFFER] = "\0 ";
    char infoStr[100]; // Buffer for formatted information
    
    // Default representation for first sample or minimal change
    if (iteration == 0 || fabs(difference) < 0.01) {
        snprintf(graphicsStr, sizeof(graphicsStr), "|%s %.2f (%.2f)", 
                difference >= 0 ? "o" : "@", difference, virtual_used_gb);
    } else {
        // Prepare graphics based on magnitude and direction of change
        char changeSymbol = difference < 0 ? ':' : '#';
        int symbolsCount = (int)(fabs(difference) * 100); // Convert change to symbol count

        // Add base bar
        strcat(graphicsStr, "|");
        // Add additional symbols based on change magnitude
        size_t remaining = sizeof(graphicsStr) - strlen(graphicsStr) - 50;
        for (int j = 0; j < symbolsCount && (size_t)j < remaining; ++j) {
            strncat(graphicsStr, &changeSymbol, 1);
        }
        // Add closing symbol
        strcat(graphicsStr, difference < 0 ? "@" : "*");
        
        // Format and add difference and usage
        snprintf(infoStr, sizeof(infoStr), " %.2f (%.2f)", difference, virtual_used_gb);
        strncat(graphicsStr, infoStr, sizeof(graphicsStr) - strlen(graphicsStr) - 1);
    }
    
    // Ensure buffer limits are not exceeded
    strcat(memArr[iteration], graphicsStr);
    memArr[iteration][MAX_MEMORY_BUFFER - 1] = '\0'; // Ensure string is properly terminated

    // Update previous usage for next call
    *prev_used_gb = virtual_used_gb;
}

/**
 * Virtual memory usage calculation function
 * @return Virtual memory usage (GB)
 */
double getVirtualMemoryUsage(void) {
    return calculate_memory_usage();
}

/**
 * Reserve space function
 * @param samples Number of lines to reserve
 */
void reserveSpace(int samples) {
    for (int i = 0; i < samples + 1; i++) {
        printf("\n");
    }
} 