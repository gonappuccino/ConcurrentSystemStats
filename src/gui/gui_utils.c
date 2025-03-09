#include "gui_utils.h"

#ifdef HAVE_GTK

/**
 * Create default widget options
 */
WidgetOptions create_default_widget_options(void) {
    WidgetOptions options = {
        .name = NULL,
        .css_class = NULL,
        .expand = FALSE,
        .fill = TRUE,
        .padding = 0,
        .halign = GTK_ALIGN_FILL,
        .valign = GTK_ALIGN_FILL,
        .width = -1,
        .height = -1,
        .use_size = FALSE,
        .margin_top = 0,
        .margin_bottom = 0,
        .margin_left = 0,
        .margin_right = 0,
        .use_margin = FALSE
    };
    
    return options;
}

/**
 * Create default label options
 */
LabelOptions create_default_label_options(void) {
    LabelOptions options = {
        .common = create_default_widget_options(),
        .text = NULL,
        .use_markup = FALSE,
        .selectable = FALSE,
        .justify = GTK_JUSTIFY_LEFT,
        .xalign = 0.0,
        .yalign = 0.5,
        .ellipsize = PANGO_ELLIPSIZE_NONE,
        .max_width_chars = -1,
        .color = NULL,
        .font_desc = NULL
    };
    
    return options;
}

/**
 * Create default container options
 */
ContainerOptions create_default_container_options(void) {
    ContainerOptions options = {
        .common = create_default_widget_options(),
        .spacing = 0,
        .border_width = 0,
        .homogeneous = FALSE
    };
    
    return options;
}

/**
 * Create default progress bar options
 */
ProgressBarOptions create_default_progress_bar_options(void) {
    ProgressBarOptions options = {
        .common = create_default_widget_options(),
        .fraction = 0.0,
        .text = NULL,
        .show_text = FALSE,
        .inverted = FALSE,
        .css_class = NULL
    };
    
    return options;
}

/**
 * Create default drawing area options
 */
DrawingAreaOptions create_default_drawing_area_options(void) {
    DrawingAreaOptions options = {
        .common = create_default_widget_options(),
        .draw_func = NULL,
        .user_data = NULL
    };
    
    return options;
}

/**
 * Apply common widget options to a widget
 */
void apply_widget_options(GtkWidget* widget, const WidgetOptions* options) {
    if (widget == NULL || options == NULL) {
        return;
    }
    
    // Set widget name
    if (options->name != NULL) {
        gtk_widget_set_name(widget, options->name);
    }
    
    // Apply CSS class
    if (options->css_class != NULL) {
        GtkStyleContext* context = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(context, options->css_class);
    }
    
    // Set alignment
    gtk_widget_set_halign(widget, options->halign);
    gtk_widget_set_valign(widget, options->valign);
    
    // Set size request if needed
    if (options->use_size) {
        gtk_widget_set_size_request(widget, options->width, options->height);
    }
    
    // Set margins if needed
    if (options->use_margin) {
        gtk_widget_set_margin_top(widget, options->margin_top);
        gtk_widget_set_margin_bottom(widget, options->margin_bottom);
        gtk_widget_set_margin_start(widget, options->margin_left);
        gtk_widget_set_margin_end(widget, options->margin_right);
    }
}

/**
 * Create a GTK label with the specified options
 */
GtkWidget* create_label(const LabelOptions* options) {
    GtkWidget* label;
    
    if (options == NULL) {
        // Create a default label
        label = gtk_label_new(NULL);
        return label;
    }
    
    if (options->use_markup && options->text != NULL) {
        label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label), options->text);
    } else {
        label = gtk_label_new(options->text);
    }
    
    // Apply common widget options
    apply_widget_options(label, &options->common);
    
    // Apply label-specific options
    gtk_label_set_selectable(GTK_LABEL(label), options->selectable);
    gtk_label_set_justify(GTK_LABEL(label), options->justify);
    gtk_label_set_xalign(GTK_LABEL(label), options->xalign);
    gtk_label_set_yalign(GTK_LABEL(label), options->yalign);
    
    if (options->ellipsize != PANGO_ELLIPSIZE_NONE) {
        gtk_label_set_ellipsize(GTK_LABEL(label), options->ellipsize);
    }
    
    if (options->max_width_chars > 0) {
        gtk_label_set_max_width_chars(GTK_LABEL(label), options->max_width_chars);
    }
    
    // Apply font and color if specified
    if (options->font_desc != NULL || options->color != NULL) {
        char* markup_text = NULL;
        
        if (options->text != NULL) {
            // Start with base text
            const char* base_text = options->text;
            
            // Create markup with font and color attributes
            if (options->font_desc != NULL && options->color != NULL) {
                markup_text = g_markup_printf_escaped("<span font_desc='%s' foreground='%s'>%s</span>", 
                                                     options->font_desc, options->color, base_text);
            } else if (options->font_desc != NULL) {
                markup_text = g_markup_printf_escaped("<span font_desc='%s'>%s</span>", 
                                                     options->font_desc, base_text);
            } else if (options->color != NULL) {
                markup_text = g_markup_printf_escaped("<span foreground='%s'>%s</span>", 
                                                     options->color, base_text);
            }
            
            if (markup_text != NULL) {
                gtk_label_set_markup(GTK_LABEL(label), markup_text);
                g_free(markup_text);
            }
        }
    }
    
    return label;
}

