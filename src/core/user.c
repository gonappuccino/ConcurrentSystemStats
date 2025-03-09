#include "user.h"
#include <signal.h>

#ifdef __APPLE__
#include <utmpx.h>
#else
#include <utmp.h>
#include <paths.h>
#endif

/**
 * 사용자 정보 수집 및 파이프로 전송하는 함수
 * @param userFD 사용자 정보 파이프 파일 디스크립터 배열
 * @param ucountFD 사용자 수 파이프 파일 디스크립터 배열
 */
void storeUserInfo(int userFD[2], int ucountFD[2]) {
#ifdef __APPLE__
    struct utmpx *utmp;
    int userLine_count = 0;
    char all_users[MAX_USER_BUFFER] = {0};

    setutxent(); // utmp 파일 열기

    while ((utmp = getutxent()) != NULL) {
        // 사용자 프로세스 확인
        if (utmp->ut_type == USER_PROCESS) {
            char buffer[256] = {0};
            snprintf(buffer, sizeof(buffer), "%s\t %s (%s)\n", 
                    utmp->ut_user, utmp->ut_line, utmp->ut_host);
            
            // 모든 사용자 정보를 하나의 큰 버퍼에 누적
            strncat(all_users, buffer, sizeof(all_users) - strlen(all_users) - 1);
            userLine_count++;
        }
    }

    endutxent(); // utmp 파일 닫기
#else
    struct utmp *utmp;
    int userLine_count = 0;
    char all_users[MAX_USER_BUFFER] = {0};
    
    if (utmpname(_PATH_UTMP) == -1) {
        perror("Error setting utmp file");
        return;
    }

    setutent(); // utmp 파일 열기

    while ((utmp = getutent()) != NULL) {
        // 사용자 프로세스 확인
        if (utmp->ut_type == USER_PROCESS) {
            char buffer[256] = {0};
            snprintf(buffer, sizeof(buffer), "%s\t %s (%s)\n", 
                    utmp->ut_user, utmp->ut_line, utmp->ut_host);
            
            // 모든 사용자 정보를 하나의 큰 버퍼에 누적
            strncat(all_users, buffer, sizeof(all_users) - strlen(all_users) - 1);
            userLine_count++;
        }
    }

    endutent(); // utmp 파일 닫기
#endif

    // 먼저 사용자 수 전송
    if (write(ucountFD[1], &userLine_count, sizeof(userLine_count)) == -1) {
        perror("Error writing user count to pipe");
        return;
    }

    // 전체 사용자 정보 한 번에 전송
    size_t len = strlen(all_users) + 1;
    
    if (write(userFD[1], &len, sizeof(len)) == -1 || 
        write(userFD[1], all_users, len) == -1) {
        perror("Error writing user data to pipe");
        return;
    }
}

/**
 * 사용자 정보 출력 함수
 * @param userFD 사용자 정보 파이프 파일 디스크립터 배열
 */
void printUserInfo(int userFD[2]) {
    size_t len = 0;
    char buffer[MAX_USER_BUFFER] = {0};
    
    printf("### Sessions/users ###\n");
    
    // 먼저 데이터 길이 읽기
    if (read(userFD[0], &len, sizeof(len)) <= 0 || len > sizeof(buffer)) {
        printf("No active user sessions\n");
        return;
    }
    
    // 실제 데이터 읽기
    ssize_t bytesRead = read(userFD[0], buffer, len);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // null 종료 보장
        printf("%s", buffer);
    } else {
        printf("No active user sessions\n");
    }
} 