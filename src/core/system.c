#include "system.h"
#include "../platform/platform.h"
#include "memory.h"
#include "cpu.h"
#include "user.h"
#include <getopt.h>
#include <signal.h>

// 전역 변수
volatile sig_atomic_t exit_flag = 0;
static ProcessIDs process_ids = {-1, -1, -1};

/**
 * Ctrl+Z 처리 함수 - SIGTSTP 신호를 무시하도록 설정
 */
static void handleCtrlZ() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN; // SIGTSTP(Ctrl-Z) 무시

    if (sigaction(SIGTSTP, &action, NULL) == -1) {
        perror("Unable to set up signal handler for SIGTSTP");
        exit(EXIT_FAILURE);
    }
}

/**
 * 자식 프로세스 설정 함수 - 신호 처리 설정
 */
static void childProcessFunction() {
    // 자식 프로세스에서는 신호를 무시하도록 설정
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

/**
 * 자식 프로세스 종료 및 정리 함수
 */
static void cleanupChildProcesses() {
    int status;
    
    // 자식 프로세스 종료 신호 전송
    if (process_ids.memPID > 0) kill(process_ids.memPID, SIGTERM);
    if (process_ids.userPID > 0) kill(process_ids.userPID, SIGTERM);
    if (process_ids.cpuPID > 0) kill(process_ids.cpuPID, SIGTERM);
    
    // 자식 프로세스 종료 대기
    if (process_ids.memPID > 0) waitpid(process_ids.memPID, &status, 0);
    if (process_ids.userPID > 0) waitpid(process_ids.userPID, &status, 0);
    if (process_ids.cpuPID > 0) waitpid(process_ids.cpuPID, &status, 0);
}

/**
 * SIGINT(Ctrl+C) 처리 함수
 * @param signal 신호 번호
 */
static void signalHandler(int signal) {
    if (signal == SIGINT) {
        char userInput[MAX_USER_INPUT] = {0};
        printf("\nCtrl-C detected: terminate? (y/yes to terminate, anything else to continue): ");
        fflush(stdout);
        
        if (fgets(userInput, sizeof(userInput), stdin) != NULL) {
            // 입력 끝의 개행 문자 제거
            size_t len = strlen(userInput);
            if (len > 0 && userInput[len-1] == '\n') {
                userInput[len-1] = '\0';
            }
            
            // 대소문자 구분 없이 y 또는 yes 확인
            if (strcasecmp(userInput, "y") == 0 || strcasecmp(userInput, "yes") == 0) {
                printf("Terminating...\n");
                cleanupChildProcesses();
                exit(EXIT_SUCCESS);
            } else {
                printf("Continuing...\n");
            }
        }
    }
}

/**
 * 신호 핸들러 설정 함수
 */
void setupSignalHandlers() {
    // Ctrl+Z 핸들러 설정
    handleCtrlZ();
    
    // SIGINT 핸들러 설정
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction error for SIGINT");
        exit(EXIT_FAILURE);
    }
}

/**
 * 프로그램 옵션 파싱 함수
 * @param argc 명령행 인수 개수
 * @param argv 명령행 인수 배열
 * @return 프로그램 옵션 구조체
 */
ProgramOptions parseCommandLineOptions(int argc, char *argv[]) {
    ProgramOptions options = {
        .samples = DEFAULT_SAMPLES,
        .tdelay = DEFAULT_DELAY,
        .user = 0,
        .system = 0,
        .sequential = 0,
        .graphics = 0
    };
    
    // 명령행 옵션 구조체
    struct option long_options[] = {
        {"system", no_argument, 0, 's'}, 
        {"user", no_argument, 0, 'u'}, 
        {"graphics", no_argument, 0, 'g'},        
        {"sequential", no_argument, 0, 'a'}, 
        {"samples", optional_argument, 0, 'b'},
        {"tdelay", optional_argument, 0, 'c'},
        {0, 0, 0, 0}
    };
    
    // 옵션 처리
    int opt;
    while ((opt = getopt_long(argc, argv, "sugab::c::", long_options, NULL)) != -1) {
        switch (opt) {
            case 's': options.system = 1; break;
            case 'u': options.user = 1; break;
            case 'g': options.graphics = 1; break;
            case 'a': options.sequential = 1; break;
            case 'b': if (optarg) options.samples = atoi(optarg); break;
            case 'c': if (optarg) options.tdelay = atoi(optarg); break;
        }
    }
    
    // 추가 인수 처리
    for (int ind = optind, i = 0; ind < argc; ind++, i++) {
        switch (i) {
            case 0: options.samples = atoi(argv[ind]); break;
            case 1: options.tdelay = atoi(argv[ind]); break;
        }
    }
    
    return options;
}

/**
 * 상단 정보 출력 함수
 * @param samples 샘플 수
 * @param tdelay 지연 시간(초)
 * @param sequential 순차 모드 여부
 * @param iteration 현재 반복 인덱스
 */
void printTopInfo(int samples, int tdelay, int sequential, int iteration) {
    struct rusage usage_info;
    
    int result = getrusage(RUSAGE_SELF, &usage_info);
    if (sequential) {
        printf(">>> iteration %d\n", iteration);
    } else {
        printf("\033[H\033[2J"); // 화면 지우기
        printf("Nbr of samples: %d-- every %d secs\n", samples, tdelay);
    }
    
    if (result == 0) {
        printf("Memory usage: %ld kilobytes\n", usage_info.ru_maxrss);
    } else {
        printf("Failed to get resource usage info\n");
    }
}

/**
 * 시스템 정보 출력 함수
 */
