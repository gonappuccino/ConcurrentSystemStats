#include "error.h"
#include <time.h>

// Global variables
int g_verbose_mode = 0;  // Default: not verbose
FILE* g_log_file = NULL; // Default: no log file

// Error level strings for display
static const char* error_level_strings[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

// Error code strings for display
static const char* error_code_strings[] = {
    "SUCCESS",
    "MEMORY",
    "IO",
    "PIPE",
    "FORK",
    "SIGNAL",
    "SYSTEM",
    "PARAMETER",
    "GTK",
    "PLATFORM",
    "UNKNOWN"
};

/**
 * Initialize the error handling system
 */
int error_init(const char* log_filename, int verbose) {
    g_verbose_mode = verbose;
    
    if (log_filename != NULL) {
        g_log_file = fopen(log_filename, "a");
        if (g_log_file == NULL) {
            fprintf(stderr, "Error: Could not open log file '%s' for writing\n", log_filename);
            return -1;
        }
        
        // Write a header to the log file
        time_t now = time(NULL);
        char time_str[32];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
        
        fprintf(g_log_file, 
                "\n------------------------------------------------------\n"
                "Log started at %s\n"
                "------------------------------------------------------\n",
                time_str);
    }
    
    return 0;
}

/**
 * Clean up the error handling system
 */
void error_cleanup(void) {
    if (g_log_file != NULL) {
        time_t now = time(NULL);
        char time_str[32];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
        
        fprintf(g_log_file, 
                "------------------------------------------------------\n"
                "Log ended at %s\n"
                "------------------------------------------------------\n",
                time_str);
        
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

/**
 * Log an error message with the specified level
 */
void log_message(ErrorLevel level, ErrorCode code, const char* file, 
                int line, const char* func, const char* fmt, ...) {
    // Skip DEBUG messages if not in verbose mode
    if (level == ERROR_DEBUG && !g_verbose_mode) {
        return;
    }
    
    // Prepare the timestamp
    time_t now = time(NULL);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Format the base message
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);
    
    // Format the full log entry
    char log_entry[2048];
    snprintf(log_entry, sizeof(log_entry), 
             "[%s] %s [%s] [%s:%d:%s] %s\n",
             time_str,
             error_level_strings[level],
             error_code_strings[code],
             file, line, func,
             message);
    
    // Print to console based on level
    if (level >= ERROR_WARNING) {
        fprintf(stderr, "%s", log_entry);
    } else if (g_verbose_mode) {
        printf("%s", log_entry);
    }
    
    // Log to file if available
    if (g_log_file != NULL) {
        fprintf(g_log_file, "%s", log_entry);
        fflush(g_log_file); // Ensure the log is written immediately
    }
}

/**
 * Handle a fatal error (print message and exit)
 */
void fatal_error(ErrorCode code, const char* file, int line, 
                const char* func, const char* fmt, ...) {
    // Format the message
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);
    
    // Log the fatal error
    log_message(ERROR_FATAL, code, file, line, func, message);
    
    // Clean up before exit
    error_cleanup();
    
    // Exit with the error code
    exit(code == SYS_MON_SUCCESS ? EXIT_FAILURE : code);
}

/**
 * Check if an error condition is true and log a message if it is
 */
ErrorCode check_error(int condition, ErrorLevel level, ErrorCode code, 
                     const char* file, int line, const char* func, 
                     const char* fmt, ...) {
    if (condition) {
        // Format the message
        char message[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(message, sizeof(message), fmt, args);
        va_end(args);
        
        // Log the error
        log_message(level, code, file, line, func, message);
        
        return code;
    }
    
    return SYS_MON_SUCCESS;
} 