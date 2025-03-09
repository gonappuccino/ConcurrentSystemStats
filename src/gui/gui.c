#include "gui.h"
#include <stdio.h>
#include "gui_utils.h"
#include "error.h"

#ifdef HAVE_GTK
#include "cpu.h"
#include "memory.h"
#include "system.h"
#include "user.h"
#include "platform.h"
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <cairo.h>
#include <math.h>

// Global variables
static GuiWidgets widgets;
static GuiData gui_data;
static VimColorTheme vim_theme;

/**
 * Initialize VIM color theme
 */
void init_vim_theme(void) {
    // Background color - darker (#121212 instead of #1c1c1c)
    vim_theme.background = (GdkRGBA){0.07, 0.07, 0.07, 1.0};
    
    // Foreground color - light gray (#d0d0d0)
    vim_theme.foreground = (GdkRGBA){0.816, 0.816, 0.816, 1.0};
    
    // Comment color - gray (#808080)
    vim_theme.comment = (GdkRGBA){0.5, 0.5, 0.5, 1.0};
    
    // Keyword color - blue (#5f87d7)
    vim_theme.keyword = (GdkRGBA){0.373, 0.529, 0.843, 1.0};
    
    // String color - green (#87af5f)
    vim_theme.string = (GdkRGBA){0.529, 0.686, 0.373, 1.0};
    
    // Warning color - red (#d75f5f)
    vim_theme.warning = (GdkRGBA){0.843, 0.373, 0.373, 1.0};
    
    // Special color - orange (#d78700)
    vim_theme.special = (GdkRGBA){0.843, 0.529, 0.0, 1.0};
}

/**
 * GUI initialization function
 */
