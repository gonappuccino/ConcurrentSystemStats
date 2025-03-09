#!/bin/bash

# 색상 설정 
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m' # 색상 초기화

echo -e "${BOLD}Git 저장소 설정 스크립트${NC}"
echo -e "${BLUE}====================================${NC}"

# Git이 설치되어 있는지 확인
if ! command -v git &> /dev/null; then
    echo -e "${RED}Git이 설치되어 있지 않습니다. 먼저 Git을 설치해주세요.${NC}"
    exit 1
fi

# Git 저장소 초기화
echo -e "${YELLOW}Git 저장소를 초기화합니다...${NC}"
git init

# .gitignore 파일 생성
echo -e "${YELLOW}.gitignore 파일을 생성합니다...${NC}"
cat > .gitignore << 'EOL'
# 빌드 결과물
build/
system_monitor_cli
system_monitor_gui
*.o
*.a
*.so
*.dylib

# IDE 관련 파일
.vscode/
.idea/
*.swp
*.swo

# 로그 파일
logs/*.log

# 시스템 파일
.DS_Store
Thumbs.db

# 임시 파일
*.tmp
*~
EOL

# README.md가 있는지 확인하고 없으면 생성
if [ ! -f "README.md" ]; then
    echo -e "${YELLOW}README.md 파일을 생성합니다...${NC}"
    cat > README.md << 'EOL'
# Concurrent System Stats

다중 프로세스 시스템 모니터링 도구 (CLI 및 GUI 인터페이스 제공)

## 프로젝트 구조

```
ConcurrentSystemStats/
├── src/                    # 소스 코드
│   ├── core/               # 핵심 모니터링 기능
│   │   ├── cpu.c/h         # CPU 모니터링
│   │   ├── memory.c/h      # 메모리 모니터링
│   │   ├── system.c/h      # 시스템 정보
│   │   └── user.c/h        # 사용자 세션 모니터링
│   ├── gui/                # GUI 관련 코드
│   │   ├── gui.c/h         # 메인 GUI 구현
│   │   └── gui_utils.c/h   # GUI 유틸리티 함수
│   ├── platform/           # 플랫폼별 구현
│   │   ├── platform.h      # 공통 플랫폼 인터페이스
│   │   ├── platform_linux.c # Linux 구현
│   │   └── platform_mac.c  # macOS 구현
│   ├── utils/              # 유틸리티 함수
│   │   ├── common.h        # 공통 정의
│   │   └── error.c/h       # 오류 처리
│   └── main/               # 진입점
│       ├── main.c          # CLI 진입점
│       └── gui_main.c      # GUI 진입점
├── scripts/                # 빌드 및 실행 스크립트
├── logs/                   # 로그 파일
└── build/                  # 빌드 결과물
```

## 요구사항

- C 컴파일러 (GCC 또는 Clang)
- GUI 버전: GTK+3 개발 라이브러리

### 의존성 설치

#### macOS

```bash
brew install gtk+3
```

#### Ubuntu/Debian

```bash
sudo apt install libgtk-3-dev
```

#### Fedora

```bash
sudo dnf install gtk3-devel
```

## 빌드 방법

```bash
make clean && make all    # CLI 및 GUI 버전 빌드
make cli                  # CLI 버전만 빌드
make gui                  # GUI 버전만 빌드
```

## 실행 방법

```bash
./system_monitor_cli      # CLI 버전 실행
./system_monitor_gui      # GUI 버전 실행

# 또는 스크립트 사용
./scripts/run_gui.sh      # GUI 실행
./scripts/run_gui.sh --debug  # 디버그 모드로 GUI 실행
```

## 기능

- 실시간 CPU 사용량 모니터링
- 메모리 사용량 추적
- 시스템 정보 표시
- 사용자 세션 모니터링
- 그래픽 시각화 (GUI 버전)
- 멀티프로세스 데이터 수집

## 지원 플랫폼

- Linux
- macOS
EOL
fi

# 변경사항 스테이징
echo -e "${YELLOW}변경사항을 스테이징합니다...${NC}"
git add .

# 첫 커밋 생성
echo -e "${YELLOW}첫 커밋을 생성합니다...${NC}"
git commit -m "초기 커밋: 폴더 구조화된 동시 시스템 모니터링 도구"

# 원격 저장소 추가 안내
echo -e "\n${BOLD}Git 저장소가 성공적으로 설정되었습니다!${NC}"
echo -e "${BLUE}다음 단계:${NC}"
echo -e "1. GitHub, GitLab 등에 새 저장소를 생성하세요."
echo -e "2. 원격 저장소를 추가하고 코드를 푸시하세요:"
echo -e ""
echo -e "   ${BOLD}# GitHub 예시${NC}"
echo -e "   git remote add origin https://github.com/사용자명/저장소명.git"
echo -e "   git branch -M main"
echo -e "   git push -u origin main"
echo -e ""
echo -e "   ${BOLD}# GitLab 예시${NC}"
echo -e "   git remote add origin https://gitlab.com/사용자명/저장소명.git"
echo -e "   git branch -M main"
echo -e "   git push -u origin main"
echo -e ""
echo -e "${GREEN}완료! 이제 코드가 Git 저장소에 추가되었습니다.${NC}" 