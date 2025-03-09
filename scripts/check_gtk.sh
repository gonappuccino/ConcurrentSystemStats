#!/bin/bash

# Color settings
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m' # Reset color

echo -e "${BOLD}GTK+3 Environment Diagnostic Tool${NC}"
echo -e "${BLUE}====================================${NC}"

# System information
OS=$(uname -s)
ARCH=$(uname -m)
echo -e "Operating System: ${BOLD}$OS${NC}"
echo -e "Architecture: ${BOLD}$ARCH${NC}"

# Check HomeBrew
echo -e "\n${BOLD}Checking HomeBrew installation...${NC}"
if command -v brew >/dev/null 2>&1; then
    BREW_VERSION=$(brew --version | head -n 1)
    BREW_PREFIX=$(brew --prefix)
    echo -e "${GREEN}HomeBrew is installed: $BREW_VERSION${NC}"
    echo -e "${GREEN}HomeBrew path: $BREW_PREFIX${NC}"
else
    echo -e "${RED}HomeBrew is not installed.${NC}"
    echo -e "${YELLOW}Installing GTK+3 through HomeBrew is recommended on macOS.${NC}"
    echo -e "${YELLOW}Installation command: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\"${NC}"
fi

# Check GTK+3
echo -e "\n${BOLD}Checking GTK+3 installation...${NC}"
if pkg-config --exists gtk+-3.0 2>/dev/null; then
    GTK_VERSION=$(pkg-config --modversion gtk+-3.0)
    echo -e "${GREEN}GTK+3 is installed: version $GTK_VERSION${NC}"
    
    GTK_CFLAGS=$(pkg-config --cflags gtk+-3.0)
    echo -e "${GREEN}GTK+3 compile flags: $GTK_CFLAGS${NC}"
    
    GTK_LIBS=$(pkg-config --libs gtk+-3.0)
    echo -e "${GREEN}GTK+3 link flags: $GTK_LIBS${NC}"
    
    # GTK+3 pkg-config file location
    if [ "$OS" = "Darwin" ]; then
        GTK_PC_PATH=$(find /opt/homebrew /usr/local -name 'gtk+-3.0.pc' 2>/dev/null | head -n 1)
        if [ -n "$GTK_PC_PATH" ]; then
            echo -e "${GREEN}GTK+3 pkg-config file: $GTK_PC_PATH${NC}"
        else
            echo -e "${YELLOW}Cannot find GTK+3 pkg-config file.${NC}"
        fi
    else
        GTK_PC_PATH=$(pkg-config --variable=prefix gtk+-3.0)
        echo -e "${GREEN}GTK+3 installation path: $GTK_PC_PATH${NC}"
    fi
else
    echo -e "${RED}GTK+3 is not installed or cannot be detected by pkg-config.${NC}"
    
    if [ "$OS" = "Darwin" ]; then
        echo -e "${YELLOW}Installation command: brew install gtk+3${NC}"
    elif [ "$OS" = "Linux" ]; then
        if command -v apt-get >/dev/null 2>&1; then
            echo -e "${YELLOW}Installation command: sudo apt-get install libgtk-3-dev${NC}"
        elif command -v dnf >/dev/null 2>&1; then
            echo -e "${YELLOW}Installation command: sudo dnf install gtk3-devel${NC}"
        elif command -v pacman >/dev/null 2>&1; then
            echo -e "${YELLOW}Installation command: sudo pacman -S gtk3${NC}"
        fi
    fi
fi

# Check environment variables
echo -e "\n${BOLD}Checking environment variables...${NC}"
echo -e "PKG_CONFIG_PATH: ${BLUE}$PKG_CONFIG_PATH${NC}"
echo -e "GDK_BACKEND: ${BLUE}$GDK_BACKEND${NC}"
echo -e "DISPLAY: ${BLUE}$DISPLAY${NC}"

# Check XQuartz (macOS only)
if [ "$OS" = "Darwin" ]; then
    echo -e "\n${BOLD}Checking XQuartz...${NC}"
    if [ -d "/opt/X11" ] || [ -d "/usr/X11" ]; then
        echo -e "${GREEN}XQuartz is installed.${NC}"
    else
        echo -e "${YELLOW}XQuartz is not installed. Install if needed:${NC}"
        echo -e "${YELLOW}Installation command: brew install --cask xquartz${NC}"
    fi
fi

# Diagnostic summary
echo -e "\n${BOLD}Diagnostic Summary${NC}"
echo -e "${BLUE}====================================${NC}"

if pkg-config --exists gtk+-3.0 2>/dev/null; then
    echo -e "${GREEN}✓ GTK+3 is properly installed.${NC}"
    echo -e "${GREEN}✓ Compiler and linker can recognize GTK+3.${NC}"
    
    if [ -f "system_monitor_gui" ]; then
        echo -e "${GREEN}✓ GUI application is built.${NC}"
        echo -e "${GREEN}✓ Use ./run_gui.sh to run.${NC}"
    else
        echo -e "${YELLOW}! GUI application is not built yet.${NC}"
        echo -e "${YELLOW}! Run ./compile_gui.sh to build.${NC}"
    fi
else
    echo -e "${RED}✗ GTK+3 is not properly installed or not recognized by pkg-config.${NC}"
    echo -e "${YELLOW}! Refer to the installation instructions above.${NC}"
fi

echo -e "\n${BLUE}Troubleshooting steps:${NC}"
echo -e "1. Run ${YELLOW}compile_gui.sh${NC} to check build errors"
echo -e "2. Run ${YELLOW}run_gui.sh --debug${NC} to see debug information"
echo -e "3. ${YELLOW}export PKG_CONFIG_PATH=\"\$(dirname \$(find /opt/homebrew /usr/local -name 'gtk+-3.0.pc' 2>/dev/null | head -n 1)):\$PKG_CONFIG_PATH\"${NC}"
echo -e "   Use the above command to manually set pkg-config path" 