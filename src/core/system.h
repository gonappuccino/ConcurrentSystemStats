#ifndef SYSTEM_H
#define SYSTEM_H

#include "common.h"

/**
 * System information output function
 * 
 * Collects and outputs basic system information including OS name, version, hostname, 
 * architecture, and uptime.
 */
void printSystemInfo(void);

/**
 * System uptime information collection function
 * 
 * Calculates elapsed time since system boot in days, hours, minutes, and seconds.
 * This information is used for system information output and GUI display.
 * 
 * @param days Pointer to store uptime days
 * @param hours Pointer to store uptime hours
 * @param minutes Pointer to store uptime minutes
 * @param seconds Pointer to store uptime seconds
 */
void getSystemUptimeInfo(int *days, int *hours, int *minutes, int *seconds);

// Top information output function
void printTopInfo(int samples, int tdelay, int sequential, int iteration);

/**
 * Child process creation function
 * 
 * Creates child processes to collect CPU, memory, and user information.
 * Each child process collects specific information and sends it back to the parent process through pipes.
 * 
 * @param samples Number of samples to collect
 * @param tdelay Sampling interval (seconds)
 * @param pipes Pointer to pipe structure
 * @return Process ID structure of created processes
 */
ProcessIDs createChildProcesses(int samples, int tdelay, PipeSet *pipes);

// Signal handler setup function
void setupSignalHandlers(void);

// Program options parsing function
ProgramOptions parseCommandLineOptions(int argc, char *argv[]);

#endif // SYSTEM_H 