/**
 * Create a GTK label with markup text
 */
GtkWidget* create_markup_label(const char* markup, const LabelOptions* options) {
    LabelOptions local_options;
    
    if (options == NULL) {
        local_options = create_default_label_options();
    } else {
        local_options = *options;
    }
    
    local_options.text = markup;
    local_options.use_markup = TRUE;
    
    return create_label(&local_options);
}

/**
 * Create a GTK box container with the specified options
 */
GtkWidget* create_box(GtkOrientation orientation, const ContainerOptions* options) {
    GtkWidget* box;
    
    if (options == NULL) {
        // Create a default box
        box = gtk_box_new(orientation, 0);
        return box;
    }
    
    box = gtk_box_new(orientation, options->spacing);
    
    // Apply common widget options
    apply_widget_options(box, &options->common);
    
    // Apply container-specific options
    gtk_container_set_border_width(GTK_CONTAINER(box), options->border_width);
    gtk_box_set_homogeneous(GTK_BOX(box), options->homogeneous);
    
    return box;
}

/**
 * Create a GTK frame with the specified options
 */
GtkWidget* create_frame(const char* label, const ContainerOptions* options) {
    GtkWidget* frame;
    
    if (label == NULL) {
        frame = gtk_frame_new(NULL);
    } else {
        frame = gtk_frame_new(label);
    }
    
    if (options != NULL) {
        // Apply common widget options
        apply_widget_options(frame, &options->common);
        
        // Apply container-specific options
        gtk_container_set_border_width(GTK_CONTAINER(frame), options->border_width);
    }
    
    return frame;
}

/**
 * Create a GTK progress bar with the specified options
 */
GtkWidget* create_progress_bar(const ProgressBarOptions* options) {
    GtkWidget* progress_bar = gtk_progress_bar_new();
    
    if (options == NULL) {
        return progress_bar;
    }
    
    // Apply common widget options
    apply_widget_options(progress_bar, &options->common);
    
    // Apply progress bar-specific options
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), options->fraction);
    
    if (options->text != NULL) {
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), options->text);
        gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
    } else {
        gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), options->show_text);
    }
    
    gtk_progress_bar_set_inverted(GTK_PROGRESS_BAR(progress_bar), options->inverted);
    
    // Apply CSS class if specified
    if (options->css_class != NULL) {
        GtkStyleContext* context = gtk_widget_get_style_context(progress_bar);
        gtk_style_context_add_class(context, options->css_class);
    }
    
    return progress_bar;
}

/**
 * Create a GTK drawing area with the specified options
 */
GtkWidget* create_drawing_area(const DrawingAreaOptions* options) {
    GtkWidget* drawing_area = gtk_drawing_area_new();
    
    if (options == NULL) {
        return drawing_area;
    }
    
    // Apply common widget options
    apply_widget_options(drawing_area, &options->common);
    
    // Set the draw function if provided
    if (options->draw_func != NULL) {
        g_signal_connect(drawing_area, "draw", 
                       G_CALLBACK(options->draw_func), options->user_data);
    }
    
    return drawing_area;
}

/**
 * Add a child widget to a container with specified options
 */
void add_to_container(GtkWidget* container, GtkWidget* child, 
                     gboolean expand, gboolean fill, guint padding) {
    if (container == NULL || child == NULL) {
        return;
    }
    
    if (GTK_IS_BOX(container)) {
        gtk_box_pack_start(GTK_BOX(container), child, expand, fill, padding);
    } else if (GTK_IS_CONTAINER(container)) {
        gtk_container_add(GTK_CONTAINER(container), child);
    }
}

