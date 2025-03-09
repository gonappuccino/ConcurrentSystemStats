#ifndef COMMON_H
#define COMMON_H

// Include standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

// Platform-specific headers (include platform headers first to use cpu_stats_t definition)
#ifdef __APPLE__
    #include "platform.h" // Platform compatibility layer
    #include <utmpx.h>
    #include <mach/mach.h>
    #include <mach/mach_host.h>
#else
    #include <sys/sysinfo.h>
    #include <utmp.h>
#endif

// Constants definition
#define MAX_USER_INPUT 32       // Maximum user input length
#define MAX_MEMORY_BUFFER 1024  // Maximum memory buffer size
#define MAX_CPU_BUFFER 1024     // Maximum CPU buffer size
#define MAX_USER_BUFFER 4096    // Maximum user buffer size
#define DEFAULT_SAMPLES 10      // Default number of samples
#define DEFAULT_DELAY 1         // Default delay in seconds

/**
 * Program options structure
 * Stores user options passed from command line.
 */
typedef struct {
    int samples;     // Number of samples
    int tdelay;      // Delay in seconds
    int user;        // Whether to display user information
    int system;      // Whether to display system information
    int sequential;  // Whether to use sequential mode
    int graphics;    // Whether to display graphics
} ProgramOptions;

/**
 * Pipe structure
 * Manages pipes used for inter-process communication.
 */
typedef struct {
    int cpuPFD[2];   // CPU previous state pipe
    int cpuCFD[2];   // CPU current state pipe
    int userFD[2];   // User information pipe
    int memFD[2];    // Memory information pipe
    int ucountFD[2]; // User count pipe
} PipeSet;

/**
 * Process ID structure
 * Stores PIDs for each child process.
 */
typedef struct {
    pid_t memPID;    // Memory information collection process ID
    pid_t userPID;   // User information collection process ID
    pid_t cpuPID;    // CPU information collection process ID
} ProcessIDs;

/**
 * CPU usage data structure
 * Stores data needed for CPU usage calculation.
 */
typedef struct {
    unsigned long prevUsage[7];   // Previous CPU usage
    unsigned long currUsage[7];   // Current CPU usage
    float currentPercentage;      // Current CPU usage percentage
    float prevPercentage;         // Previous CPU usage percentage
} CPUData;

/**
 * Memory usage data structure
 * Stores memory usage information.
 */
typedef struct {
    double virtualUsedGB;         // Virtual memory usage in GB
    double prevUsedGB;            // Previous memory usage in GB
    char memoryArray[][MAX_MEMORY_BUFFER];  // Memory information storage array
} MemoryData;

/**
 * Status code enumeration
 * Defines various status codes during program execution.
 */
enum StatusCode {
    STATUS_SUCCESS = 0,        // Success
    STATUS_ERROR_PIPE = 1,     // Pipe error
    STATUS_ERROR_FORK = 2,     // Fork error
    STATUS_ERROR_SIGNAL = 3,   // Signal error
    STATUS_ERROR_MEMORY = 4    // Memory error
};

// External global variable declaration
extern volatile sig_atomic_t exit_flag;  // Exit flag

// Function declarations
void setupSignalHandlers(void);  // Signal handler setup function
ProgramOptions parseCommandLineOptions(int argc, char *argv[]);  // Command line options parsing function
void printTopInfo(int samples, int tdelay, int sequential, int count);  // Top information output function
void reserveSpace(int samples);  // Output space reservation function
double getVirtualMemoryUsage(void);  // Virtual memory usage calculation function
void createMemoryGraphics(double virtual_used_gb, double *prev_used_gb, char memArr[][MAX_MEMORY_BUFFER], int idx);  // Memory graphics creation function
void printMemoryInfo(int sequential, int samples, char memArr[][MAX_MEMORY_BUFFER], int idx, int *memFD);  // Memory information output function
void printUserInfo(int *userFD);  // User information output function
void printCPUCores(void);  // CPU core information output function
void setCPUGraphics(int sequential, char cpuArr[][MAX_CPU_BUFFER], float cur_cpuUsage, float *prevCpuUsageFloat, int i);  // CPU graphics setting function

#endif // COMMON_H 