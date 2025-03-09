# System Monitoring Application Makefile
# Support for both CLI and GUI versions

# Compiler and basic flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Isrc/utils -Isrc/core -Isrc/gui -Isrc/platform -Isrc/main
LDFLAGS = -lm

# Build directory
BUILD_DIR = build

# Common source files
COMMON_SRCS = src/core/cpu.c src/core/memory.c src/core/system.c src/core/user.c src/utils/error.c

# Platform detection and settings
UNAME_S := $(shell uname -s)

# GTK path settings on macOS (HomeBrew)
ifeq ($(UNAME_S),Darwin)
    # Find pkg-config path on HomeBrew macOS
    PKG_CONFIG_PATH := $(shell find /usr/local/Cellar -name "gtk+-3.0.pc" -exec dirname {} \; 2>/dev/null)
    export PKG_CONFIG_PATH
    PLATFORM_SRC = src/platform/platform_mac.c
    PLATFORM_FLAGS = -D_DARWIN_C_SOURCE
else
    PLATFORM_SRC = src/platform/platform_linux.c
    PLATFORM_FLAGS =
endif

# Check GTK+ availability (with debug messages)
PKG_CONFIG_CMD = pkg-config --exists gtk+-3.0
PKG_CONFIG_STATUS := $(shell $(PKG_CONFIG_CMD) && echo $$?)

ifeq ($(PKG_CONFIG_STATUS),0)
    HAS_GTK3 := 1
    GTK_FLAGS := $(shell pkg-config --cflags --libs gtk+-3.0)
    GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
    GTK_LIBS := $(shell pkg-config --libs gtk+-3.0)
    GTK_AVAIL := 1
    # Define HAVE_GTK macro when GTK is available
    CFLAGS += -DHAVE_GTK
else
    HAS_GTK3 := 0
    GTK_AVAIL := 0
    $(warning pkg-config command: $(PKG_CONFIG_CMD))
    $(warning PKG_CONFIG_PATH: $(PKG_CONFIG_PATH))
    $(warning Result code: $(PKG_CONFIG_STATUS))
    $(warning GTK+3 library not found. GUI version will not be built.)
endif

# CLI and GUI source files with updated paths
CLI_SRCS = src/main/main.c $(COMMON_SRCS) $(PLATFORM_SRC)
GUI_SRCS = src/main/gui_main.c src/gui/gui.c src/gui/gui_utils.c $(COMMON_SRCS) $(PLATFORM_SRC)

# Object files (created in build directory)
CLI_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(CLI_SRCS))
GUI_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(GUI_SRCS))

# Output binaries
CLI_BIN = system_monitor_cli
GUI_BIN = system_monitor_gui

