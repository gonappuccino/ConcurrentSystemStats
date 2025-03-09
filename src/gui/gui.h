#ifndef GUI_H
#define GUI_H

#include "common.h"

// Include GTK+ headers (conditionally)
#ifdef HAVE_GTK
#include <gtk/gtk.h>
#include <time.h>
#include <unistd.h>

/**
 * VIM color theme structure
 * 
 * Defines a color theme similar to the VIM editor.
 * Used to provide visual consistency in the GUI.
 */
typedef struct {
    GdkRGBA background;   // Background color (#1c1c1c - dark gray)
    GdkRGBA foreground;   // Foreground color (#d0d0d0 - light gray)
    GdkRGBA comment;      // Comment color (#808080 - gray)
    GdkRGBA keyword;      // Keyword color (#5f87d7 - blue)
    GdkRGBA string;       // String color (#87af5f - green)
    GdkRGBA warning;      // Warning color (#d75f5f - red)
    GdkRGBA special;      // Special color (#d78700 - orange)
} VimColorTheme;

/**
 * GUI widgets structure
 * 
 * Stores pointers to all GTK widgets used in the GUI.
 * Allows access to widgets from GUI update functions.
 */
typedef struct {
    GtkWidget *window;
    GtkWidget *main_box;
    
    // Tab container
    GtkWidget *notebook;
    
    // Dashboard tab widgets
    GtkWidget *dashboard_system_info;
    GtkWidget *dashboard_cpu_label;
    GtkWidget *dashboard_cpu_bar;
    GtkWidget *dashboard_cpu_graph;
    GtkWidget *dashboard_memory_label;
    GtkWidget *dashboard_memory_bar;
    GtkWidget *dashboard_memory_graph;
    GtkWidget *dashboard_swap_label;
    GtkWidget *dashboard_swap_bar;
    GtkWidget *dashboard_swap_graph;
    GtkWidget *dashboard_users_list;
    
    // System tab widgets
    GtkWidget *system_box;
    GtkWidget *system_info_label;
    
    // CPU tab widgets
    GtkWidget *cpu_box;
    GtkWidget *cpu_usage_label;
    GtkWidget *cpu_usage_bar;
    GtkWidget *cpu_usage_graph;
    
    // Memory tab widgets
    GtkWidget *memory_box;
    GtkWidget *memory_usage_label;
    GtkWidget *memory_usage_bar;
    GtkWidget *memory_usage_graph;
    GtkWidget *swap_usage_label;
    GtkWidget *swap_usage_bar;
    GtkWidget *swap_usage_graph;
    
    // User sessions tab widgets
    GtkWidget *users_box;
    GtkWidget *users_list;
    
    // Status bar
    GtkWidget *statusbar;
    guint statusbar_context_id;
} GuiWidgets;

/**
 * GUI data structure
 * 
 * Stores system monitoring data to be displayed in the GUI.
 * This structure holds system information collected in the background for GUI display.
 */
typedef struct {
    // CPU data
    double cpu_usage;
    float *cpu_history;
    int cpu_history_size;
    
    // Memory data
    double memory_total;
    double memory_used;
    double *memory_history;
    int memory_history_size;
    
    // Swap data
    double swap_total;
    double swap_used;
    double *swap_history;
    int swap_history_size;
    
    // System information
    char *system_name;
    char *node_name;
    char *release;
    char *version;
    char *machine;
    int uptime_days;
    int uptime_hours;
    int uptime_minutes;
    int uptime_seconds;
    
    // User session data
    char **users;
    int user_count;
    
    // Update interval (milliseconds)
    guint update_interval;
} GuiData;

/**
 * Structure to hold system data for GUI
 */
typedef struct {
    int pipe_fd[2];                      // Pipe file descriptors for IPC
    GtkWidget *cpuLabel;                 // Label for CPU usage display
    GtkWidget *memoryLabel;              // Label for memory usage display
    unsigned long prevCpuUsage[7];       // Previous CPU usage statistics
    unsigned long currCpuUsage[7];       // Current CPU usage statistics
    double prevMemoryUsage;              // Previous memory usage in GB
    double currMemoryUsage;              // Current memory usage in GB
    double totalMemory;                  // Total system memory in GB
} SystemData;

// GUI function declarations
void init_gui(int *argc, char ***argv);
void run_gui(void);
void update_gui_data(GuiData *data);
void cleanup_gui(void);

// VIM theme related functions
void apply_vim_theme(GtkWidget *widget);
VimColorTheme *get_vim_theme(void);

// Data collection and update functions
gboolean update_system_data(gpointer user_data);
void update_cpu_display(GuiWidgets *widgets, GuiData *data);
void update_memory_display(GuiWidgets *widgets, GuiData *data);
void update_system_info_display(GuiWidgets *widgets, GuiData *data);
void update_users_display(GuiWidgets *widgets, GuiData *data);

// Graph drawing functions
gboolean draw_cpu_graph(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean draw_memory_graph(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean draw_swap_graph(GtkWidget *widget, cairo_t *cr, gpointer data);

// Event handlers
void on_window_destroy(GtkWidget *widget, gpointer data);

/**
 * Function to create the main GUI window
 * @param pipe_fd Pipe file descriptor array
 */
void createGUI(int pipe_fd[2]);

/**
 * Function to update the CPU usage display
 * @param label GTK label to update
 * @param prevCpuUsage Previous CPU usage statistics
 * @param currCpuUsage Current CPU usage statistics
 */
void updateCPUDisplay(GtkWidget *label, unsigned long prevCpuUsage[7], unsigned long currCpuUsage[7]);

/**
 * Function to update system data
 * @param data Pointer to the SystemData structure
 * @return TRUE to keep the timer running
 */
gboolean updateSystemData(gpointer data);

/**
 * Function to create system statistics widgets
 * @param sysData Pointer to the SystemData structure
 * @return GtkWidget* Pointer to the created system statistics widget
 */
GtkWidget* createSystemStatsWidget(SystemData *sysData);

#else
// Minimal implementation when GTK+ is not available
typedef struct {
    int dummy;
} GuiData;

// Empty implementation functions
static inline void init_gui(int *argc, char ***argv) { 
    (void)argc; (void)argv; 
    fprintf(stderr, "Cannot initialize GUI: GTK+3 library is not available.\n");
}
static inline void run_gui(void) { 
    fprintf(stderr, "Cannot run GUI: GTK+3 library is not available.\n");
}
static inline void cleanup_gui(void) { }

#endif // HAVE_GTK

#endif // GUI_H 