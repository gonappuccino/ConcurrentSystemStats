#!/bin/bash

# Color output definitions
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m' # Reset color

echo -e "${BOLD}System Monitoring GUI Build Helper${NC}"
echo -e "${BLUE}====================================${NC}"

# Check operating system
OS=$(uname -s)
echo -e "Operating System: ${BOLD}$OS${NC}"

# Check GTK+3 packages
if [ "$OS" = "Darwin" ]; then
    # macOS environment
    echo -e "\n${BOLD}Checking GTK+3 installation with HomeBrew...${NC}"
    if brew list gtk+3 &>/dev/null; then
        echo -e "${GREEN}GTK+3 is installed.${NC}"
    else
        echo -e "${YELLOW}GTK+3 is not installed. Attempting to install...${NC}"
        brew install gtk+3
    fi
    
    # Find pkg-config files
    echo -e "\n${BOLD}Looking for GTK+3 pkg-config files...${NC}"
    GTK_PC_PATH=$(find /usr/local/Cellar -name "gtk+-3.0.pc" -exec dirname {} \; 2>/dev/null | head -n 1)
    
    if [ -z "$GTK_PC_PATH" ]; then
        echo -e "${YELLOW}Cannot find gtk+-3.0.pc file.${NC}"
        exit 1
    else
        echo -e "${GREEN}pkg-config file path: $GTK_PC_PATH${NC}"
        
        # Set PKG_CONFIG_PATH environment variable
        export PKG_CONFIG_PATH="$GTK_PC_PATH:$PKG_CONFIG_PATH"
        echo -e "${BLUE}PKG_CONFIG_PATH set: $PKG_CONFIG_PATH${NC}"
    fi
    
elif [ "$OS" = "Linux" ]; then
    # Linux environment
    echo -e "\n${BOLD}Checking GTK+3 with package manager...${NC}"
    
    if command -v apt-get &>/dev/null; then
        # Debian/Ubuntu
        if dpkg -l libgtk-3-dev &>/dev/null; then
            echo -e "${GREEN}GTK+3 development package is installed.${NC}"
        else
            echo -e "${YELLOW}GTK+3 development package is not installed. Attempting to install...${NC}"
            sudo apt-get update && sudo apt-get install -y libgtk-3-dev
        fi
    elif command -v dnf &>/dev/null; then
        # Fedora
        if rpm -q gtk3-devel &>/dev/null; then
            echo -e "${GREEN}GTK+3 development package is installed.${NC}"
        else
            echo -e "${YELLOW}GTK+3 development package is not installed. Attempting to install...${NC}"
            sudo dnf install -y gtk3-devel
        fi
    fi
fi

# Check GTK+3 with pkg-config
echo -e "\n${BOLD}Checking GTK+3 with pkg-config...${NC}"
if pkg-config --exists gtk+-3.0; then
    echo -e "${GREEN}GTK+3 pkg-config check successful!${NC}"
    GTK_VERSION=$(pkg-config --modversion gtk+-3.0)
    echo -e "GTK+ version: ${BOLD}$GTK_VERSION${NC}"
    
    # Check GTK+3 compile flags
    GTK_CFLAGS=$(pkg-config --cflags gtk+-3.0)
    echo -e "GTK+ compile flags: ${BLUE}$GTK_CFLAGS${NC}"
    
    # Check GTK+3 link flags
    GTK_LIBS=$(pkg-config --libs gtk+-3.0)
    echo -e "GTK+ link flags: ${BLUE}$GTK_LIBS${NC}"
else
    echo -e "${YELLOW}GTK+3 not found in pkg-config. Check your path settings.${NC}"
    exit 1
fi

# Build
echo -e "\n${BOLD}Building GUI application...${NC}"
make clean
make debug
make gui

# Check build results
if [ -f "system_monitor_gui" ]; then
    echo -e "\n${GREEN}Build successful!${NC}"
    echo -e "Run the GUI application with: ${BOLD}./system_monitor_gui${NC}"
else
    echo -e "\n${YELLOW}Build failed!${NC}"
    echo -e "Check the error messages and take necessary actions."
fi 