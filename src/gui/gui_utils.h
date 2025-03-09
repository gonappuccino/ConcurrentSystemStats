#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include "gui.h"
#include "error.h"

#ifdef HAVE_GTK

/**
 * Widget creation options
 */
typedef struct {
    // Common widget properties
    const char* name;           // Widget name (for CSS)
    const char* css_class;      // CSS class
    gboolean expand;            // Whether to expand in container
    gboolean fill;              // Whether to fill the allocated space
    guint padding;              // Padding around the widget
    
    // Alignment options
    GtkAlign halign;            // Horizontal alignment
    GtkAlign valign;            // Vertical alignment
    
    // Size options
    gint width;                 // Requested width (-1 for default)
    gint height;                // Requested height (-1 for default)
    gboolean use_size;          // Whether to use the width/height values
    
    // Margin options
    gint margin_top;            // Top margin
    gint margin_bottom;         // Bottom margin
    gint margin_left;           // Left margin
    gint margin_right;          // Right margin
    gboolean use_margin;        // Whether to use margin values
} WidgetOptions;

/**
 * Label creation options
 */
typedef struct {
    WidgetOptions common;       // Common widget options
    const char* text;           // Label text
    gboolean use_markup;        // Whether to use markup (HTML-like)
    gboolean selectable;        // Whether text is selectable
    GtkJustification justify;   // Text justification
    gfloat xalign;              // Horizontal text alignment (0.0-1.0)
    gfloat yalign;              // Vertical text alignment (0.0-1.0)
    PangoEllipsizeMode ellipsize; // Text ellipsis mode
    gint max_width_chars;       // Maximum width in characters
    const char* color;          // Text color (CSS color string)
    const char* font_desc;      // Font description
} LabelOptions;

/**
 * Container creation options
 */
typedef struct {
    WidgetOptions common;       // Common widget options
    guint spacing;              // Spacing between children
    guint border_width;         // Border width
    gboolean homogeneous;       // Whether children are the same size
} ContainerOptions;

/**
 * Progress bar creation options
 */
typedef struct {
    WidgetOptions common;       // Common widget options
    gdouble fraction;           // Progress bar value (0.0-1.0)
    const char* text;           // Progress bar text
    gboolean show_text;         // Whether to show text
    gboolean inverted;          // Whether to invert direction
    const char* css_class;      // CSS class for styling
} ProgressBarOptions;

/**
 * Drawing area creation options
 */
typedef struct {
    WidgetOptions common;       // Common widget options
    GCallback draw_func;        // Draw function
    gpointer user_data;         // User data for draw function
} DrawingAreaOptions;

/**
 * Create a default widget options structure
 * 
 * @return Default widget options
 */
WidgetOptions create_default_widget_options(void);

/**
 * Create a default label options structure
 * 
 * @return Default label options
 */
LabelOptions create_default_label_options(void);

/**
 * Create a default container options structure
 * 
 * @return Default container options
 */
ContainerOptions create_default_container_options(void);

/**
 * Create a default progress bar options structure
 * 
 * @return Default progress bar options
 */
ProgressBarOptions create_default_progress_bar_options(void);

/**
 * Create a default drawing area options structure
 * 
 * @return Default drawing area options
 */
DrawingAreaOptions create_default_drawing_area_options(void);

/**
 * Apply common widget options to a widget
 * 
 * @param widget Widget to apply options to
 * @param options Options to apply
 */
void apply_widget_options(GtkWidget* widget, const WidgetOptions* options);

/**
 * Create a GTK label with the specified options
 * 
 * @param options Label options
 * @return New label widget
 */
GtkWidget* create_label(const LabelOptions* options);

/**
 * Create a GTK label with markup text
 * 
 * @param markup Markup text for the label
 * @param options Additional options (can be NULL for defaults)
 * @return New label widget
 */
GtkWidget* create_markup_label(const char* markup, const LabelOptions* options);

/**
 * Create a GTK box container with the specified options
 * 
 * @param orientation Box orientation (GTK_ORIENTATION_HORIZONTAL or GTK_ORIENTATION_VERTICAL)
 * @param options Container options
 * @return New box container widget
 */
GtkWidget* create_box(GtkOrientation orientation, const ContainerOptions* options);

/**
 * Create a GTK frame with the specified options
 * 
 * @param label Frame label (can be NULL)
 * @param options Container options
 * @return New frame widget
 */
GtkWidget* create_frame(const char* label, const ContainerOptions* options);

/**
 * Create a GTK progress bar with the specified options
 * 
 * @param options Progress bar options
 * @return New progress bar widget
 */
GtkWidget* create_progress_bar(const ProgressBarOptions* options);

/**
 * Create a GTK drawing area with the specified options
 * 
 * @param options Drawing area options
 * @return New drawing area widget
 */
GtkWidget* create_drawing_area(const DrawingAreaOptions* options);

/**
 * Add a child widget to a container with specified options
 * 
 * @param container Container widget
 * @param child Child widget
 * @param expand Whether the child should expand
 * @param fill Whether the child should fill the allocated space
 * @param padding Padding around the child
 */
void add_to_container(GtkWidget* container, GtkWidget* child, 
                     gboolean expand, gboolean fill, guint padding);

/**
 * Add CSS class to a widget
 * 
 * @param widget Widget to add class to
 * @param css_class CSS class to add
 */
void add_css_class(GtkWidget* widget, const char* css_class);

/**
 * Set a widget name (for CSS)
 * 
 * @param widget Widget to set name for
 * @param name Name to set
 */
void set_widget_name(GtkWidget* widget, const char* name);

/**
 * Create a card-style UI container with a title and content area
 * 
 * @param title Card title
 * @param title_color Title color (CSS color string)
 * @param content_widget Content widget
 * @return Card container widget
 */
GtkWidget* create_card(const char* title, const char* title_color, GtkWidget* content_widget);

/**
 * Create a dashboard layout with multiple cards in a grid
 * 
 * @param row_spacing Row spacing
 * @param column_spacing Column spacing
 * @return Dashboard grid widget
 */
GtkWidget* create_dashboard_grid(guint row_spacing, guint column_spacing);

/**
 * Add a card to a dashboard grid
 * 
 * @param grid Dashboard grid
 * @param card Card widget
 * @param left Grid column
 * @param top Grid row
 * @param width Number of columns to span
 * @param height Number of rows to span
 */
void add_card_to_dashboard(GtkWidget* grid, GtkWidget* card, 
                          gint left, gint top, gint width, gint height);

/**
 * Update the text of a label widget
 * 
 * @param label Label widget
 * @param text New text
 * @param use_markup Whether to use markup
 */
void update_label_text(GtkLabel* label, const char* text, gboolean use_markup);

/**
 * Set CSS style for progress bar based on value
 * 
 * @param progress_bar Progress bar widget
 * @param value Current value (0.0-1.0)
 * @param low_threshold Threshold for "low" style
 * @param medium_threshold Threshold for "medium" style
 * @param high_threshold Threshold for "high" style
 * @param low_class CSS class for "low" style
 * @param medium_class CSS class for "medium" style
 * @param high_class CSS class for "high" style
 * @param critical_class CSS class for "critical" style
 */
void style_progress_bar_by_value(GtkWidget* progress_bar, gdouble value,
                                gdouble low_threshold, gdouble medium_threshold, 
                                gdouble high_threshold,
                                const char* low_class, const char* medium_class,
                                const char* high_class, const char* critical_class);

#endif // HAVE_GTK

#endif // GUI_UTILS_H 