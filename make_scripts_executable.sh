#!/bin/bash

# 모든 스크립트에 실행 권한 부여
chmod +x setup_git_repo.sh
chmod +x push_to_github.sh
chmod +x scripts/*.sh
chmod +x organize_project.sh

echo "모든 스크립트에 실행 권한이 부여되었습니다."
echo "이제 다음 명령어로 GitHub 저장소 설정을 시작할 수 있습니다:"
echo "./setup_git_repo.sh" 