void init_gui(int *argc, char ***argv) {
    LOG_INFO(SYS_MON_SUCCESS, "Starting GUI initialization...");
    
    // Initialize GTK
    gtk_init(argc, argv);
    LOG_INFO(SYS_MON_SUCCESS, "GTK initialization complete");
    
    // Initialize VIM theme
    init_vim_theme();
    LOG_INFO(SYS_MON_SUCCESS, "VIM theme initialization complete");
    
    // Create main window
    widgets.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (widgets.window == NULL) {
        LOG_FATAL(SYS_MON_ERR_GTK, "Failed to create main window");
    }
    LOG_INFO(SYS_MON_SUCCESS, "Main window creation complete");
    
    gtk_window_set_title(GTK_WINDOW(widgets.window), "System Monitor (VIM Theme)");
    gtk_window_set_default_size(GTK_WINDOW(widgets.window), 1024, 768);
    g_signal_connect(widgets.window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    // Create main box using utility function
    ContainerOptions main_box_options = create_default_container_options();
    main_box_options.spacing = 5;
    widgets.main_box = create_box(GTK_ORIENTATION_VERTICAL, &main_box_options);
    gtk_container_add(GTK_CONTAINER(widgets.window), widgets.main_box);
    LOG_INFO(SYS_MON_SUCCESS, "Main box creation complete");
    
    // Create notebook (tabs)
    widgets.notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(widgets.main_box), widgets.notebook, TRUE, TRUE, 0);
    
    // --- Dashboard tab ---
    ContainerOptions dashboard_options = create_default_container_options();
    dashboard_options.spacing = 10;
    dashboard_options.border_width = 15;
    GtkWidget *dashboard_box = create_box(GTK_ORIENTATION_VERTICAL, &dashboard_options);
    
    // Dashboard title using utility function
    LabelOptions title_options = create_default_label_options();
    title_options.font_desc = "Monospace Bold 16";
    GtkWidget *dashboard_title = create_markup_label("<span>System Dashboard</span>", &title_options);
    gtk_widget_set_halign(dashboard_title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(dashboard_box), dashboard_title, FALSE, FALSE, 10);
    
    // Create dashboard grid
    GtkWidget *dashboard_grid = create_dashboard_grid(15, 15);
    gtk_box_pack_start(GTK_BOX(dashboard_box), dashboard_grid, TRUE, TRUE, 0);
    
    // --- System card ---
    // Create system info content area
    widgets.dashboard_system_info = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.dashboard_system_info), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.dashboard_system_info), 0.0);
    
    // Create system card using utility function
    GtkWidget *system_card = create_card("System Info", "#5f87d7", widgets.dashboard_system_info);
    
    // Add to dashboard
    add_card_to_dashboard(dashboard_grid, system_card, 0, 0, 1, 1);
    
    // --- CPU card ---
    GtkWidget *cpu_card = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(cpu_card), GTK_SHADOW_ETCHED_IN);
    GtkWidget *cpu_card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(cpu_card_box), 10);
    gtk_container_add(GTK_CONTAINER(cpu_card), cpu_card_box);
    
    // CPU card header
    GtkWidget *cpu_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(cpu_header), 
                        "<span font_desc='Monospace Bold 12' foreground='#5f87d7'>CPU Usage</span>");
    gtk_widget_set_halign(cpu_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(cpu_card_box), cpu_header, FALSE, FALSE, 0);
    
    // CPU usage label
    widgets.dashboard_cpu_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.dashboard_cpu_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.dashboard_cpu_label), 0.0);
    gtk_box_pack_start(GTK_BOX(cpu_card_box), widgets.dashboard_cpu_label, FALSE, FALSE, 5);
    
    // CPU usage progress bar
    widgets.dashboard_cpu_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(cpu_card_box), widgets.dashboard_cpu_bar, FALSE, FALSE, 5);
    
    // CPU usage mini graph
    widgets.dashboard_cpu_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(widgets.dashboard_cpu_graph, -1, 100);
    g_signal_connect(widgets.dashboard_cpu_graph, "draw", G_CALLBACK(draw_cpu_graph), &gui_data);
    gtk_box_pack_start(GTK_BOX(cpu_card_box), widgets.dashboard_cpu_graph, TRUE, TRUE, 0);
    
    // --- Memory card ---
    GtkWidget *memory_card = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(memory_card), GTK_SHADOW_ETCHED_IN);
    GtkWidget *memory_card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(memory_card_box), 10);
    gtk_container_add(GTK_CONTAINER(memory_card), memory_card_box);
    
    // Memory card header
    GtkWidget *memory_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(memory_header), 
                        "<span font_desc='Monospace Bold 12' foreground='#87af5f'>Memory Usage</span>");
    gtk_widget_set_halign(memory_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(memory_card_box), memory_header, FALSE, FALSE, 0);
    
    // Memory usage label
    widgets.dashboard_memory_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.dashboard_memory_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.dashboard_memory_label), 0.0);
    gtk_box_pack_start(GTK_BOX(memory_card_box), widgets.dashboard_memory_label, FALSE, FALSE, 5);
    
    // Memory usage progress bar
    widgets.dashboard_memory_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(memory_card_box), widgets.dashboard_memory_bar, FALSE, FALSE, 5);
    
    // Memory usage mini graph
    widgets.dashboard_memory_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(widgets.dashboard_memory_graph, -1, 100);
    g_signal_connect(widgets.dashboard_memory_graph, "draw", G_CALLBACK(draw_memory_graph), &gui_data);
    gtk_box_pack_start(GTK_BOX(memory_card_box), widgets.dashboard_memory_graph, TRUE, TRUE, 0);
    
    // --- Swap card ---
    GtkWidget *swap_card = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(swap_card), GTK_SHADOW_ETCHED_IN);
    GtkWidget *swap_card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(swap_card_box), 10);
    gtk_container_add(GTK_CONTAINER(swap_card), swap_card_box);
    
    // Swap card header
    GtkWidget *swap_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(swap_header), 
                        "<span font_desc='Monospace Bold 12' foreground='#d78700'>Swap Usage</span>");
    gtk_widget_set_halign(swap_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(swap_card_box), swap_header, FALSE, FALSE, 0);
    
    // Swap usage label
    widgets.dashboard_swap_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.dashboard_swap_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.dashboard_swap_label), 0.0);
    gtk_box_pack_start(GTK_BOX(swap_card_box), widgets.dashboard_swap_label, FALSE, FALSE, 5);
    
    // Swap usage progress bar
    widgets.dashboard_swap_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(swap_card_box), widgets.dashboard_swap_bar, FALSE, FALSE, 5);
    
    // Swap usage mini graph
    widgets.dashboard_swap_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(widgets.dashboard_swap_graph, -1, 100);
    g_signal_connect(widgets.dashboard_swap_graph, "draw", G_CALLBACK(draw_swap_graph), &gui_data);
    gtk_box_pack_start(GTK_BOX(swap_card_box), widgets.dashboard_swap_graph, TRUE, TRUE, 0);
    
    // --- Users card ---
    GtkWidget *users_card = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(users_card), GTK_SHADOW_ETCHED_IN);
    GtkWidget *users_card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(users_card_box), 10);
    gtk_container_add(GTK_CONTAINER(users_card), users_card_box);
    
    // Users card header
    GtkWidget *users_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(users_header), 
                        "<span font_desc='Monospace Bold 12' foreground='#d78700'>User Sessions</span>");
    gtk_widget_set_halign(users_header, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(users_card_box), users_header, FALSE, FALSE, 0);
    
    // Users list scroll window
    GtkWidget *dashboard_users_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboard_users_scroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(users_card_box), dashboard_users_scroll, TRUE, TRUE, 5);
    
    widgets.dashboard_users_list = gtk_tree_view_new();
    
    // Set the TreeView style - replaced with CSS style
    GtkStyleContext *tree_context = gtk_widget_get_style_context(widgets.dashboard_users_list);
    gtk_style_context_add_class(tree_context, "dark-bg");
    
    gtk_container_add(GTK_CONTAINER(dashboard_users_scroll), widgets.dashboard_users_list);
    
    // Arrange cards in grid (2x3 grid)
    gtk_grid_attach(GTK_GRID(dashboard_grid), system_card, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(dashboard_grid), cpu_card, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(dashboard_grid), memory_card, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(dashboard_grid), swap_card, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(dashboard_grid), users_card, 0, 2, 2, 1);
    
    // Add dashboard tab
    GtkWidget *dashboard_label = gtk_label_new("Dashboard");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets.notebook), dashboard_box, dashboard_label);
    
    // --- System tab ---
    widgets.system_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(widgets.system_box), 10);
    
    // System information label
    widgets.system_info_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.system_info_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.system_info_label), 0.0);
    gtk_box_pack_start(GTK_BOX(widgets.system_box), widgets.system_info_label, FALSE, FALSE, 0);
    
    // Add system tab
    GtkWidget *system_label = gtk_label_new("System");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets.notebook), widgets.system_box, system_label);
    
    // --- CPU tab ---
    widgets.cpu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(widgets.cpu_box), 10);
    
    // CPU usage label
    widgets.cpu_usage_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.cpu_usage_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.cpu_usage_label), 0.0);
    gtk_box_pack_start(GTK_BOX(widgets.cpu_box), widgets.cpu_usage_label, FALSE, FALSE, 0);
    
    // CPU usage progress bar
    widgets.cpu_usage_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(widgets.cpu_box), widgets.cpu_usage_bar, FALSE, FALSE, 0);
    
    // CPU usage graph
    widgets.cpu_usage_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(widgets.cpu_usage_graph, -1, 300);
    g_signal_connect(widgets.cpu_usage_graph, "draw", 
                    G_CALLBACK(draw_cpu_graph), &gui_data);
    gtk_box_pack_start(GTK_BOX(widgets.cpu_box), widgets.cpu_usage_graph, TRUE, TRUE, 0);
    
    // Add CPU tab
    GtkWidget *cpu_label = gtk_label_new("CPU");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets.notebook), widgets.cpu_box, cpu_label);
    
    // --- Memory tab ---
    widgets.memory_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(widgets.memory_box), 10);
    
    // Memory usage label
    widgets.memory_usage_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.memory_usage_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.memory_usage_label), 0.0);
    gtk_box_pack_start(GTK_BOX(widgets.memory_box), widgets.memory_usage_label, FALSE, FALSE, 0);
    
    // Memory usage progress bar
    widgets.memory_usage_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(widgets.memory_box), widgets.memory_usage_bar, FALSE, FALSE, 0);
    
    // Memory usage graph
    widgets.memory_usage_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(widgets.memory_usage_graph, -1, 200);
    g_signal_connect(widgets.memory_usage_graph, "draw", 
                    G_CALLBACK(draw_memory_graph), &gui_data);
    gtk_box_pack_start(GTK_BOX(widgets.memory_box), widgets.memory_usage_graph, TRUE, TRUE, 0);
    
    // Swap usage label
    widgets.swap_usage_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(widgets.swap_usage_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(widgets.swap_usage_label), 0.0);
    gtk_box_pack_start(GTK_BOX(widgets.memory_box), widgets.swap_usage_label, FALSE, FALSE, 10);
    
    // Swap usage progress bar
    widgets.swap_usage_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(widgets.memory_box), widgets.swap_usage_bar, FALSE, FALSE, 0);
    
    // Swap usage graph
    widgets.swap_usage_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(widgets.swap_usage_graph, -1, 200);
    g_signal_connect(widgets.swap_usage_graph, "draw", 
                    G_CALLBACK(draw_swap_graph), &gui_data);
    gtk_box_pack_start(GTK_BOX(widgets.memory_box), widgets.swap_usage_graph, TRUE, TRUE, 0);
    
    // Add memory tab
    GtkWidget *memory_label = gtk_label_new("Memory");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets.notebook), widgets.memory_box, memory_label);
    
    // --- Users tab ---
    widgets.users_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(widgets.users_box), 10);
    
    // Users list scroll window
    GtkWidget *users_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(users_scroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(widgets.users_box), users_scroll, TRUE, TRUE, 0);
    
    widgets.users_list = gtk_tree_view_new();
    
    // Set the TreeView style - replaced with CSS style
    tree_context = gtk_widget_get_style_context(widgets.users_list);
    gtk_style_context_add_class(tree_context, "dark-bg");
    
    gtk_container_add(GTK_CONTAINER(users_scroll), widgets.users_list);
    
    // Add users tab
    GtkWidget *users_label = gtk_label_new("Users");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets.notebook), widgets.users_box, users_label);
    
    // Status bar
    widgets.statusbar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(widgets.main_box), widgets.statusbar, FALSE, FALSE, 0);
    widgets.statusbar_context_id = gtk_statusbar_get_context_id(
        GTK_STATUSBAR(widgets.statusbar), "System Monitor");
    
    // Activate first tab (Dashboard)
    gtk_notebook_set_current_page(GTK_NOTEBOOK(widgets.notebook), 0);
    
    // Initialize GUI data
    gui_data.update_interval = 1000; // 1 second
    
    // Apply VIM theme
    apply_vim_theme(widgets.window);
    LOG_INFO(SYS_MON_SUCCESS, "VIM theme applied");
    
    // Add dark-bg class to the TreeView style context (already added above, removed duplication)
    GtkStyleContext *users_style = gtk_widget_get_style_context(widgets.users_list);
    gtk_style_context_add_class(users_style, "dark-bg");
    
    GtkStyleContext *dashboard_users_style = gtk_widget_get_style_context(widgets.dashboard_users_list);
    gtk_style_context_add_class(dashboard_users_style, "dark-bg");
    
    // Set timer
    g_timeout_add(gui_data.update_interval, update_system_data, &gui_data);
    LOG_INFO(SYS_MON_SUCCESS, "Timer set");
    
    // First update execution
    update_system_data(&gui_data);
    LOG_INFO(SYS_MON_SUCCESS, "First data update complete");
    
    LOG_INFO(SYS_MON_SUCCESS, "GUI initialization complete.");
}

/**
 * GUI execution function
 */
