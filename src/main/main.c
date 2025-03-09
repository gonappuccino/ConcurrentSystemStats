#define _POSIX_C_SOURCE 200809L
#include "common.h"
#include "memory.h"
#include "cpu.h"
#include "user.h"
#include "system.h"
#include "error.h"

#ifdef ENABLE_GUI
#include "gui.h"
#endif

// 상수 정의
#define DEFAULT_REFRESH_RATE 1  // 기본 갱신 주기 (초)

// 함수 선언
void runSequentialMode(int samples, int tdelay, int user, int system, int graphics,
                      PipeSet *pipes);
void runNonsequentialMode(int samples, int tdelay, int user, int system, int graphics,
                         PipeSet *pipes, int userLine_count);
void closePipes(PipeSet *pipes);
void printUsage(const char* programName);

/**
 * 사용법 출력 함수
 * 프로그램의 명령줄 인수와 사용 방법을 출력합니다.
 * 
 * @param programName 프로그램 이름
 */
void printUsage(const char* programName) {
    printf("Usage: %s [options]\n", programName);
    printf("Options:\n");
    printf("  -h, --help                  Display this help message\n");
    printf("  -r, --refresh <seconds>     Set refresh rate (default: %d second)\n", DEFAULT_REFRESH_RATE);
    printf("  -s, --sequential            Run in sequential mode\n");
    printf("  -u, --user                  Display user information only\n");
    printf("  -m, --system                Display system information only\n");
    printf("  -g, --graphics              Enable graphical display\n");
    printf("  --samples <count>           Number of samples to collect (default: 10)\n");
    printf("  --tdelay <seconds>          Time between samples (default: 1 second)\n");
}

/**
 * 파이프 닫기 함수
 * 프로그램에서 사용한 모든 파이프를 닫습니다.
 * 
 * @param pipes 파이프 구조체 포인터
 */
void closePipes(PipeSet *pipes) {
    close(pipes->cpuPFD[0]);
    close(pipes->cpuCFD[0]);
    close(pipes->userFD[0]);
    close(pipes->memFD[0]);
    close(pipes->ucountFD[0]);
}

/**
 * 메인 함수
 * 프로그램의 진입점으로 명령줄 인수를 처리하고 CLI 또는 GUI 모드로 실행합니다.
 * 
 * @param argc 명령행 인수 개수
 * @param argv 명령행 인수 배열
 * @return 종료 코드
 */
int main(int argc, char *argv[]) {
    // Initialize error handling system
    error_init("system_monitor.log", 0);  // 0 for non-verbose mode
    
    LOG_INFO(SYS_MON_SUCCESS, "System monitor starting");
    
    // 명령줄 인수 처리
    int refresh_rate = DEFAULT_REFRESH_RATE;
    
    // 사용되지 않는 변수 제거 (경고 해결)
    // int gui_mode = 0;  <- 이 줄 삭제 또는 아래와 같이 수정
    
    // 필요하다면 나중에 GUI 모드 지원 추가를 위해 주석으로 유지
    // bool gui_mode = false;
    
    // 명령줄 인수 처리
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--refresh") == 0) {
            if (i + 1 < argc) {
                refresh_rate = atoi(argv[i + 1]);
                if (refresh_rate <= 0) {
                    refresh_rate = DEFAULT_REFRESH_RATE;
                }
                i++; // 다음 인자 건너뛰기
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        }
        // GUI 모드 옵션은 CLI 모드에서 무시
        // else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gui") == 0) {
        //     gui_mode = true;
        // }
    }
    
#ifdef ENABLE_GUI
    // GUI 모드로 실행
    if (gui_mode) {
        init_gui(&argc, &argv);
        run_gui();
        cleanup_gui();
        return 0;
    }
#endif
    
    // CLI 모드로 실행
    // 신호 핸들러 설정
    setupSignalHandlers();
    
    // 프로그램 옵션 파싱
    ProgramOptions options = parseCommandLineOptions(argc, argv);
    
    // 파이프 생성
    PipeSet pipes;
    
    // Create pipes for inter-process communication
    if (pipe(pipes.cpuPFD) < 0 || 
        pipe(pipes.cpuCFD) < 0 || 
        pipe(pipes.userFD) < 0 || 
        pipe(pipes.memFD) < 0 || 
        pipe(pipes.ucountFD) < 0) {
        
        LOG_FATAL(SYS_MON_ERR_PIPE, "Pipe creation failed: %s", strerror(errno));
        // Fatal error logs and exits automatically
    }
    
    // Create child processes
    ProcessIDs pids = createChildProcesses(options.samples, options.tdelay, &pipes);
    
    // Check if process creation was successful
    if (pids.cpuPID <= 0 || pids.memPID <= 0 || pids.userPID <= 0) {
        LOG_FATAL(SYS_MON_ERR_FORK, "Failed to create child processes");
    }
    
    // 사용자 수 읽기
    int userLine_count = 0;
    read(pipes.ucountFD[0], &userLine_count, sizeof(userLine_count));
    
    // 순차 모드 또는 비순차 모드 실행
    // 순차 모드: 화면이 갱신될 때마다 이전 출력이 유지되고 새로운 출력이 추가됨
    // 비순차 모드: 화면이 갱신될 때마다 이전 출력이 지워지고 새로운 출력으로 대체됨
    if (options.sequential) {
        runSequentialMode(options.samples, options.tdelay, options.user, 
                         options.system, options.graphics, &pipes);
    } else {
        runNonsequentialMode(options.samples, options.tdelay, options.user, 
                            options.system, options.graphics, &pipes, userLine_count);
    }
    
    // 파이프 닫기
    closePipes(&pipes);
    
    // 시스템 정보 출력
    printf("------------------------------------\n");
    printSystemInfo();
    printf("----------------------------------\n");
    
    // Clean up error handling
    error_cleanup();
    return 0;
}

