#include "cpu.h"
#include "../platform/platform.h"
#include "../utils/error.h"
#include <signal.h>

// Global variables for storing previous CPU statistics
static unsigned long prev_cpu_usage[7] = {0};
static int first_run = 1;

/**
 * Function to collect CPU information and send through pipe
 * @param pipe_fd Pipe file descriptor array
 */
void storeCPUInfo(int pipe_fd[2]) {
    unsigned long cpu_usage[7] = {0};
    
    // Get CPU statistics using platform-independent function
    get_cpu_stats(cpu_usage);
    
    // Debug output: Log CPU statistics values
    printf("CPU Raw Data: User=%lu Nice=%lu System=%lu Idle=%lu\n",
           cpu_usage[0], cpu_usage[1], cpu_usage[2], cpu_usage[3]);
    
    // Send information through pipe
    ssize_t bytes_written = write(pipe_fd[1], &cpu_usage, sizeof(cpu_usage));
    if (bytes_written == -1) {
        perror("Error writing to pipe from storeCPUInfo");
        kill(getpid(), SIGTERM);
        kill(getppid(), SIGTERM);
        close(pipe_fd[1]);
        exit(EXIT_FAILURE);
    }
    
    printf("CPU statistics data transmitted: %zd bytes\n", bytes_written);
}

/**
 * Calculate CPU usage
 * @param prevCpuUsage Previous CPU state
 * @param currCpuUsage Current CPU state
 * @return CPU usage percentage
 */
double calculateCPUUsage(unsigned long prevCpuUsage[7], unsigned long currCpuUsage[7]) {
    // Check if this is the first run
    if (first_run) {
        // Store previous values
        for (int i = 0; i < 7; i++) {
            prev_cpu_usage[i] = prevCpuUsage[i];
        }
        first_run = 0;
        
        // On initial execution, the difference between current and previous values is minimal
        // Calculate using only current values (usage vs idle)
        unsigned long total = currCpuUsage[0] + currCpuUsage[1] + 
                            currCpuUsage[2] + currCpuUsage[3];
        
        if (total == 0) return 0.0;
        
        double idle_percent = (double)currCpuUsage[3] * 100.0 / total;
        double cpu_usage = 100.0 - idle_percent;
        
        printf("First CPU calculation (direct method): %.2f%%\n", cpu_usage);
        return cpu_usage;
    }
    
    // Previous CPU times
    unsigned long prevUser = prevCpuUsage[0];
    unsigned long prevNice = prevCpuUsage[1];
    unsigned long prevSystem = prevCpuUsage[2];
    unsigned long prevIdle = prevCpuUsage[3];
    
    // Current CPU times
    unsigned long currUser = currCpuUsage[0];
    unsigned long currNice = currCpuUsage[1];
    unsigned long currSystem = currCpuUsage[2];
    unsigned long currIdle = currCpuUsage[3];
    
    // Calculate current total CPU time
    unsigned long currTotal = currUser + currNice + currSystem + currIdle;
    
    // Debug: print raw values
    printf("Previous CPU: User=%lu, Nice=%lu, System=%lu, Idle=%lu\n", 
           prevUser, prevNice, prevSystem, prevIdle);
    printf("Current CPU: User=%lu, Nice=%lu, System=%lu, Idle=%lu\n", 
           currUser, currNice, currSystem, currIdle);
    
    // Previous and current total CPU time
    unsigned long prevTotal = prevUser + prevNice + prevSystem + prevIdle;
    (void)prevTotal; // 의도적으로 사용하지 않음을 표시
    
    // Calculate CPU time differences
    unsigned long userDiff = 0;
    unsigned long niceDiff = 0;
    unsigned long systemDiff = 0;
    unsigned long idleDiff = 0;
    
    // Check if current value is greater than or equal to previous value (rare cases of inversion)
    if (currUser >= prevUser) userDiff = currUser - prevUser;
    if (currNice >= prevNice) niceDiff = currNice - prevNice;
    if (currSystem >= prevSystem) systemDiff = currSystem - prevSystem;
    if (currIdle >= prevIdle) idleDiff = currIdle - prevIdle;
    
    // Total CPU time difference over interval
    unsigned long totalDiff = userDiff + niceDiff + systemDiff + idleDiff;
    
    // Debug: print differences
    printf("CPU Differences: User=%lu, System=%lu, Idle=%lu, Total=%lu\n",
           userDiff, systemDiff, idleDiff, totalDiff);
    
    // Prevent division by zero
    if (totalDiff == 0) {
        printf("No CPU time difference. Using direct calculation.\n");
        
        // Calculate percentage using current CPU statistics (calculate idle ratio to total, then subtract from 100%)
        if (currTotal == 0) return 0.0;
        
        double idle_percent = (double)currIdle * 100.0 / currTotal;
        double cpu_usage = 100.0 - idle_percent;
        
        printf("Directly calculated CPU usage: %.2f%%\n", cpu_usage);
        return cpu_usage;
    }
    
    // Calculate actual CPU usage (non-idle time / total time)
    double nonIdlePercent = (double)(totalDiff - idleDiff) * 100.0 / totalDiff;
    
    // Range validation (0-100%)
    if (nonIdlePercent < 0.0) {
        printf("Warning: CPU usage is negative (%.2f%%). Adjusting to 0%%.\n", nonIdlePercent);
        nonIdlePercent = 0.0;
    } else if (nonIdlePercent > 100.0) {
        printf("Warning: CPU usage exceeds 100%% (%.2f%%). Adjusting to 100%%.\n", nonIdlePercent);
        nonIdlePercent = 100.0;
    }
    
    // Update previous values (for next calculation)
    for (int i = 0; i < 7; i++) {
        prev_cpu_usage[i] = currCpuUsage[i];
    }
    
    printf("Final CPU usage: %.2f%%\n", nonIdlePercent);
    return nonIdlePercent;
}

