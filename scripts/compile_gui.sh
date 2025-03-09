#!/bin/bash

# Color settings
GREEN="\033[0;32m"
RED="\033[0;31m"
BLUE="\033[0;34m"
YELLOW="\033[0;33m"
BOLD="\033[1m"
NC="\033[0m" # No Color

echo -e "${BOLD}System Monitoring Tool GUI Compiler${NC}"
echo -e "${BLUE}=====================================${NC}"

# Check for GTK+3 first
if ! pkg-config --exists gtk+-3.0; then
    echo -e "${RED}Error: GTK+3 development libraries not found.${NC}"
    echo -e "${YELLOW}Please install GTK+3 before compiling:${NC}"
    echo -e "  - macOS: brew install gtk+3"
    echo -e "  - Ubuntu/Debian: sudo apt install libgtk-3-dev"
    echo -e "  - Fedora: sudo dnf install gtk3-devel"
    exit 1
fi

# Determine platform
PLATFORM=$(uname -s)
echo -e "Build environment ready (${PLATFORM} platform)"

# Ensure build directory exists
if [ ! -d "build" ]; then
    mkdir build
fi

# Get GTK+3 compiler flags
GTK_CFLAGS=$(pkg-config --cflags gtk+-3.0)
GTK_LIBS=$(pkg-config --libs gtk+-3.0)

# Common compiler flags
COMMON_FLAGS="-Wall -Wextra -I."

# Debug flags
DEBUG_FLAGS="-g -DDEBUG"

# Platform-specific compilation
if [ "$PLATFORM" = "Darwin" ]; then
    # macOS specific
    CC="clang"
    LD_FLAGS="$GTK_LIBS"
    COMPILE_FLAGS="$COMMON_FLAGS -Isrc/core -Isrc/gui -Isrc/platform -Isrc/utils -Isrc/main $GTK_CFLAGS -DHAVE_GTK -D_DARWIN_C_SOURCE"
    PLATFORM_SRC="src/platform/platform_mac.c"
elif [ "$PLATFORM" = "Linux" ]; then
    # Linux specific
    CC="gcc"
    LD_FLAGS="$GTK_LIBS -lm"
    COMPILE_FLAGS="$COMMON_FLAGS -Isrc/core -Isrc/gui -Isrc/platform -Isrc/utils -Isrc/main $GTK_CFLAGS -DHAVE_GTK"
    PLATFORM_SRC="src/platform/platform_linux.c"
else
    echo -e "${RED}Unsupported platform: $PLATFORM${NC}"
    exit 1
fi

# First, build the CLI version
echo -e "${BLUE}Building CLI version...${NC}"
make cli
if [ $? -ne 0 ]; then
    echo -e "${RED}CLI build failed!${NC}"
    exit 1
fi
echo -e "${GREEN}CLI version build completed: system_monitor_cli${NC}"

# Build GUI version
echo -e "\n${BLUE}Building GUI version...${NC}"

# Compile src/core/memory.c
echo -e "Compiling: src/core/memory.c"
$CC $COMPILE_FLAGS $DEBUG_FLAGS -c src/core/memory.c -o build/memory.o
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi

# Compile platform specific code
echo -e "Compiling: $PLATFORM_SRC"
$CC $COMPILE_FLAGS $DEBUG_FLAGS -c $PLATFORM_SRC -o build/platform.o
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi

# Compile src/main/gui_main.c
echo -e "Compiling: src/main/gui_main.c"
$CC $COMPILE_FLAGS $DEBUG_FLAGS -c src/main/gui_main.c -o build/gui_main.o
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi

# Compile src/gui/gui.c
echo -e "Compiling: src/gui/gui.c"
$CC $COMPILE_FLAGS $DEBUG_FLAGS -c src/gui/gui.c -o build/gui.o
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi

# Link all GUI files
echo -e "Linking GUI binary..."
$CC build/gui_main.o build/gui.o build/memory.o build/platform.o -o system_monitor_gui $LD_FLAGS
if [ $? -ne 0 ]; then
    echo -e "${RED}Linking failed!${NC}"
    exit 1
fi

echo -e "${GREEN}GUI version build completed: system_monitor_gui${NC}"
echo -e "${YELLOW}To run the GUI version, use: ./system_monitor_gui${NC}" 