void printSystemInfo() {
    struct utsname sysinfo;
    int days, hours, minutes, seconds;
    
    // 플랫폼 독립적 함수로 시스템 가동 시간 가져오기
    get_system_uptime(&days, &hours, &minutes, &seconds);
    
    // 일(days)을 시간으로 변환하여 총 시간 계산
    int days_to_hr = 24 * days;
    int total_hr = days_to_hr + hours;
    
    if (uname(&sysinfo) == 0) {
        printf("### System Information ###\n");
        printf("System Name = %s\n", sysinfo.sysname);
        printf("Machine Name= %s\n", sysinfo.nodename);
        printf("Version= %s\n", sysinfo.version);
        printf("Release= %s\n", sysinfo.release);
        printf("Architecture= %s\n", sysinfo.machine);
        printf("System running since last reboot: %d days %02d:%02d:%02d (%02d:%02d:%02d)\n",
               days, hours, minutes, seconds, total_hr, minutes, seconds);
    } else {
        perror("Error getting system information");
    }
}

/**
 * 파이프 설정 함수
 * @param pipes 파이프 구조체 포인터
 * @return 성공 시 STATUS_SUCCESS, 실패 시 오류 코드
 */
static int setupPipes(PipeSet *pipes) {
    if (pipe(pipes->memFD) == -1 || pipe(pipes->userFD) == -1 || 
        pipe(pipes->cpuPFD) == -1 || pipe(pipes->cpuCFD) == -1 || pipe(pipes->ucountFD) == -1) {
        perror("Pipe creation failed");
        return STATUS_ERROR_PIPE;
    }
    return STATUS_SUCCESS;
}

/**
 * 자식 프로세스 생성 및 관리 함수
 * @param samples 샘플 수
 * @param tdelay 지연 시간(초)
 * @param pipes 파이프 구조체 포인터
 * @return 프로세스 ID 구조체
 */
ProcessIDs createChildProcesses(int samples, int tdelay, PipeSet *pipes) {
    // 파이프 설정
    if (setupPipes(pipes) != STATUS_SUCCESS) {
        exit(EXIT_FAILURE);
    }
    
    // 메모리 정보 수집 자식 프로세스 생성
    process_ids.memPID = fork();
    if (process_ids.memPID == -1) {
        perror("Memory fork failed");
        exit(EXIT_FAILURE);
    } else if (process_ids.memPID == 0) {
        childProcessFunction();
        
        // 사용하지 않는 파이프 닫기
        close(pipes->cpuPFD[0]); close(pipes->cpuPFD[1]);
        close(pipes->cpuCFD[0]); close(pipes->cpuCFD[1]);
        close(pipes->userFD[0]); close(pipes->userFD[1]);
        close(pipes->ucountFD[0]); close(pipes->ucountFD[1]);
        close(pipes->memFD[0]); // 읽기 종단 닫기
        
        // 메모리 정보 수집 및 전송 - 매개변수 순서 수정
        storeMemoryInfo(tdelay, samples, pipes->memFD);
        
        close(pipes->memFD[1]);
        exit(EXIT_SUCCESS);
    }
    
    // 사용자 정보 수집 자식 프로세스 생성
    process_ids.userPID = fork();
    if (process_ids.userPID == -1) {
        perror("User fork failed");
        kill(process_ids.memPID, SIGTERM);
        waitpid(process_ids.memPID, NULL, 0);
        exit(EXIT_FAILURE);
    } else if (process_ids.userPID == 0) {
        childProcessFunction();
        
        // 사용하지 않는 파이프 닫기
        close(pipes->cpuPFD[0]); close(pipes->cpuPFD[1]);
        close(pipes->cpuCFD[0]); close(pipes->cpuCFD[1]);
        close(pipes->memFD[0]); close(pipes->memFD[1]);
        close(pipes->userFD[0]); // 읽기 종단 닫기
        close(pipes->ucountFD[0]); // 읽기 종단 닫기
        
        // 사용자 정보 수집 및 전송
        storeUserInfo(pipes->userFD, pipes->ucountFD);
        
        close(pipes->userFD[1]);
        close(pipes->ucountFD[1]);
        exit(EXIT_SUCCESS);
    }
    
    // CPU 정보 수집 자식 프로세스 생성
    process_ids.cpuPID = fork();
    if (process_ids.cpuPID == -1) {
        perror("CPU fork failed");
        kill(process_ids.memPID, SIGTERM);
        kill(process_ids.userPID, SIGTERM);
        waitpid(process_ids.memPID, NULL, 0);
        waitpid(process_ids.userPID, NULL, 0);
        exit(EXIT_FAILURE);
    } else if (process_ids.cpuPID == 0) {
        childProcessFunction();
        
        // 사용하지 않는 파이프 닫기
        close(pipes->memFD[0]); close(pipes->memFD[1]);
        close(pipes->userFD[0]); close(pipes->userFD[1]);
        close(pipes->ucountFD[0]); close(pipes->ucountFD[1]);
        close(pipes->cpuPFD[0]); // 읽기 종단 닫기
        close(pipes->cpuCFD[0]); // 읽기 종단 닫기
        
        // CPU 정보 수집 및 전송
        for (int i = 0; i < samples; i++) {
            storeCPUInfo(pipes->cpuPFD);
            sleep(tdelay);
            storeCPUInfo(pipes->cpuCFD);
        }
        
        close(pipes->cpuPFD[1]);
        close(pipes->cpuCFD[1]);
        exit(EXIT_SUCCESS);
    }
    
    // 부모 프로세스에서는 쓰기용 파이프 닫기
    close(pipes->memFD[1]);
    close(pipes->userFD[1]);
    close(pipes->cpuPFD[1]);
    close(pipes->cpuCFD[1]);
    close(pipes->ucountFD[1]);
    
    return process_ids;
} 