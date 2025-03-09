# Concurrent System Stats

A multi-process system monitoring tool with both CLI and GUI interfaces.

## Project Structure

The project is organized into the following directories:

```
ConcurrentSystemStats/
├── src/                    # Source code
│   ├── core/               # Core monitoring functionality
│   │   ├── cpu.c/h         # CPU monitoring
│   │   ├── memory.c/h      # Memory monitoring
│   │   ├── system.c/h      # System information
│   │   └── user.c/h        # User session monitoring
│   ├── gui/                # GUI-related code
│   │   ├── gui.c/h         # Main GUI implementation
│   │   └── gui_utils.c/h   # GUI utility functions
│   ├── platform/           # Platform-specific implementations
│   │   ├── platform.h      # Common platform interface
│   │   ├── platform_linux.c # Linux-specific implementation
│   │   └── platform_mac.c  # macOS-specific implementation
│   ├── utils/              # Utility functions
│   │   ├── common.h        # Common definitions
│   │   └── error.c/h       # Error handling
│   └── main/               # Entry points
│       ├── main.c          # CLI entry point
│       └── gui_main.c      # GUI entry point
├── scripts/                # Build and run scripts
├── logs/                   # Log files
└── build/                  # Build artifacts
```

## Requirements

- C compiler (GCC or Clang)
- For GUI version: GTK+3 development libraries

### Installing Dependencies

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

## Building

### Using Make

Build both CLI and GUI versions (if GTK is available):

```bash
make
```

Build only CLI version:

```bash
make cli
```

Build only GUI version:

```bash
make gui
```

### Using Scripts

Build the GUI version:

```bash
./scripts/compile_gui.sh
```

## Running

### CLI Version

```bash
./system_monitor_cli [options]
```

Options:
- `--samples=N`: Number of samples to collect (default: 10)
- `--tdelay=N`: Delay between samples in seconds (default: 1)
- `--system`: Display only system information
- `--user`: Display only user information
- `--graphics`: Enable graphical output in CLI
- `--sequential`: Use sequential output mode

### GUI Version

```bash
./system_monitor_gui
```

Or use the provided run script:

```bash
./scripts/run_gui.sh
```

For debugging:

```bash
./scripts/run_gui.sh --debug
```

## Features

- Real-time CPU usage monitoring
- Memory usage tracking
- System information display
- User session monitoring
- Graphical visualization (in GUI version)
- Multi-process data collection for improved performance

## Platform Support

- Linux
- macOS 