void run_gui(void) {
    LOG_INFO(SYS_MON_SUCCESS, "Running GUI...");
    
    // Run GTK main loop
    gtk_widget_show_all(widgets.window);
    gtk_main();
}

/**
 * GUI cleanup function
 */
void cleanup_gui(void) {
    LOG_INFO(SYS_MON_SUCCESS, "Cleaning up GUI resources...");
    
    // Free memory
    free(gui_data.cpu_history);
    free(gui_data.memory_history);
    free(gui_data.swap_history);
    
    free(gui_data.system_name);
    free(gui_data.node_name);
    free(gui_data.version);
    free(gui_data.release);
    free(gui_data.machine);
    
    if (gui_data.users != NULL) {
        for (int i = 0; i < gui_data.user_count; i++) {
            free(gui_data.users[i]);
        }
        free(gui_data.users);
    }
    
    LOG_INFO(SYS_MON_SUCCESS, "GUI resources cleaned up.");
}

/**
 * VIM theme getter function
 */
VimColorTheme *get_vim_theme(void) {
    return &vim_theme;
}

/**
 * Apply VIM theme CSS to widget
 */
void apply_vim_theme(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    
    const char *css = 
        "window, notebook, box, scrolledwindow, grid, viewport { "
        "   background-color: #121212; "
        "   color: #d0d0d0; "
        "}"
        "label { "
        "   color: #d0d0d0; "
        "   font-family: 'Monospace'; "
        "   background-color: transparent; "
        "}"
        "notebook tab { "
        "   background-color: #1a1a1a; "
        "   color: #d0d0d0; "
        "   border: 1px solid #333333; "
        "   padding: 4px 8px; "
        "   font-weight: bold; "
        "}"
        "notebook tab:active { "
        "   background-color: #2a2a2a; "
        "   box-shadow: inset 0 -2px 0 #5f87d7; "
        "}"
        "frame { "
        "   background-color: #1a1a1a; "
        "   border: 1px solid #333333; "
        "   border-radius: 3px; "
        "   box-shadow: 0 1px 3px rgba(0,0,0,0.5); "
        "}"
        "progressbar trough { "
        "   background-color: #1a1a1a; "
        "   border-radius: 3px; "
        "   min-height: 8px; "
        "}"
        "progressbar progress { "
        "   background-color: #5f87d7; "
        "   border-radius: 3px; "
        "}"
        "progressbar.cpu-low progress { "
        "   background-color: #87af5f; "
        "}"
        "progressbar.cpu-medium progress { "
        "   background-color: #d7d75f; "
        "}"
        "progressbar.cpu-high progress { "
        "   background-color: #d78700; "
        "}"
        "progressbar.cpu-critical progress { "
        "   background-color: #d75f5f; "
        "}"
        "progressbar.memory-low progress { "
        "   background-color: #87af5f; "
        "}"
        "progressbar.memory-medium progress { "
        "   background-color: #d7d75f; "
        "}"
        "progressbar.memory-high progress { "
        "   background-color: #d78700; "
        "}"
        "progressbar.memory-critical progress { "
        "   background-color: #d75f5f; "
        "}"
        "progressbar.swap-low progress { "
        "   background-color: #87af5f; "
        "}"
        "progressbar.swap-medium progress { "
        "   background-color: #d78700; "
        "}"
        "progressbar.swap-high progress { "
        "   background-color: #d75f5f; "
        "}"
        "treeview { "
        "   background-color: #1a1a1a; "
        "   color: #d0d0d0; "
        "   font-family: 'Monospace'; "
        "}"
        "treeview header { "
        "   background-color: #2a2a2a; "
        "   color: #d0d0d0; "
        "   border: 1px solid #333333; "
        "}"
        "treeview:selected { "
        "   background-color: #333333; "
        "}"
        ".view { "
        "   background-color: #1a1a1a; "
        "   color: #d0d0d0; "
        "}"
        ".dark-bg { "
        "   background-color: #1a1a1a; "
        "   color: #d0d0d0; "
        "}"
        "treeview.view { "
        "   background-color: #1a1a1a; "
        "   color: #d0d0d0; "
        "}"
        "treeview.dark-bg { "
        "   background-color: #1a1a1a; "
        "   color: #d0d0d0; "
        "}"
        "treeview.view:selected { "
        "   background-color: #333333; "
        "   color: #ffffff; "
        "}"
        "statusbar { "
        "   background-color: #1a1a1a; "
        "   color: #d0d0d0; "
        "   font-family: 'Monospace'; "
        "   border-top: 1px solid #333333; "
        "   padding: 2px; "
        "}"
        "scrollbar { "
        "   background-color: #1a1a1a; "
        "   border: none; "
        "}"
        "scrollbar slider { "
        "   background-color: #333333; "
        "   border-radius: 3px; "
        "   min-width: 8px; "
        "   min-height: 8px; "
        "}"
        "scrollbar slider:hover { "
        "   background-color: #444444; "
        "}"
        "separator { "
        "   background-color: #333333; "
        "   min-height: 1px; "
        "}";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), 
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    if (GTK_IS_CONTAINER(widget)) {
        gtk_container_forall(GTK_CONTAINER(widget), 
                            (GtkCallback)apply_vim_theme, NULL);
    }
    
    g_object_unref(provider);
}

/**
 * CPU graph drawing callback
 */
