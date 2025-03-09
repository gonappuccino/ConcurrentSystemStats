#!/bin/bash

# 색상 설정 
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m' # 색상 초기화

echo -e "${BOLD}GitHub 저장소에 코드 업로드 스크립트${NC}"
echo -e "${BLUE}====================================${NC}"

# 사용자 입력 받기
echo -e "${YELLOW}GitHub 사용자명을 입력하세요:${NC}"
read github_username
echo -e "${YELLOW}GitHub 저장소명을 입력하세요:${NC}"
read repo_name

# 저장소 URL 생성
repo_url="https://github.com/$github_username/$repo_name.git"

echo -e "${YELLOW}리모트 저장소: $repo_url${NC}"
echo -e "${YELLOW}이 URL로 진행할까요? (y/n)${NC}"
read confirm

if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo -e "${RED}작업이 취소되었습니다.${NC}"
    exit 1
fi

# Git 초기화 여부 확인
if [ ! -d ".git" ]; then
    echo -e "${YELLOW}Git 저장소가 초기화되어 있지 않습니다. setup_git_repo.sh를 먼저 실행하세요.${NC}"
    
    echo -e "${YELLOW}지금 setup_git_repo.sh를 실행할까요? (y/n)${NC}"
    read init_confirm
    
    if [ "$init_confirm" == "y" ] || [ "$init_confirm" == "Y" ]; then
        chmod +x setup_git_repo.sh
        ./setup_git_repo.sh
    else
        echo -e "${RED}작업이 취소되었습니다.${NC}"
        exit 1
    fi
fi

# 리모트 설정
echo -e "${YELLOW}원격 저장소를 추가합니다...${NC}"
git remote add origin $repo_url || git remote set-url origin $repo_url

# 메인 브랜치로 전환
echo -e "${YELLOW}메인 브랜치로 전환합니다...${NC}"
git branch -M main

# 푸시
echo -e "${YELLOW}GitHub에 코드를 푸시합니다...${NC}"
git push -u origin main

if [ $? -eq 0 ]; then
    echo -e "${GREEN}코드가 성공적으로 푸시되었습니다!${NC}"
    echo -e "${BLUE}저장소 URL: ${BOLD}https://github.com/$github_username/$repo_name${NC}"
else
    echo -e "${RED}푸시 중 오류가 발생했습니다. GitHub 인증을 확인하세요.${NC}"
    echo -e "${YELLOW}GitHub CLI나 개인 액세스 토큰을 설정해야 할 수 있습니다.${NC}"
    echo -e "도움말: https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token"
fi 