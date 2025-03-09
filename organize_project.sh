#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
BOLD='\033[1m'
NC='\033[0m' # No Color

echo -e "${BOLD}System Monitor Project Organizer${NC}"
echo -e "${BLUE}====================================${NC}"

# Create directory structure
echo -e "${YELLOW}Creating directory structure...${NC}"
mkdir -p src/core
mkdir -p src/gui
mkdir -p src/platform
mkdir -p src/utils
mkdir -p src/main
mkdir -p scripts
mkdir -p logs
mkdir -p build

# Move core files
echo -e "${YELLOW}Moving core files...${NC}"
mv cpu.c cpu.h memory.c memory.h system.c system.h user.c user.h src/core/

# Move GUI files
echo -e "${YELLOW}Moving GUI files...${NC}"
mv gui.c gui.h gui_utils.c gui_utils.h src/gui/

# Move platform files
echo -e "${YELLOW}Moving platform files...${NC}"
mv platform.h platform_linux.c platform_mac.c platform_darwin.c sysinfo.h src/platform/

# Move utility files
echo -e "${YELLOW}Moving utility files...${NC}"
mv common.h error.c error.h src/utils/

# Move main application files
echo -e "${YELLOW}Moving main application files...${NC}"
mv main.c gui_main.c main_gui.c src/main/

# Move scripts
echo -e "${YELLOW}Moving script files...${NC}"
mv build_gui.sh check_gtk.sh compile_gui.sh run_gui.sh scripts/

# Move log files
echo -e "${YELLOW}Moving log files...${NC}"
mv compile_errors.log gui_debug.log logs/

# Keep some files in root
echo -e "${YELLOW}Keeping some files in root directory...${NC}"
# .gitattributes and Makefile already in root

# Make script files executable
echo -e "${YELLOW}Making script files executable...${NC}"
chmod +x scripts/*.sh

# Update scripts to use new paths
echo -e "${YELLOW}Updating script paths...${NC}"
sed -i '' 's|cpu\.c|src/core/cpu.c|g' scripts/compile_gui.sh 2>/dev/null || sed -i 's|cpu\.c|src/core/cpu.c|g' scripts/compile_gui.sh
sed -i '' 's|memory\.c|src/core/memory.c|g' scripts/compile_gui.sh 2>/dev/null || sed -i 's|memory\.c|src/core/memory.c|g' scripts/compile_gui.sh
sed -i '' 's|platform_mac\.c|src/platform/platform_mac.c|g' scripts/compile_gui.sh 2>/dev/null || sed -i 's|platform_mac\.c|src/platform/platform_mac.c|g' scripts/compile_gui.sh
sed -i '' 's|platform_linux\.c|src/platform/platform_linux.c|g' scripts/compile_gui.sh 2>/dev/null || sed -i 's|platform_linux\.c|src/platform/platform_linux.c|g' scripts/compile_gui.sh
sed -i '' 's|gui_main\.c|src/main/gui_main.c|g' scripts/compile_gui.sh 2>/dev/null || sed -i 's|gui_main\.c|src/main/gui_main.c|g' scripts/compile_gui.sh
sed -i '' 's|gui\.c|src/gui/gui.c|g' scripts/compile_gui.sh 2>/dev/null || sed -i 's|gui\.c|src/gui/gui.c|g' scripts/compile_gui.sh

# Update scripts with new include paths
sed -i '' 's|COMPILE_FLAGS="$COMMON_FLAGS|COMPILE_FLAGS="$COMMON_FLAGS -Isrc/core -Isrc/gui -Isrc/platform -Isrc/utils -Isrc/main|g' scripts/compile_gui.sh 2>/dev/null || sed -i 's|COMPILE_FLAGS="$COMMON_FLAGS|COMPILE_FLAGS="$COMMON_FLAGS -Isrc/core -Isrc/gui -Isrc/platform -Isrc/utils -Isrc/main|g' scripts/compile_gui.sh

echo -e "${GREEN}Project organization complete!${NC}"
echo -e "New structure:"
echo -e "  ${BLUE}src/${NC} - Source code"
echo -e "    ${BLUE}core/${NC} - Core functionality (CPU, memory, system)"
echo -e "    ${BLUE}gui/${NC} - GUI implementation"
echo -e "    ${BLUE}platform/${NC} - Platform-specific code"
echo -e "    ${BLUE}utils/${NC} - Utility functions"
echo -e "    ${BLUE}main/${NC} - Main application entry points"
echo -e "  ${BLUE}scripts/${NC} - Build and run scripts"
echo -e "  ${BLUE}logs/${NC} - Log files"
echo -e "  ${BLUE}build/${NC} - Build artifacts"
echo -e ""

echo -e "${GREEN}All setup steps completed automatically!${NC}"
echo -e "You can now build your project with:"
echo -e "   ${BOLD}make clean && make all${NC}" 