/**
 * Add CSS class to a widget
 */
void add_css_class(GtkWidget* widget, const char* css_class) {
    if (widget == NULL || css_class == NULL) {
        return;
    }
    
    GtkStyleContext* context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, css_class);
}

/**
 * Set a widget name (for CSS)
 */
void set_widget_name(GtkWidget* widget, const char* name) {
    if (widget == NULL || name == NULL) {
        return;
    }
    
    gtk_widget_set_name(widget, name);
}

/**
 * Create a card-style UI container with a title and content area
 */
GtkWidget* create_card(const char* title, const char* title_color, GtkWidget* content_widget) {
    GtkWidget* card_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card_frame), GTK_SHADOW_ETCHED_IN);
    
    GtkWidget* card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(card_box), 10);
    gtk_container_add(GTK_CONTAINER(card_frame), card_box);
    
    // Create title if provided
    if (title != NULL) {
        GtkWidget* header_label = gtk_label_new(NULL);
        
        // Format title with markup
        char* markup;
        if (title_color != NULL) {
            markup = g_markup_printf_escaped("<span font_desc='Monospace Bold 12' foreground='%s'>%s</span>", 
                                           title_color, title);
        } else {
            markup = g_markup_printf_escaped("<span font_desc='Monospace Bold 12'>%s</span>", title);
        }
        
        gtk_label_set_markup(GTK_LABEL(header_label), markup);
        g_free(markup);
        
        gtk_widget_set_halign(header_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(card_box), header_label, FALSE, FALSE, 0);
    }
    
    // Add content widget if provided
    if (content_widget != NULL) {
        gtk_box_pack_start(GTK_BOX(card_box), content_widget, TRUE, TRUE, 5);
    }
    
    return card_frame;
}

/**
 * Create a dashboard layout with multiple cards in a grid
 */
GtkWidget* create_dashboard_grid(guint row_spacing, guint column_spacing) {
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), row_spacing);
    gtk_grid_set_column_spacing(GTK_GRID(grid), column_spacing);
    
    return grid;
}

/**
 * Add a card to a dashboard grid
 */
void add_card_to_dashboard(GtkWidget* grid, GtkWidget* card, 
                          gint left, gint top, gint width, gint height) {
    if (grid == NULL || card == NULL) {
        return;
    }
    
    gtk_grid_attach(GTK_GRID(grid), card, left, top, width, height);
}

/**
 * Update the text of a label widget
 */
void update_label_text(GtkLabel* label, const char* text, gboolean use_markup) {
    if (label == NULL || text == NULL) {
        return;
    }
    
    if (use_markup) {
        gtk_label_set_markup(label, text);
    } else {
        gtk_label_set_text(label, text);
    }
}

/**
 * Set CSS style for progress bar based on value
 */
void style_progress_bar_by_value(GtkWidget* progress_bar, gdouble value,
                                gdouble low_threshold, gdouble medium_threshold, 
                                gdouble high_threshold,
                                const char* low_class, const char* medium_class,
                                const char* high_class, const char* critical_class) {
    if (progress_bar == NULL) {
        return;
    }
    
    GtkStyleContext* context = gtk_widget_get_style_context(progress_bar);
    
    // Remove all possible classes
    if (low_class != NULL) {
        gtk_style_context_remove_class(context, low_class);
    }
    
    if (medium_class != NULL) {
        gtk_style_context_remove_class(context, medium_class);
    }
    
    if (high_class != NULL) {
        gtk_style_context_remove_class(context, high_class);
    }
    
    if (critical_class != NULL) {
        gtk_style_context_remove_class(context, critical_class);
    }
    
    // Apply appropriate class based on value
    if (value >= high_threshold) {
        if (critical_class != NULL) {
            gtk_style_context_add_class(context, critical_class);
        }
    } else if (value >= medium_threshold) {
        if (high_class != NULL) {
            gtk_style_context_add_class(context, high_class);
        }
    } else if (value >= low_threshold) {
        if (medium_class != NULL) {
            gtk_style_context_add_class(context, medium_class);
        }
    } else {
        if (low_class != NULL) {
            gtk_style_context_add_class(context, low_class);
        }
    }
}

#endif // HAVE_GTK 