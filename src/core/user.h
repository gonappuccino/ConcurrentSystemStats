#ifndef USER_H
#define USER_H

#include "common.h"

/**
 * User information collection and storage function
 * 
 * Collects information about current user sessions and sends it through pipes.
 * This function is called by child processes to monitor session status.
 * 
 * @param userFD Pipe file descriptor for user information
 * @param ucountFD Pipe file descriptor for user count
 */
void storeUserInfo(int userFD[2], int ucountFD[2]);

/**
 * Current user count function
 * 
 * Calculates and returns the number of currently active user sessions.
 * 
 * @return Number of active user sessions
 */
int getUserCount(void);

// User information output function
void printUserInfo(int userFD[2]);

#endif // USER_H 