/**
 * Print CPU core count
 */
void printCPUCores(void) {
    int num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of CPU cores: %d\n", num_cpu);
}

/**
 * Set CPU usage graphics function
 * @param sequential Whether sequential mode is enabled
 * @param cpuArr CPU graphics array
 * @param curCpuUsage Current CPU usage
 * @param prevCpuUsage Pointer to previous CPU usage
 * @param sampleIndex Current sample index
 */
void setCPUGraphics(int sequential, char cpuArr[][MAX_CPU_BUFFER], float curCpuUsage, float *prevCpuUsage, int sampleIndex) {
    int default_num = 3; // Default number of bars
    int additionalBars; // Additional bars based on CPU usage
    
    // If this is the first sample
    if (sampleIndex == 0) {
        additionalBars = (int)curCpuUsage; // Additional bars based on current CPU usage
    } else {
        // Additional bars based on difference between previous and current CPU usage
        additionalBars = (int)curCpuUsage - (int)(*prevCpuUsage);
    }

    // Initialize string to hold CPU usage information
    char cpuUsageStr[MAX_CPU_BUFFER] = "         "; // Include initial spacing
    default_num += additionalBars; // Update total number of bars
    
    // Add bars
    for (int i = 0; i < default_num && i < (MAX_CPU_BUFFER - 50); i++) {
        strcat(cpuUsageStr, "|");
    }

    // Add CPU usage percentage
    char usagePercent[50];
    snprintf(usagePercent, sizeof(usagePercent), " %.2f%%", curCpuUsage);
    strcat(cpuUsageStr, usagePercent);

    // Store completed string in array
    strncpy(cpuArr[sampleIndex], cpuUsageStr, MAX_CPU_BUFFER - 1);
    cpuArr[sampleIndex][MAX_CPU_BUFFER - 1] = '\0'; // Ensure string is properly terminated

    // Adjust output based on sequential mode
    if (sequential) {
        // In sequential mode, print CPU usage info up to current index
        for (int j = 0; j <= sampleIndex; j++) {
            printf("%s\n", cpuArr[j]);
        }
    } else {
        // In non-sequential mode, print all CPU usage info up to current index
        for (int k = 0; k <= sampleIndex; k++) {
            printf("%s\n", cpuArr[k]);
        }
    }

    // Update previous CPU usage with current usage
    *prevCpuUsage = curCpuUsage;
} 