/**
 * 순차 모드 실행 함수
 * 화면이 갱신될 때마다 이전 출력을 유지하고 새로운 출력을 추가하는 모드입니다.
 * 
 * @param samples 샘플 수
 * @param tdelay 지연 시간(초)
 * @param user 사용자 정보 표시 여부
 * @param system 시스템 정보 표시 여부
 * @param graphics 그래픽 표시 여부
 * @param pipes 파이프 구조체 포인터
 */
void runSequentialMode(int samples, int tdelay, int user, int system, int graphics,
                      PipeSet *pipes) {
    // 데이터 저장을 위한 배열 및 변수 초기화
    char memArr[samples][MAX_MEMORY_BUFFER];  // 메모리 정보 저장 배열
    char cpuArr[samples][MAX_CPU_BUFFER];     // CPU 정보 저장 배열
    unsigned long prevCpuUsage[7], currCpuUsage[7];  // CPU 사용량 데이터
    double virtual_used_gb = 0.0, prev_used_gb = 0.0;  // 메모리 사용량
    float cur_cpuUsage = 0.0, prevCpuUsageFloat = 0.0;  // CPU 사용량 계산 결과
    size_t len = 0;
    ssize_t bytes_read = 0;
    
    // 샘플 수만큼 반복 실행
    for (int i = 0; i < samples; i++) {
        cpuArr[i][0] = '\0';  // CPU 배열 초기화
        sleep(tdelay);  // 지정된 시간만큼 대기
        
        // 상단 정보 출력 (샘플 수, 지연 시간, 현재 반복 횟수)
        printTopInfo(samples, tdelay, 1, i);  // sequential = 1
        
        // 시스템 정보 표시 조건 확인
        if (!user || (user && system)) {
            printf("---------------------------------------\n");
            
            // 메모리 정보 읽기
            len = 0;
            bytes_read = read(pipes->memFD[0], &len, sizeof(len));
            
            // 메모리 정보가 성공적으로 읽혔는지 확인
            if (bytes_read > 0 && len > 0 && len < sizeof(memArr[i])) {
                bytes_read = read(pipes->memFD[0], memArr[i], len);
                
                if (bytes_read > 0) {
                    memArr[i][bytes_read] = '\0';  // 문자열 종료 처리
                    
                    // 그래픽 표시 옵션이 활성화된 경우
                    if (graphics) {
                        virtual_used_gb = getVirtualMemoryUsage();
                        createMemoryGraphics(virtual_used_gb, &prev_used_gb, memArr, i);
                    }
                    
                    // 메모리 정보 출력
                    printMemoryInfo(1, samples, memArr, i, pipes->memFD);
                }
            }
            
            // 사용자 정보 출력 여부 확인
            if ((user && system) || !system) {
                printf("---------------------------------------\n");
                printUserInfo(pipes->userFD);
                printf("---------------------------------------\n");
            }
            
            // CPU 코어 정보 출력
            printCPUCores();
            
            // CPU 정보 읽기
            read(pipes->cpuPFD[0], &prevCpuUsage, sizeof(prevCpuUsage));
            read(pipes->cpuCFD[0], &currCpuUsage, sizeof(currCpuUsage));
            
            // CPU 사용량 계산 및 출력
            cur_cpuUsage = calculateCPUUsage(prevCpuUsage, currCpuUsage);
            printf("total cpu use: %.2f%%\n", cur_cpuUsage);
            
            // CPU 그래픽 표시
            if (graphics) {
                setCPUGraphics(1, cpuArr, cur_cpuUsage, &prevCpuUsageFloat, i);
            }
        } else {
            // 사용자 정보만 표시
            printf("---------------------------------------\n");
            printUserInfo(pipes->userFD);
            printf("---------------------------------------\n");
        }
    }
}