gboolean draw_cpu_graph(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GuiData *gui_data = (GuiData *)data;
    GtkAllocation allocation;
    
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    // Draw background - using darker background
    cairo_set_source_rgb(cr, 0.07, 0.07, 0.07);  // Darker background #121212
    cairo_paint(cr);
    
    // Draw grid
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.3);  // Darker grid lines
    
    // Horizontal grid lines
    for (int i = 1; i < 4; i++) {
        double y = height * i / 4.0;
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);
    }
    
    // Vertical grid lines
    for (int i = 1; i < 6; i++) {
        double x = width * i / 6.0;
        cairo_move_to(cr, x, 0);
        cairo_line_to(cr, x, height);
    }
    cairo_stroke(cr);
    
    // Display CPU usage (25%, 50%, 75%, 100%)
    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.7);
    cairo_set_font_size(cr, 9);
    
    for (int i = 1; i <= 4; i++) {
        char text[10];
        snprintf(text, sizeof(text), "%d%%", i * 25);
        cairo_move_to(cr, 2, height - (height * i / 4.0) - 2);
        cairo_show_text(cr, text);
    }
    
    // Draw border
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);  // Darker border
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);
    
    // Draw CPU usage
    if (gui_data->cpu_history != NULL && gui_data->cpu_history_size > 0) {
        double x_step = (double)width / gui_data->cpu_history_size;
        
        // Debug output for CPU usage validity check
        printf("Drawing CPU graph: current usage=%.2f%%, history size=%d\n", 
               gui_data->cpu_usage, gui_data->cpu_history_size);
        
        // Fill area below graph
        cairo_set_source_rgba(cr, 0.373, 0.529, 0.843, 0.3);  // Blue, semi-transparent
        
        cairo_move_to(cr, 0, height);
        
        for (int i = 0; i < gui_data->cpu_history_size; i++) {
            double x = i * x_step;
            double usage = gui_data->cpu_history[i];
            
            // Clamp usage range (0-100%)
            if (usage < 0) usage = 0;
            if (usage > 100) usage = 100;
            
            double y = height * (1.0 - usage / 100.0);
            
            if (i == 0) {
                cairo_move_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        
        cairo_line_to(cr, width, height);
        cairo_close_path(cr);
        cairo_fill(cr);
        
        // Draw graph line
        cairo_set_source_rgb(cr, 0.373, 0.529, 0.843);  // Blue
        cairo_set_line_width(cr, 1.5);
        
        for (int i = 0; i < gui_data->cpu_history_size; i++) {
            double x = i * x_step;
            double usage = gui_data->cpu_history[i];
            
            // Clamp usage range (0-100%)
            if (usage < 0) usage = 0;
            if (usage > 100) usage = 100;
            
            double y = height * (1.0 - usage / 100.0);
            
            if (i == 0) {
                cairo_move_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        
        cairo_stroke(cr);
        
        // Mark last data point - use blue color only (no white outline)
        if (gui_data->cpu_history_size > 0) {
            int last = gui_data->cpu_history_size - 1;
            double x = last * x_step;
            double usage = gui_data->cpu_history[last];
            
            // Clamp usage range (0-100%)
            if (usage < 0) usage = 0;
            if (usage > 100) usage = 100;
            
            double y = height * (1.0 - usage / 100.0);
            
            // Draw point with dark background and blue color
            cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);  // Dark gray background
            cairo_arc(cr, x, y, 3.5, 0, 2 * M_PI);
            cairo_fill(cr);
            
            cairo_set_source_rgb(cr, 0.373, 0.529, 0.843);  // Blue
            cairo_arc(cr, x, y, 2.5, 0, 2 * M_PI);
            cairo_fill(cr);
        }
    }
    
    return FALSE;
}

/**
 * Memory graph drawing callback
 */
gboolean draw_memory_graph(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GuiData *gui_data = (GuiData *)data;
    GtkAllocation allocation;
    
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    // Draw background - using darker background from VIM theme
    cairo_set_source_rgb(cr, 0.07, 0.07, 0.07);  // Darker background #121212
    cairo_paint(cr);
    
    // Draw grid
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.3);  // Darker grid lines
    
    // Horizontal grid lines
    for (int i = 1; i < 4; i++) {
        double y = height * i / 4.0;
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);
    }
    
    // Vertical grid lines
    for (int i = 1; i < 6; i++) {
        double x = width * i / 6.0;
        cairo_move_to(cr, x, 0);
        cairo_line_to(cr, x, height);
    }
    cairo_stroke(cr);
    
    // Display memory usage percentage (25%, 50%, 75%, 100%)
    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.7);
    cairo_set_font_size(cr, 9);
    
    for (int i = 1; i <= 4; i++) {
        char text[10];
        snprintf(text, sizeof(text), "%d%%", i * 25);
        cairo_move_to(cr, 2, height - (height * i / 4.0) - 2);
        cairo_show_text(cr, text);
    }
    
    // Draw border
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);  // Darker border
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);
    
    // Draw memory usage - based on percentage
    if (gui_data->memory_history != NULL && gui_data->memory_history_size > 0 && gui_data->memory_total > 0) {
        double x_step = (double)width / gui_data->memory_history_size;
        
        // Fill area below graph
        cairo_set_source_rgba(cr, 0.529, 0.686, 0.373, 0.3);
        
        cairo_move_to(cr, 0, height);
        
        // Calculate first point - based on percentage
        double memory_percent = (gui_data->memory_history[0] / gui_data->memory_total) * 100.0;
        if (memory_percent > 100.0) memory_percent = 100.0; // Clamp to maximum 100%
        
        cairo_line_to(cr, 0, height * (1.0 - memory_percent / 100.0));
        
        for (int i = 1; i < gui_data->memory_history_size; i++) {
            double x = i * x_step;
            
            // Convert each point to percentage
            memory_percent = (gui_data->memory_history[i] / gui_data->memory_total) * 100.0;
            if (memory_percent > 100.0) memory_percent = 100.0; // Clamp to maximum 100%
            
            double y = height * (1.0 - memory_percent / 100.0);
            cairo_line_to(cr, x, y);
        }
        
        cairo_line_to(cr, width, height);
        cairo_close_path(cr);
        cairo_fill(cr);
        
        // Draw graph line
        cairo_set_source_rgb(cr, 0.529, 0.686, 0.373);
        cairo_set_line_width(cr, 1.5);
        
        // Recalculate first point
        memory_percent = (gui_data->memory_history[0] / gui_data->memory_total) * 100.0;
        if (memory_percent > 100.0) memory_percent = 100.0;
        
        cairo_move_to(cr, 0, height * (1.0 - memory_percent / 100.0));
        
        for (int i = 1; i < gui_data->memory_history_size; i++) {
            double x = i * x_step;
            
            // Convert percentage
            memory_percent = (gui_data->memory_history[i] / gui_data->memory_total) * 100.0;
            if (memory_percent > 100.0) memory_percent = 100.0;
            
            double y = height * (1.0 - memory_percent / 100.0);
            cairo_line_to(cr, x, y);
        }
        
        cairo_stroke(cr);
        
        // Draw point - use dark background instead of white
        if (gui_data->memory_history_size > 0) {
            double x = (gui_data->memory_history_size - 1) * x_step;
            
            // Calculate last point percentage
            memory_percent = (gui_data->memory_history[gui_data->memory_history_size - 1] / gui_data->memory_total) * 100.0;
            if (memory_percent > 100.0) memory_percent = 100.0;
            
            double y = height * (1.0 - memory_percent / 100.0);
            
            // Draw point with dark background
            cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);  // Dark gray background
            cairo_arc(cr, x, y, 3.5, 0, 2 * M_PI);
            cairo_fill(cr);
            
            cairo_set_source_rgb(cr, 0.529, 0.686, 0.373);
            cairo_arc(cr, x, y, 2.5, 0, 2 * M_PI);
            cairo_fill(cr);
        }
    }
    
    return FALSE;
}

/**
 * Swap memory graph drawing callback
 */
gboolean draw_swap_graph(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GuiData *gui_data = (GuiData *)data;
    GtkAllocation allocation;
    
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    // Draw background - using darker background from VIM theme
    cairo_set_source_rgb(cr, 0.07, 0.07, 0.07);  // Darker background #121212
    cairo_paint(cr);
    
    // Draw grid
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.3);  // Darker grid lines
    
    // Horizontal grid lines
    for (int i = 1; i < 4; i++) {
        double y = height * i / 4.0;
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);
    }
    
    // Vertical grid lines
    for (int i = 1; i < 6; i++) {
        double x = width * i / 6.0;
        cairo_move_to(cr, x, 0);
        cairo_line_to(cr, x, height);
    }
    cairo_stroke(cr);
    
    // Display swap usage percentage (25%, 50%, 75%, 100%)
    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.7);
    cairo_set_font_size(cr, 9);
    
    for (int i = 1; i <= 4; i++) {
        char text[10];
        snprintf(text, sizeof(text), "%d%%", i * 25);
        cairo_move_to(cr, 2, height - (height * i / 4.0) - 2);
        cairo_show_text(cr, text);
    }
    
    // Draw border
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);  // Darker border
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);
    
    // Draw swap usage
    if (gui_data->swap_history != NULL && gui_data->swap_history_size > 0 && gui_data->swap_total > 0) {
        double x_step = (double)width / gui_data->swap_history_size;
        
        // Fill area below graph
        cairo_set_source_rgba(cr, 0.843, 0.529, 0.0, 0.3);
        
        cairo_move_to(cr, 0, height);
        
        // Calculate first point - based on percentage
        double swap_percent = (gui_data->swap_history[0] / gui_data->swap_total) * 100.0;
        if (swap_percent > 100.0) swap_percent = 100.0; // Clamp to maximum 100%
        
        cairo_line_to(cr, 0, height * (1.0 - swap_percent / 100.0));
        
        for (int i = 1; i < gui_data->swap_history_size; i++) {
            double x = i * x_step;
            
            // Convert each point to percentage
            swap_percent = (gui_data->swap_history[i] / gui_data->swap_total) * 100.0;
            if (swap_percent > 100.0) swap_percent = 100.0; // Clamp to maximum 100%
            
            double y = height * (1.0 - swap_percent / 100.0);
            cairo_line_to(cr, x, y);
        }
        
        cairo_line_to(cr, width, height);
        cairo_close_path(cr);
        cairo_fill(cr);
        
        // Draw graph line
        cairo_set_source_rgb(cr, 0.843, 0.529, 0.0);
        cairo_set_line_width(cr, 1.5);
        
        // Recalculate first point
        swap_percent = (gui_data->swap_history[0] / gui_data->swap_total) * 100.0;
        if (swap_percent > 100.0) swap_percent = 100.0;
        
        cairo_move_to(cr, 0, height * (1.0 - swap_percent / 100.0));
        
        for (int i = 1; i < gui_data->swap_history_size; i++) {
            double x = i * x_step;
            
            // Convert percentage
            swap_percent = (gui_data->swap_history[i] / gui_data->swap_total) * 100.0;
            if (swap_percent > 100.0) swap_percent = 100.0;
            
            double y = height * (1.0 - swap_percent / 100.0);
            cairo_line_to(cr, x, y);
        }
        
        cairo_stroke(cr);
        
        // Draw point with dark background
        if (gui_data->swap_history_size > 0) {
            double x = (gui_data->swap_history_size - 1) * x_step;
            
            // Calculate last point percentage
            swap_percent = (gui_data->swap_history[gui_data->swap_history_size - 1] / gui_data->swap_total) * 100.0;
            if (swap_percent > 100.0) swap_percent = 100.0;
            
            double y = height * (1.0 - swap_percent / 100.0);
            
            // Draw point with dark background
            cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);  // Dark gray background
            cairo_arc(cr, x, y, 3.5, 0, 2 * M_PI);
            cairo_fill(cr);
            
            cairo_set_source_rgb(cr, 0.843, 0.529, 0.0);
            cairo_arc(cr, x, y, 2.5, 0, 2 * M_PI);
            cairo_fill(cr);
        }
    }
    
    return FALSE;
}

