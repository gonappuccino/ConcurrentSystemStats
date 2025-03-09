#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/**
 * Error levels for the application
 */
typedef enum {
    ERROR_DEBUG,    // Debug information, only printed in debug mode
    ERROR_INFO,     // Informational messages
    ERROR_WARNING,  // Warnings that don't stop execution
    ERROR_ERROR,    // Errors that may affect functionality
    ERROR_FATAL     // Fatal errors that terminate execution
} ErrorLevel;

/**
 * Error codes for the application
 * Using SYS_MON_ prefix to avoid conflicts with system headers
 */
typedef enum {
    SYS_MON_SUCCESS = 0,       // No error
    SYS_MON_ERR_MEMORY,        // Memory allocation error
    SYS_MON_ERR_IO,            // I/O error
    SYS_MON_ERR_PIPE,          // Pipe error
    SYS_MON_ERR_FORK,          // Fork error
    SYS_MON_ERR_SIGNAL,        // Signal handling error
    SYS_MON_ERR_SYSTEM,        // System call error
    SYS_MON_ERR_PARAMETER,     // Invalid parameter
    SYS_MON_ERR_GTK,           // GTK-related error
    SYS_MON_ERR_PLATFORM,      // Platform-specific error
    SYS_MON_ERR_UNKNOWN        // Unknown error
} ErrorCode;

/**
 * Global variables for error handling
 */
extern int g_verbose_mode;  // Control verbosity of error messages
extern FILE* g_log_file;    // Log file handle

/**
 * Initialize the error handling system
 * 
 * @param log_filename Name of the log file (NULL for no file logging)
 * @param verbose Enable verbose mode
 * @return 0 on success, -1 on failure
 */
int error_init(const char* log_filename, int verbose);

/**
 * Clean up the error handling system
 */
void error_cleanup(void);

/**
 * Log an error message with the specified level
 * 
 * @param level Error level
 * @param code Error code
 * @param file Source file where the error occurred
 * @param line Line number where the error occurred
 * @param func Function where the error occurred
 * @param fmt Format string for the error message
 * @param ... Additional arguments for the format string
 */
void log_message(ErrorLevel level, ErrorCode code, const char* file, 
                int line, const char* func, const char* fmt, ...);

/**
 * Handle a fatal error (print message and exit)
 * 
 * @param code Error code
 * @param file Source file where the error occurred
 * @param line Line number where the error occurred
 * @param func Function where the error occurred
 * @param fmt Format string for the error message
 * @param ... Additional arguments for the format string
 */
void fatal_error(ErrorCode code, const char* file, int line, 
                const char* func, const char* fmt, ...);

/**
 * Check if an error condition is true and log a message if it is
 * 
 * @param condition Error condition to check
 * @param level Error level
 * @param code Error code
 * @param file Source file where the error occurred
 * @param line Line number where the error occurred
 * @param func Function where the error occurred
 * @param fmt Format string for the error message
 * @param ... Additional arguments for the format string
 * @return ErrorCode (SYS_MON_SUCCESS if condition is false)
 */
ErrorCode check_error(int condition, ErrorLevel level, ErrorCode code, 
                     const char* file, int line, const char* func, 
                     const char* fmt, ...);

// Convenience macros for easy use
#define LOG_DEBUG(code, fmt, ...) \
    log_message(ERROR_DEBUG, code, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_INFO(code, fmt, ...) \
    log_message(ERROR_INFO, code, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_WARNING(code, fmt, ...) \
    log_message(ERROR_WARNING, code, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(code, fmt, ...) \
    log_message(ERROR_ERROR, code, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_FATAL(code, fmt, ...) \
    fatal_error(code, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define CHECK_ERROR(condition, level, code, fmt, ...) \
    check_error(condition, level, code, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define CHECK_FATAL(condition, code, fmt, ...) \
    if(condition) { LOG_FATAL(code, fmt, ##__VA_ARGS__); }

#define CHECK_ALLOC(ptr) \
    CHECK_FATAL(ptr == NULL, SYS_MON_ERR_MEMORY, "Memory allocation failed")

#define CHECK_SYSTEM(condition) \
    CHECK_ERROR(condition, ERROR_ERROR, SYS_MON_ERR_SYSTEM, "System error: %s", strerror(errno))

#endif // ERROR_H 