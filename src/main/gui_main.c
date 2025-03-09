#include "common.h"
#include "gui.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Print debug message
    printf("Starting GUI application...\n");
    
#ifdef HAVE_GTK
    printf("GTK+ library detected. Initializing GUI...\n");
    
    // Initialize and run GUI mode
    init_gui(&argc, &argv);
    run_gui();
    cleanup_gui();
    return 0;
#else
    // Print error message when GTK+ is not available
    fprintf(stderr, "Error: GTK+3 library is not installed.\n");
    fprintf(stderr, "To use the GUI version, install GTK+3:\n");
    fprintf(stderr, "  - macOS: brew install gtk+3\n");
    fprintf(stderr, "  - Ubuntu/Debian: sudo apt install libgtk-3-dev\n");
    fprintf(stderr, "  - Fedora: sudo dnf install gtk3-devel\n");
    fprintf(stderr, "\nAfter installation, run the following command to check pkg-config path:\n");
    fprintf(stderr, "  find /usr/local/Cellar -name \"gtk+-3.0.pc\" -exec dirname {} \\;\n");
    fprintf(stderr, "  export PKG_CONFIG_PATH=<path from above command>\n");
    return 1;
#endif
} 