/**
 * Window close event handler
 */
void on_window_destroy(GtkWidget *widget, gpointer data) {
    (void)widget; // Ignore unused parameter warning
    (void)data;   // Ignore unused parameter warning
    
    LOG_INFO(SYS_MON_SUCCESS, "Window close event detected. Shutting down program...");
    gtk_main_quit();
}

/**
 * Update system information display
 */
void update_system_info_display(GuiWidgets *widgets, GuiData *data) {
    char system_info[1024];
    
    // Update main system information tab - simple markup
    snprintf(system_info, sizeof(system_info),
             "<span font_desc=\"Monospace\">"
             "<b>System Information</b>\n\n"
             "System Name: <span foreground=\"#5f87d7\">%s</span>\n"
             "Machine Name: <span foreground=\"#5f87d7\">%s</span>\n"
             "Version: <span foreground=\"#5f87d7\">%s</span>\n"
             "Release: <span foreground=\"#5f87d7\">%s</span>\n"
             "Architecture: <span foreground=\"#5f87d7\">%s</span>\n\n"
             "Uptime: <span foreground=\"#87af5f\">%d days %02d:%02d:%02d</span>"
             "</span>",
             data->system_name ? data->system_name : "Unknown",
             data->node_name ? data->node_name : "Unknown",
             data->version ? data->version : "Unknown",
             data->release ? data->release : "Unknown",
             data->machine ? data->machine : "Unknown",
             data->uptime_days, data->uptime_hours, 
             data->uptime_minutes, data->uptime_seconds);
    
    gtk_label_set_markup(GTK_LABEL(widgets->system_info_label), system_info);
    
    // Update dashboard system information - simplified markup
    snprintf(system_info, sizeof(system_info),
             "<span font_desc=\"Monospace\">"
             "<span foreground=\"#d0d0d0\">System Info: %s %s</span>\n"
             "<span foreground=\"#808080\">%s</span>\n\n"
             "<b>Hostname:</b> <span foreground=\"#5f87d7\">%s</span>\n"
             "<b>Architecture:</b> <span foreground=\"#5f87d7\">%s</span>\n"
             "<b>Kernel:</b> <span foreground=\"#5f87d7\">%s</span>\n\n"
             "<b>Uptime:</b> <span foreground=\"#87af5f\">%d days %02d:%02d:%02d</span>"
             "</span>",
             data->system_name ? data->system_name : "Unknown",
             data->machine ? data->machine : "",
             data->release ? data->release : "Unknown",
             data->node_name ? data->node_name : "Unknown",
             data->machine ? data->machine : "Unknown",
             data->version ? data->version : "Unknown",
             data->uptime_days, data->uptime_hours, 
             data->uptime_minutes, data->uptime_seconds);
    
    gtk_label_set_markup(GTK_LABEL(widgets->dashboard_system_info), system_info);
}

/**
 * Update CPU display
 */
void update_cpu_display(GuiWidgets *widgets, GuiData *data) {
    char cpu_info[256];
    
    // Validate CPU usage
    if (data->cpu_usage < 0) {
        data->cpu_usage = 0.0;
        printf("[ Warning: CPU usage is negative. Adjusting to 0. ]\n");
    }
    if (data->cpu_usage > 100.0) {
        data->cpu_usage = 100.0;
        printf("[ Warning: CPU usage exceeds 100%%. Adjusting to 100%%. ]\n");
    }
    
    printf("[ update_cpu_display: CPU usage = %.2f%% ]\n", data->cpu_usage);
    
    // Update main CPU tab - simple markup
    snprintf(cpu_info, sizeof(cpu_info),
             "<span font_desc=\"Monospace\">"
             "CPU Usage: <span foreground=\"#5f87d7\">%.2f%%</span>"
             "</span>",
             data->cpu_usage);
    
    gtk_label_set_markup(GTK_LABEL(widgets->cpu_usage_label), cpu_info);
    
    // Color based on CPU usage
    char *cpu_color = "#87af5f";  // Default color (green)
    char *cpu_level = "Normal";
    
    if (data->cpu_usage > 90.0) {
        cpu_color = "#d75f5f";  // Red
        cpu_level = "Critical";
    } else if (data->cpu_usage > 70.0) {
        cpu_color = "#d78700";  // Orange
        cpu_level = "High";
    } else if (data->cpu_usage > 40.0) {
        cpu_color = "#d7d75f";  // Yellow
        cpu_level = "Moderate";
    }
    
    // Update dashboard CPU information - simplified markup
    snprintf(cpu_info, sizeof(cpu_info),
             "<span font_desc=\"Monospace\">"
             "<span foreground=\"#d0d0d0\">CPU Usage</span>\n"
             "<span foreground=\"%s\">%.1f%%</span>\n"
             "<span foreground=\"%s\">%s</span>"
             "</span>",
             cpu_color, data->cpu_usage,
             cpu_color, cpu_level);
    
    // Debug log before updating label
    printf("[ Updating CPU label: %s ]\n", cpu_info);
    
    gtk_label_set_markup(GTK_LABEL(widgets->dashboard_cpu_label), cpu_info);
    
    // Update CPU progress bar
    double fraction = data->cpu_usage / 100.0;
    printf("[ Updating CPU progress bar: %.2f ]\n", fraction);
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->cpu_usage_bar), fraction);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->dashboard_cpu_bar), fraction);
    
    // Set progress bar color
    GtkStyleContext *context = gtk_widget_get_style_context(widgets->dashboard_cpu_bar);
    gtk_style_context_remove_class(context, "cpu-low");
    gtk_style_context_remove_class(context, "cpu-medium");
    gtk_style_context_remove_class(context, "cpu-high");
    gtk_style_context_remove_class(context, "cpu-critical");
    
    if (data->cpu_usage > 90.0) {
        gtk_style_context_add_class(context, "cpu-critical");
    } else if (data->cpu_usage > 70.0) {
        gtk_style_context_add_class(context, "cpu-high");
    } else if (data->cpu_usage > 40.0) {
        gtk_style_context_add_class(context, "cpu-medium");
    } else {
        gtk_style_context_add_class(context, "cpu-low");
    }
    
    // Redraw CPU graph widget
    gtk_widget_queue_draw(widgets->cpu_usage_graph);
    gtk_widget_queue_draw(widgets->dashboard_cpu_graph);
    
    printf("[ update_cpu_display: complete ]\n");
}

