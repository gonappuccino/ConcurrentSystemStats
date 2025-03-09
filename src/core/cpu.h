#ifndef CPU_H
#define CPU_H

#include "common.h"

/**
 * CPU 정보 수집 및 저장 함수
 * 
 * 현재 시스템의 CPU 사용량 정보를 수집하여 파이프를 통해 전송합니다.
 * 이 함수는 자식 프로세스에서 호출되며 주기적으로 CPU 상태를 모니터링합니다.
 * 
 * @param pipe_fd CPU 정보를 전송할 파이프 파일 디스크립터
 */
void storeCPUInfo(int pipe_fd[2]);

/**
 * CPU 사용량 계산 함수
 * 
 * 이전 CPU 상태와 현재 CPU 상태를 비교하여 CPU 사용률을 계산합니다.
 * 사용자 모드, 시스템 모드, 유휴 등 다양한 CPU 상태를 고려하여 계산합니다.
 * 
 * @param prevCpuUsage 이전 CPU 상태 배열 (사용자, nice, 시스템, 유휴, iowait, irq, softirq)
 * @param currCpuUsage 현재 CPU 상태 배열 (사용자, nice, 시스템, 유휴, iowait, irq, softirq)
 * @return 계산된 CPU 사용률 (백분율)
 */
double calculateCPUUsage(unsigned long prevCpuUsage[7], unsigned long currCpuUsage[7]);

/**
 * CPU 코어 수 반환 함수
 * 
 * 시스템의 CPU 코어 수를 반환합니다. 이 정보는 시스템 정보 표시에 사용됩니다.
 * 
 * @return CPU 코어 수
 */
int getCPUCores(void);

// CPU 코어 개수 출력 함수
void printCPUCores(void);

// CPU 사용량 그래픽 설정 함수
void setCPUGraphics(int sequential, char cpuArr[][MAX_CPU_BUFFER], float curCpuUsage, float *prevCpuUsage, int sampleIndex);

// CPU 정보 수집 및 표시 함수
void printCPUInfoAndGraphics(int cpuPFD[2], int cpuCFD[2], int sequential, int index, int graphics);

#endif // CPU_H 