/**
 * 비순차 모드 실행 함수
 * 화면이 갱신될 때마다 이전 출력이 지워지고 새로운 출력으로 대체되는 모드입니다.
 * 
 * @param samples 샘플 수
 * @param tdelay 지연 시간(초)
 * @param user 사용자 정보 표시 여부
 * @param system 시스템 정보 표시 여부
 * @param graphics 그래픽 표시 여부
 * @param pipes 파이프 구조체 포인터
 * @param userLine_count 사용자 수
 */
void runNonsequentialMode(int samples, int tdelay, int user, int system, int graphics,
                         PipeSet *pipes, int userLine_count) {
    // 데이터 저장을 위한 배열 및 변수 초기화
    char memArr[samples][MAX_MEMORY_BUFFER];  // 메모리 정보 저장 배열
    char cpuArr[samples][MAX_CPU_BUFFER];     // CPU 정보 저장 배열
    unsigned long prevCpuUsage[7], currCpuUsage[7];  // CPU 사용량 데이터
    double virtual_used_gb = 0.0, prev_used_gb = 0.0;  // 메모리 사용량
    float cur_cpuUsage = 0.0, prevCpuUsageFloat = 0.0;  // CPU 사용량 계산 결과
    size_t len = 0;
    ssize_t bytes_read = 0;
    int systemStartGraphics = 0;
    int memStartCursor = 0;
    int CPU_GRAPH_START_LINE;
    
    // 샘플 수만큼 반복 실행
    for (int i = 0; i < samples; i++) {
        sleep(tdelay);  // 지정된 시간만큼 대기
        
        // 상단 정보 출력 (샘플 수, 지연 시간, 현재 반복 횟수)
        printTopInfo(samples, tdelay, 0, i);  // sequential = 0
        
        // 시스템 정보 표시 조건 확인
        if (!user || (user && system)) {
            printf("------------------------------------------------\n");
            reserveSpace(samples);  // 출력 공간 확보
            
            // 사용자 정보 출력 여부 확인
            if ((user && system) || !system) {
                printf("---------------------------------------\n");
                printUserInfo(pipes->userFD);
                printf("---------------------------------------\n");
            }
            
            // CPU 코어 정보 출력
            printCPUCores();
            
            // CPU 정보 읽기
            read(pipes->cpuPFD[0], &prevCpuUsage, sizeof(prevCpuUsage));
            read(pipes->cpuCFD[0], &currCpuUsage, sizeof(currCpuUsage));
            
            // CPU 사용량 계산 및 출력
            cur_cpuUsage = calculateCPUUsage(prevCpuUsage, currCpuUsage);
            printf("total cpu use: %.2f%%\n", cur_cpuUsage);
            
            // 메모리 정보 읽기
            len = 0;
            bytes_read = read(pipes->memFD[0], &len, sizeof(len));
            
            // 메모리 정보가 성공적으로 읽혔는지 확인
            if (bytes_read > 0 && len > 0 && len < sizeof(memArr[i])) {
                bytes_read = read(pipes->memFD[0], memArr[i], len);
                
                if (bytes_read > 0) {
                    memArr[i][bytes_read] = '\0';  // 문자열 종료 처리
                    
                    // 그래픽 표시 옵션이 활성화된 경우
                    if (graphics) {
                        virtual_used_gb = getVirtualMemoryUsage();
                        createMemoryGraphics(virtual_used_gb, &prev_used_gb, memArr, i);
                    }
                    
                    // 커서 위치 조정 - 시스템 설정에 따라 다르게 처리
                    if (system && !user) {
                        memStartCursor = samples + 3;
                    } else if (system && user) {
                        memStartCursor = samples + userLine_count + 4;
                    } else if (user && !(system && user)) {
                        memStartCursor = samples + userLine_count + 6;
                    } else {
                        memStartCursor = samples + userLine_count + 4;
                    }
                    printf("\033[%dA", memStartCursor);  // 커서를 위로 이동
                    
                    // 메모리 정보 출력
                    printMemoryInfo(0, samples, memArr, i, pipes->memFD);
                    
                    // CPU 그래픽 표시
                    if (graphics && system) {
                        printf("\033[%d;1H", CPU_GRAPH_START_LINE = 18);  // 커서 위치 지정
                        setCPUGraphics(0, cpuArr, cur_cpuUsage, &prevCpuUsageFloat, i);
                    }
                }
            }
            
            // 커서 위치 조정
            int systemStart = userLine_count + 6;
            if (graphics) {
                systemStart = systemStartGraphics;
            } else if (system && !user) {
                systemStart = userLine_count + 3;
            } else {
                systemStart = userLine_count + 6;
            }
            printf("\033[%dB", systemStart);  // 커서를 아래로 이동
        } else {
            // 사용자 정보만 표시
            printf("---------------------------------------\n");
            printUserInfo(pipes->userFD);
            printf("---------------------------------------\n");
            printf("\033[%dB", userLine_count);  // 커서를 아래로 이동
        }
    }
} 