/**
 * Calculate memory information more accurately
 * Linux: Read information from /proc/meminfo
 * macOS: Use vm_statistics or sysctl
 * 
 * @param used_memory Pointer to store used memory (in GB)
 * @param total_memory Pointer to store total memory (in GB)
 * @param used_swap Pointer to store used swap (in GB)
 * @param total_swap Pointer to store total swap (in GB)
 * @return 0 on success, -1 on failure
 */
int get_detailed_memory_info(double *used_memory, double *total_memory, 
                             double *used_swap, double *total_swap) {
#ifdef __linux__
    // Linux system code
    FILE *fp;
    char buffer[256];
    long memTotal = 0, memFree = 0, buffers = 0, cached = 0, swapTotal = 0, swapFree = 0;
    
    // Open /proc/meminfo file
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        // If opening file fails, use sysinfo information as a fallback
        struct sysinfo sys_info;
        if (sysinfo(&sys_info) == 0) {
            *total_memory = (double)sys_info.totalram / (1024 * 1024 * 1024);
            *used_memory = *total_memory - (double)sys_info.freeram / (1024 * 1024 * 1024);
            *total_swap = (double)sys_info.totalswap / (1024 * 1024 * 1024);
            *used_swap = *total_swap - (double)sys_info.freeswap / (1024 * 1024 * 1024);
            return 0;
        }
        return -1;
    }
    
    // Parse /proc/meminfo file
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (sscanf(buffer, "MemTotal: %ld kB", &memTotal) == 1) {
            continue;
        }
        if (sscanf(buffer, "MemFree: %ld kB", &memFree) == 1) {
            continue;
        }
        if (sscanf(buffer, "Buffers: %ld kB", &buffers) == 1) {
            continue;
        }
        if (sscanf(buffer, "Cached: %ld kB", &cached) == 1) {
            continue;
        }
        if (sscanf(buffer, "SwapTotal: %ld kB", &swapTotal) == 1) {
            continue;
        }
        if (sscanf(buffer, "SwapFree: %ld kB", &swapFree) == 1) {
            continue;
        }
    }
    
    fclose(fp);
    
    // Convert values to GB
    const double KB_TO_GB = 1024.0 * 1024.0;
    *total_memory = memTotal / KB_TO_GB;
    
    // Used memory = total memory - free memory - buffers - cached
    // This method more accurately reflects "available" memory
    *used_memory = (memTotal - memFree - buffers - cached) / KB_TO_GB;
    
    // Ensure non-negative values
    if (*used_memory < 0) *used_memory = 0;
    
    *total_swap = swapTotal / KB_TO_GB;
    *used_swap = (swapTotal - swapFree) / KB_TO_GB;
    
    return 0;
    
#elif defined(__APPLE__) && defined(__MACH__)
    // macOS system code
    // macOS requires using mach interfaces or sysctl functions,
    // but here we assume platform.h's calculate_memory_usage() function is already implemented

    // Total memory (GB)
    *total_memory = calculate_memory_total();
    
    // Used memory (GB)
    *used_memory = calculate_memory_usage();
    
    // If used memory is greater than total memory, adjust
    if (*used_memory > *total_memory) {
        *used_memory = *total_memory * 0.85;  // Temporary adjustment value (85%)
    }
    
    // Swap information (macOS requires a separate function for swap)
    *total_swap = calculate_swap_total();
    *used_swap = calculate_swap_usage();
    
    return 0;
#else
    // Other systems use sysinfo
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        *total_memory = (double)sys_info.totalram / (1024 * 1024 * 1024);
        *used_memory = *total_memory - (double)sys_info.freeram / (1024 * 1024 * 1024);
        *total_swap = (double)sys_info.totalswap / (1024 * 1024 * 1024);
        *used_swap = *total_swap - (double)sys_info.freeswap / (1024 * 1024 * 1024);
        return 0;
    }
    return -1;
#endif
}

/**
 * Update memory display
 */
void update_memory_display(GuiWidgets *widgets, GuiData *data) {
    char memory_info[256];
    char swap_info[256];
    
    // Calculate memory usage percentage
    double memory_percent = data->memory_total > 0 ? 
                          (data->memory_used / data->memory_total * 100.0) : 0.0;
    double swap_percent = data->swap_total > 0 ? 
                         (data->swap_used / data->swap_total * 100.0) : 0.0;
    
    // Update main memory tab - simple markup
    snprintf(memory_info, sizeof(memory_info),
             "<span font_desc=\"Monospace\">"
             "Memory: <span foreground=\"#87af5f\">%.2f GB / %.2f GB</span> (%.1f%%)"
             "</span>",
             data->memory_used, data->memory_total, memory_percent);
    
    gtk_label_set_markup(GTK_LABEL(widgets->memory_usage_label), memory_info);
    
    // Color based on memory usage
    char *memory_color = "#87af5f";  // Default color (green)
    if (memory_percent > 90.0) {
        memory_color = "#d75f5f";  // Red
    } else if (memory_percent > 70.0) {
        memory_color = "#d78700";  // Orange
    } else if (memory_percent > 50.0) {
        memory_color = "#d7d75f";  // Yellow
    }
    
    // Update dashboard memory information - simplified markup
    snprintf(memory_info, sizeof(memory_info),
             "<span font_desc=\"Monospace\">"
             "<span foreground=\"#d0d0d0\">Memory Usage</span>\n"
             "<span foreground=\"%s\">%.1f%%</span>\n"
             "<span foreground=\"%s\">%.2f GB / %.2f GB</span>"
             "</span>",
             memory_color, memory_percent,
             memory_color, data->memory_used, data->memory_total);
    
    gtk_label_set_markup(GTK_LABEL(widgets->dashboard_memory_label), memory_info);
    
    // Color based on swap usage
    char *swap_color = "#87af5f";  // Default color (green)
    if (swap_percent > 50.0) {
        swap_color = "#d75f5f";  // Red
    } else if (swap_percent > 25.0) {
        swap_color = "#d78700";  // Orange
    } else if (swap_percent > 10.0) {
        swap_color = "#d7d75f";  // Yellow
    }
    
    // Swap level
    char *swap_level = "Normal";
    if (swap_percent > 50.0) {
        swap_level = "High";
    } else if (swap_percent > 25.0) {
        swap_level = "Moderate";
    } else if (swap_percent > 10.0) {
        swap_level = "Low";
    }
    
    // Update swap text - simple markup
    snprintf(swap_info, sizeof(swap_info),
             "<span font_desc=\"Monospace\">"
             "Swap: <span foreground=\"%s\">%.2f GB / %.2f GB</span> (%.1f%%)"
             "</span>",
             swap_color, data->swap_used, data->swap_total, swap_percent);
    
    gtk_label_set_markup(GTK_LABEL(widgets->swap_usage_label), swap_info);
    
    // Update dashboard swap information - simplified markup
    snprintf(swap_info, sizeof(swap_info),
             "<span font_desc=\"Monospace\">"
             "<span foreground=\"#d0d0d0\">Swap Usage</span>\n"
             "<span foreground=\"%s\">%.1f%%</span>\n"
             "<span foreground=\"%s\">%.2f GB / %.2f GB</span>\n"
             "<span foreground=\"%s\">%s</span>"
             "</span>",
             swap_color, swap_percent,
             swap_color, data->swap_used, data->swap_total,
             swap_color, swap_level);
    
    gtk_label_set_markup(GTK_LABEL(widgets->dashboard_swap_label), swap_info);
    
    // Update memory progress bars
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->memory_usage_bar),
                                 data->memory_total > 0 ? 
                                 (data->memory_used / data->memory_total) : 0.0);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->dashboard_memory_bar),
                                 data->memory_total > 0 ? 
                                 (data->memory_used / data->memory_total) : 0.0);
    
    // Update swap progress bars
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->swap_usage_bar),
                                 data->swap_total > 0 ? 
                                 (data->swap_used / data->swap_total) : 0.0);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->dashboard_swap_bar),
                                 data->swap_total > 0 ? 
                                 (data->swap_used / data->swap_total) : 0.0);
    
    // Set progress bar colors
    GtkStyleContext *context;
    
    // Memory progress bar color
    context = gtk_widget_get_style_context(widgets->dashboard_memory_bar);
    gtk_style_context_remove_class(context, "memory-low");
    gtk_style_context_remove_class(context, "memory-medium");
    gtk_style_context_remove_class(context, "memory-high");
    gtk_style_context_remove_class(context, "memory-critical");
    
    if (memory_percent > 90.0) {
        gtk_style_context_add_class(context, "memory-critical");
    } else if (memory_percent > 70.0) {
        gtk_style_context_add_class(context, "memory-high");
    } else if (memory_percent > 50.0) {
        gtk_style_context_add_class(context, "memory-medium");
    } else {
        gtk_style_context_add_class(context, "memory-low");
    }
    
    // Swap progress bar color
    context = gtk_widget_get_style_context(widgets->dashboard_swap_bar);
    gtk_style_context_remove_class(context, "swap-low");
    gtk_style_context_remove_class(context, "swap-medium");
    gtk_style_context_remove_class(context, "swap-high");
    
    if (swap_percent > 50.0) {
        gtk_style_context_add_class(context, "swap-high");
    } else if (swap_percent > 25.0) {
        gtk_style_context_add_class(context, "swap-medium");
    } else {
        gtk_style_context_add_class(context, "swap-low");
    }
    
    // Redraw memory graph widget
    gtk_widget_queue_draw(widgets->memory_usage_graph);
    gtk_widget_queue_draw(widgets->dashboard_memory_graph);
    
    // Redraw swap graph widget
    gtk_widget_queue_draw(widgets->swap_usage_graph);
    gtk_widget_queue_draw(widgets->dashboard_swap_graph);
}

