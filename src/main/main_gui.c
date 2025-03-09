#include "common.h"
#include "gui.h"

/**
 * Function to handle data collection and pipe communication
 * @param pipe_fd Pipe file descriptor array
 */
void dataCollectionProcess(int pipe_fd[2]) {
    // Close read end of pipe in this process
    close(pipe_fd[0]);
    
    // Initialize data collection
    printf("Data collection process started (PID: %d)\n", getpid());
    
    // Calculate initial delay to allow GUI to initialize
    struct timespec initialDelay = {0};
    initialDelay.tv_sec = 1;  // 1 second initial delay
    nanosleep(&initialDelay, NULL);
    
    // Data collection loop
    while (1) {
        // Collect and send CPU data
        printf("Collecting CPU statistics...\n");
        storeCPUInfo(pipe_fd);
        
        // Collect and send memory data
        // ... existing memory data collection ...
        
        // Sleep between updates to reduce CPU usage
        struct timespec sleepTime = {0};
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 500000000;  // 500ms
        nanosleep(&sleepTime, NULL);
    }
    
    // Close write end of pipe when done (never reached in this implementation)
    close(pipe_fd[1]);
}

int main(int argc, char *argv[]) {
    int pipe_fd[2];
    pid_t childPid;
    
    // Create pipe for process communication
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Main GUI process started (PID: %d)\n", getpid());
    
    // Initialize error handling system
    init_error_handling();
    LOG_INFO(SYS_MON_SUCCESS, "GUI System monitor starting");
    
    // Create child process for data collection
    childPid = fork();
    
    if (childPid == -1) {
        LOG_FATAL(SYS_MON_ERR_FORK, "Failed to create data collection process");
        exit(EXIT_FAILURE);
    }
    
    if (childPid == 0) {
        // Child process - handles data collection
        dataCollectionProcess(pipe_fd);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process - handles GUI
        // Close write end of pipe in parent process
        close(pipe_fd[1]);
        
        // Initialize GTK for GUI
        gtk_init(&argc, &argv);
        
        // Create GUI window
        createGUI(pipe_fd);
        
        // Start GTK main loop
        gtk_main();
        
        // Cleanup
        close(pipe_fd[0]);
    }
    
    return 0;
} 