# Color output (for better readability in terminal)
BOLD = \033[1m
GREEN = \033[32m
BLUE = \033[34m
YELLOW = \033[33m
RESET = \033[0m

# Default target (different handling based on GTK+ availability)
ifeq ($(GTK_AVAIL),1)
all: setup cli gui
else
all: setup cli
endif

# CLI version build
cli: setup $(CLI_BIN)

# GUI version build (only with GTK+ installed)
ifeq ($(GTK_AVAIL),1)
gui: setup $(GUI_BIN)
else
gui:
	@echo "$(YELLOW)GTK+3 library is not installed.$(RESET)"
	@echo "$(YELLOW)Skipping GUI version build.$(RESET)"
	@echo "$(YELLOW)To build the GUI version, install GTK+3:$(RESET)"
	@echo "$(YELLOW)  - macOS: brew install gtk+3$(RESET)"
	@echo "$(YELLOW)  - Ubuntu/Debian: sudo apt install libgtk-3-dev$(RESET)"
	@echo "$(YELLOW)  - Fedora: sudo dnf install gtk3-devel$(RESET)"
	@echo "$(YELLOW)After installation, run the following command to check the pkg-config path:$(RESET)"
	@echo "$(YELLOW)  find /usr/local/Cellar -name \"gtk+-3.0.pc\" -exec dirname {} \;$(RESET)"
	@echo "$(YELLOW)  export PKG_CONFIG_PATH=<path from command above>$(RESET)"
endif

# Create build directory
setup:
	@mkdir -p $(BUILD_DIR)/src/core
	@mkdir -p $(BUILD_DIR)/src/gui
	@mkdir -p $(BUILD_DIR)/src/platform
	@mkdir -p $(BUILD_DIR)/src/utils
	@mkdir -p $(BUILD_DIR)/src/main
	@echo "$(BLUE)Build environment ready ($(UNAME_S) platform)$(RESET)"

# Link CLI binary
$(CLI_BIN): $(CLI_OBJS)
	@echo "$(BOLD)Linking CLI binary...$(RESET)"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "$(GREEN)CLI version build complete: $@$(RESET)"

# Link GUI binary (only with GTK+ installed)
ifeq ($(GTK_AVAIL),1)
$(GUI_BIN): $(GUI_OBJS)
	@echo "$(BOLD)Linking GUI binary...$(RESET)"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(GTK_LIBS)
	@echo "$(GREEN)GUI version build complete: $@$(RESET)"
endif

# Object file compilation rule
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) $(if $(findstring gui,$<),$(GTK_CFLAGS),) -c $< -o $@

# Clean up: remove all generated files
clean:
	@rm -rf $(BUILD_DIR) $(CLI_BIN) $(GUI_BIN)
	@echo "$(GREEN)Build files cleaned up$(RESET)"

# Run CLI version
run-cli: cli
	@echo "$(BOLD)Running CLI version...$(RESET)"
	@./$(CLI_BIN)

# Run GUI version
ifeq ($(GTK_AVAIL),1)
run-gui: gui
	@echo "$(BOLD)Running GUI version...$(RESET)"
	@./$(GUI_BIN)
else
run-gui:
	@echo "$(YELLOW)GTK+3 library is not installed, cannot run GUI version.$(RESET)"
endif

# Install (common for Linux/macOS)
install: all
	@echo "$(BOLD)Installing to system...$(RESET)"
	@install -m 755 $(CLI_BIN) /usr/local/bin/
	@if [ -f $(GUI_BIN) ]; then \
		install -m 755 $(GUI_BIN) /usr/local/bin/; \
		echo "$(GREEN)Installation complete - you can run with the following commands:$(RESET)"; \
		echo "  $(CLI_BIN)"; \
		echo "  $(GUI_BIN)"; \
	else \
		echo "$(GREEN)CLI version only installed - you can run with the following command:$(RESET)"; \
		echo "  $(CLI_BIN)"; \
	fi

# Uninstall (common for Linux/macOS)
uninstall:
	@echo "$(BOLD)Removing from system...$(RESET)"
	@rm -f /usr/local/bin/$(CLI_BIN)
	@rm -f /usr/local/bin/$(GUI_BIN)
	@echo "$(GREEN)Uninstallation complete$(RESET)"

# .PHONY targets: prevent conflicts with filenames
.PHONY: all cli gui clean setup install uninstall run-cli run-gui

# Debug information (for troubleshooting build issues)
debug:
	@echo "$(BOLD)Build configuration information:$(RESET)"
	@echo "Operating system: $(UNAME_S)"
	@echo "pkg-config command: pkg-config --exists gtk+-3.0"
	@echo "pkg-config status code: $(PKG_CONFIG_STATUS)"
	@echo "PKG_CONFIG_PATH: $(PKG_CONFIG_PATH)"
	@echo "GTK+3 available: $(GTK_AVAIL)"
	@echo "HAS_GTK3: $(HAS_GTK3)"
	@echo "GTK+3 CFLAGS: $(GTK_CFLAGS)"
	@echo "GTK+3 LIBS: $(GTK_LIBS)"
	@echo "Platform source: $(PLATFORM_SRC)"
	@echo "Compiler flags: $(CFLAGS)"
	@echo "Linker flags: $(LDFLAGS)"
	@echo "CLI sources: $(CLI_SRCS)"
	@echo "CLI objects: $(CLI_OBJS)"
	@echo "GUI sources: $(GUI_SRCS)"
	@echo "GUI objects: $(GUI_OBJS)"
	@pkg-config --exists gtk+-3.0 && echo "GTK+3 pkg-config success" || echo "GTK+3 pkg-config failed: $$?"