/**
 * Update users display
 */
void update_users_display(GuiWidgets *widgets, GuiData *data) {
    GtkListStore *store;
    GtkTreeIter iter;
    
    // Update main user list
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->users_list)));
    if (store == NULL) {
        store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->users_list), GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        // Add column
        GtkTreeViewColumn *column = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(column, "User Sessions");
        gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->users_list), column);
        
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, renderer, TRUE);
        gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
    } else {
        gtk_list_store_clear(store);
    }
    
    // Update dashboard user list
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->dashboard_users_list)));
    if (store == NULL) {
        store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->dashboard_users_list), GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        // Add column
        GtkTreeViewColumn *column = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(column, "User Sessions");
        gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->dashboard_users_list), column);
        
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, renderer, TRUE);
        gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
    } else {
        gtk_list_store_clear(store);
    }
    
    // Add users (both lists)
    for (int i = 0; i < data->user_count; i++) {
        if (data->users[i] != NULL) {
            // Add to main user list
            store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->users_list)));
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, data->users[i], -1);
            
            // Add to dashboard user list
            store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->dashboard_users_list)));
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, data->users[i], -1);
        }
    }
}

/**
 * Collect system data and update GUI (timer callback)
 */
gboolean update_system_data(gpointer user_data) {
    GuiData *data = (GuiData *)user_data;
    struct utsname sys_name_info;
    int days, hours, minutes, seconds;
    
    // Collect CPU usage - simplified stable approach
    LOG_INFO(SYS_MON_SUCCESS, "Collecting CPU information");
    
    // Static variables for CPU calculation
    static unsigned long prev_stats[7] = {0};
    static unsigned long curr_stats[7] = {0};
    static double last_cpu_usage = 15.0; // Start with reasonable default
    static int samples_collected = 0;
    
    // Backup previous values before getting new ones
    memcpy(prev_stats, curr_stats, sizeof(curr_stats));
    
    // Get current CPU stats
    get_cpu_stats(curr_stats);
    
    // Calculate CPU usage
    double cpu_usage = last_cpu_usage; // Default to last usage if calculation fails
    
    // Debug info
    printf("CPU raw stats - User: %lu, Nice: %lu, System: %lu, Idle: %lu\n", 
           curr_stats[0], curr_stats[1], curr_stats[2], curr_stats[3]);
    
    // Only process if we have at least one previous sample
    if (samples_collected > 0) {
        // Calculate deltas for user, nice, system, idle
        unsigned long user_delta = (curr_stats[0] >= prev_stats[0]) ? 
                                  (curr_stats[0] - prev_stats[0]) : 0;
        unsigned long nice_delta = (curr_stats[1] >= prev_stats[1]) ? 
                                  (curr_stats[1] - prev_stats[1]) : 0;
        unsigned long system_delta = (curr_stats[2] >= prev_stats[2]) ? 
                                    (curr_stats[2] - prev_stats[2]) : 0;
        unsigned long idle_delta = (curr_stats[3] >= prev_stats[3]) ? 
                                  (curr_stats[3] - prev_stats[3]) : 0;
        
        // Total delta = sum of all activity
        unsigned long total_delta = user_delta + nice_delta + system_delta + idle_delta;
        
        // Log the deltas for debugging
        printf("CPU deltas - User: %lu, Nice: %lu, System: %lu, Idle: %lu, Total: %lu\n",
               user_delta, nice_delta, system_delta, idle_delta, total_delta);
        
        // If we have valid deltas, calculate CPU percentage
        if (total_delta > 0) {
            // CPU percentage = non-idle time / total time
            double new_usage = 100.0 * (double)(total_delta - idle_delta) / (double)total_delta;
            
            // Apply bounds
            if (new_usage < 0.0) new_usage = 0.0;
            if (new_usage > 100.0) new_usage = 100.0;
            
            printf("Raw calculated CPU usage: %.2f%%\n", new_usage);
            
            // Apply weighted average with previous value to smooth out spikes
            // More weight to previous value if this is a big change
            if (fabs(new_usage - last_cpu_usage) > 25.0) {
                // Big change - heavily favor previous value
                cpu_usage = 0.85 * last_cpu_usage + 0.15 * new_usage;
                printf("Large change detected, applying heavy smoothing\n");
            } else if (fabs(new_usage - last_cpu_usage) > 10.0) {
                // Medium change - moderately favor previous value
                cpu_usage = 0.65 * last_cpu_usage + 0.35 * new_usage;
                printf("Medium change detected, applying moderate smoothing\n");
            } else {
                // Small change - light smoothing
                cpu_usage = 0.5 * last_cpu_usage + 0.5 * new_usage;
                printf("Small change, applying light smoothing\n");
            }
            
            printf("Smoothed CPU usage: %.2f%%\n", cpu_usage);
        } else {
            // No significant activity, use previous value
            printf("No significant CPU activity detected, keeping previous value\n");
        }
    } else {
        // First sample, can't calculate yet
        printf("First CPU sample collected, waiting for next sample\n");
    }
    
    // Special case handling for when CPU reading is 0
    if (cpu_usage < 2.0 && last_cpu_usage > 5.0) {
        // Sudden drop to near-zero - likely incorrect reading
        // Only allow gradual decreases
        cpu_usage = last_cpu_usage * 0.7;  // decay toward zero rather than jump
        printf("Suspect zero reading detected, decaying value instead: %.2f%%\n", cpu_usage);
    }
    
    // Update for next iteration
    last_cpu_usage = cpu_usage;
    samples_collected++;
    
    // Update the GUI data
    data->cpu_usage = cpu_usage;
    
    // Log final value
    printf("Final CPU Usage: %.2f%%\n", data->cpu_usage);
    
    // Update CPU history
    if (data->cpu_history == NULL) {
        LOG_INFO(SYS_MON_SUCCESS, "Initializing CPU history (size: 60)");
        data->cpu_history_size = 60; // 1 minute of data (1 second intervals)
        data->cpu_history = calloc(data->cpu_history_size, sizeof(float));
        
        // Initialize with current value
        for (int i = 0; i < data->cpu_history_size; i++) {
            data->cpu_history[i] = (float)data->cpu_usage;
        }
    } else {
        // Shift data
        for (int i = 0; i < data->cpu_history_size - 1; i++) {
            data->cpu_history[i] = data->cpu_history[i + 1];
        }
        
        // Add new data
        data->cpu_history[data->cpu_history_size - 1] = (float)data->cpu_usage;
    }
    
    // Collect memory information - more accurate function
    if (get_detailed_memory_info(&data->memory_used, &data->memory_total, 
                                &data->swap_used, &data->swap_total) != 0) {
        // If error occurs, use previous method
        data->memory_used = calculate_memory_usage();
        data->memory_total = calculate_memory_total();
        data->swap_used = calculate_swap_usage();
        data->swap_total = calculate_swap_total();
    }
    
    // Update memory history
    if (data->memory_history == NULL) {
        data->memory_history_size = 60; // 1 minute of data (1 second intervals)
        data->memory_history = calloc(data->memory_history_size, sizeof(double));
        
        // Set initial history values
        for (int i = 0; i < data->memory_history_size; i++) {
            data->memory_history[i] = data->memory_used;
        }
    } else {
        // Move data
        for (int i = 0; i < data->memory_history_size - 1; i++) {
            data->memory_history[i] = data->memory_history[i + 1];
        }
        // Add new data
        data->memory_history[data->memory_history_size - 1] = data->memory_used;
    }
    
    // Update swap history
    if (data->swap_history == NULL) {
        data->swap_history_size = 60; // 1 minute of data (1 second intervals)
        data->swap_history = calloc(data->swap_history_size, sizeof(double));
        
        // Set initial history values
        for (int i = 0; i < data->swap_history_size; i++) {
            data->swap_history[i] = data->swap_used;
        }
    } else {
        // Move data
        for (int i = 0; i < data->swap_history_size - 1; i++) {
            data->swap_history[i] = data->swap_history[i + 1];
        }
        // Add new data
        data->swap_history[data->swap_history_size - 1] = data->swap_used;
    }
    
    // Collect system information
    if (uname(&sys_name_info) == 0) {
        free(data->system_name);
        data->system_name = strdup(sys_name_info.sysname);
        
        free(data->node_name);
        data->node_name = strdup(sys_name_info.nodename);
        
        free(data->version);
        data->version = strdup(sys_name_info.version);
        
        free(data->release);
        data->release = strdup(sys_name_info.release);
        
        free(data->machine);
        data->machine = strdup(sys_name_info.machine);
    }
    
    // Get system uptime
    get_system_uptime(&days, &hours, &minutes, &seconds);
    data->uptime_days = days;
    data->uptime_hours = hours;
    data->uptime_minutes = minutes;
    data->uptime_seconds = seconds;
    
    // Collect user session information
    if (data->users != NULL) {
        for (int i = 0; i < data->user_count; i++) {
            free(data->users[i]);
        }
        free(data->users);
    }
    
    // Collect user session information
    // Currently, a simple example
    data->user_count = 1;
    data->users = calloc(data->user_count, sizeof(char *));
    data->users[0] = strdup("Current User (Terminal)");
    
    // Update GUI
    update_system_info_display(&widgets, data);
    update_cpu_display(&widgets, data);
    update_memory_display(&widgets, data);
    update_users_display(&widgets, data);
    
    // Update status bar
    char status_msg[128];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    
    snprintf(status_msg, sizeof(status_msg), 
             "Last updated: %02d:%02d:%02d | CPU: %.1f%% | Memory: %.1f%% | Swap: %.1f%% | System: %s",
             tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec,
             data->cpu_usage,
             data->memory_total > 0 ? (data->memory_used / data->memory_total * 100.0) : 0.0,
             data->swap_total > 0 ? (data->swap_used / data->swap_total * 100.0) : 0.0,
             data->system_name ? data->system_name : "Unknown");
    
    gtk_statusbar_pop(GTK_STATUSBAR(widgets.statusbar), widgets.statusbar_context_id);
    gtk_statusbar_push(GTK_STATUSBAR(widgets.statusbar), 
                      widgets.statusbar_context_id, status_msg);
    
    LOG_INFO(SYS_MON_SUCCESS, "Data updated");
    return G_SOURCE_CONTINUE; // Continue timer
}

