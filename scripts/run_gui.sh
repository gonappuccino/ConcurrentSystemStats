#!/bin/bash

# Color settings
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m' # Reset color

echo -e "${BOLD}System Monitor GUI Execution Script${NC}"
echo -e "${BLUE}====================================${NC}"

# Check operating system
OS=$(uname -s)
echo -e "Operating System: ${BOLD}$OS${NC}"

# Check architecture
ARCH=$(uname -m)
echo -e "Architecture: ${BOLD}$ARCH${NC}"

# Perform clean build
echo -e "${BLUE}Performing clean build...${NC}"
make clean
if [ $? -ne 0 ]; then
    echo -e "${RED}Clean build failed${NC}"
    exit 1
fi

# Compile GUI
echo -e "${BLUE}Compiling GUI...${NC}"
./compile_gui.sh
if [ $? -ne 0 ]; then
    echo -e "${RED}GUI compilation failed${NC}"
    exit 1
fi

# Check GUI executable
if [ ! -f "system_monitor_gui" ]; then
    echo -e "${RED}Error: system_monitor_gui file does not exist!${NC}"
    echo -e "${YELLOW}Compilation may have failed.${NC}"
    exit 1
fi

# Set HomeBrew path
if [ "$OS" = "Darwin" ]; then
    if [ -d "/opt/homebrew" ]; then
        HOMEBREW_PREFIX="/opt/homebrew"
    else
        HOMEBREW_PREFIX="/usr/local"
    fi
fi

# Set required environment variables for macOS
if [ "$OS" = "Darwin" ]; then
    echo -e "${BLUE}Setting macOS environment variables...${NC}"
    
    # Set pkg-config path
    if [ -d "$HOMEBREW_PREFIX/lib/pkgconfig" ]; then
        export PKG_CONFIG_PATH="$HOMEBREW_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"
    fi
    
    # Set GTK module path
    if [ -d "$HOMEBREW_PREFIX/lib/gtk-3.0" ]; then
        export GTK_PATH="$HOMEBREW_PREFIX/lib/gtk-3.0"
    fi
    
    # Set library path
    export DYLD_LIBRARY_PATH="$HOMEBREW_PREFIX/lib:$DYLD_LIBRARY_PATH"
    
    # Set GTK debug level (debug mode only)
    if [ "$1" = "--debug" ]; then
        export GTK_DEBUG=interactive
    fi
    
    # Use Quartz backend (macOS default)
    export GDK_BACKEND=quartz
    
    # Set GTK rendering
    if [ "$ARCH" = "arm64" ]; then
        # Use Metal on Apple Silicon
        export GDK_CORE_DEVICE_EVENTS=1
        export GSK_RENDERER=cairo
        echo -e "${BLUE}Apple Silicon detected - Metal rendering enabled${NC}"
    fi
    
    # X11 backend configuration (optional if XQuartz is installed)
    if [ -d "/opt/X11" ] || [ -d "/usr/X11" ]; then
        echo -e "${YELLOW}XQuartz detected. Activate the following environment variables if needed:${NC}"
        echo -e "${YELLOW}  export GDK_BACKEND=x11${NC}"
        echo -e "${YELLOW}  export DISPLAY=:0${NC}"
    fi
    
    # Gtk settings
    export GTK_CSD=0
    export GTK2_RC_FILES=""
    export GTK_THEME="Adwaita:dark"  # Use dark theme (optional)
    
    echo -e "${GREEN}Environment variables set${NC}"
    
elif [ "$OS" = "Linux" ]; then
    echo -e "${BLUE}Setting Linux environment variables...${NC}"
    
    # Check X11 display
    if [ -z "$DISPLAY" ]; then
        echo -e "${YELLOW}DISPLAY environment variable not set. Setting to :0${NC}"
        export DISPLAY=:0
    fi
    
    # Enable debug if needed
    if [ "$1" = "--debug" ]; then
        export GTK_DEBUG=interactive
    fi
    
    echo -e "${GREEN}Environment variables set${NC}"
fi

# Make executable
chmod +x system_monitor_gui

# Run (without logs)
echo -e "${BLUE}Running GUI application...${NC}"
if [ "$1" = "--debug" ]; then
    # Run in debug mode (save logs)
    echo -e "${YELLOW}Debug mode activated - logs will be saved to gui_debug.log${NC}"
    ./system_monitor_gui 2>&1 | tee gui_debug.log
else
    # Run in normal mode
    ./system_monitor_gui
fi

# Check for errors
EXIT_CODE=${PIPESTATUS[0]}
if [ $EXIT_CODE -ne 0 ]; then
    echo -e "${RED}GUI application exited with an error. (Exit code: $EXIT_CODE)${NC}"
    echo -e "${YELLOW}For debug information, run:${NC}"
    echo -e "${BLUE}  ./run_gui.sh --debug${NC}"
else
    echo -e "${GREEN}GUI application terminated normally.${NC}"
fi 