/**
 * Function to update the CPU usage display
 * @param label GTK label to update
 * @param prevCpuUsage Previous CPU usage statistics
 * @param currCpuUsage Current CPU usage statistics
 */
void updateCPUDisplay(GtkWidget *label, unsigned long prevCpuUsage[7], unsigned long currCpuUsage[7]) {
    // Calculate CPU usage percentage
    double cpuUsage = calculateCPUUsage(prevCpuUsage, currCpuUsage);
    
    // Create visual bar representation based on CPU usage
    char bar[100] = "";
    int barLength = (int)(cpuUsage / 2); // Each bar represents 2%
    
    // Cap the bar length to prevent buffer overflow
    if (barLength > 50) barLength = 50;
    
    // Create the bar visualization
    for (int i = 0; i < barLength; i++) {
        strcat(bar, "█");
    }
    
    // Prepare the complete display text with percentage
    char displayText[256];
    snprintf(displayText, sizeof(displayText), 
             "<span font_desc='monospace'><b>CPU Usage: %.1f%%</b>\n[%s]</span>", 
             cpuUsage, bar);
    
    // Update the GUI label with the formatted text
    gtk_label_set_markup(GTK_LABEL(label), displayText);
    
    // Debug output to verify data flow
    printf("GUI: Updated CPU display with usage %.1f%%\n", cpuUsage);
}

/**
 * Function to create system statistics widgets
 * @param sysData Pointer to the SystemData structure
 * @return GtkWidget* Pointer to the created system statistics widget
 */
GtkWidget* createSystemStatsWidget(SystemData *sysData) {
    // Create a vertical box to hold all system stats
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);
    
    // Create a section title
    GtkWidget *titleLabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(titleLabel), 
                         "<span font_desc='Sans Bold 14'>System Statistics</span>");
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), titleLabel, FALSE, FALSE, 5);
    
    // Add a separator
    gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), 
                       FALSE, FALSE, 5);
    
    // Create CPU usage section with improved visualization
    GtkWidget *cpuLabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(cpuLabel), 
                         "<span font_desc='monospace'><b>CPU Usage: Waiting for data...</b>\n[                    ]</span>");
    gtk_widget_set_halign(cpuLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), cpuLabel, FALSE, FALSE, 5);
    
    // Save reference to the CPU label
    sysData->cpuLabel = cpuLabel;
    
    // Initialize CPU usage arrays
    memset(sysData->prevCpuUsage, 0, sizeof(sysData->prevCpuUsage));
    memset(sysData->currCpuUsage, 0, sizeof(sysData->currCpuUsage));
    
    // Set initial values to avoid division by zero
    sysData->prevCpuUsage[0] = 10; // USER
    sysData->prevCpuUsage[1] = 1;  // NICE
    sysData->prevCpuUsage[2] = 1;  // SYSTEM
    sysData->prevCpuUsage[3] = 100; // IDLE
    
    // Create memory usage section
    // ... existing memory widget creation code ...
    
    return vbox;
}

#else
// If GTK+ library is not available, provide dummy functions

/**
 * GUI initialization function (dummy)
 */
void init_gui(int *argc, char ***argv) {
    (void)argc;
    (void)argv;
    fprintf(stderr, "Error: GTK+3 library is not available. Unable to initialize GUI.\n");
}

/**
 * GUI execution function (dummy)
 */
void run_gui(void) {
    fprintf(stderr, "Error: GTK+3 library is not available. Unable to run GUI.\n");
}

/**
 * GUI cleanup function (dummy)
 */
void cleanup_gui(void) {
    // No operations are performed
}

#endif // HAVE_GTK