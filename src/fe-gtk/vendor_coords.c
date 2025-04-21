#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>  // Required for g_base64_encode

static void update_output(GtkEditable *editable, gpointer user_data) {
    GtkWidget *x_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(editable), "x_entry"));
    GtkWidget *y_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(editable), "y_entry"));
    GtkWidget *output_buffer = GTK_WIDGET(g_object_get_data(G_OBJECT(editable), "output_buffer"));

    const char *x_text = gtk_entry_get_text(GTK_ENTRY(x_entry));
    const char *y_text = gtk_entry_get_text(GTK_ENTRY(y_entry));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_buffer));

    if (strlen(x_text) > 0 && strlen(y_text) > 0) {
        int x = atoi(x_text);
        int y = atoi(y_text);

        // Use the UTF-8 encoded ␟ (U+241F)
        char raw_string[512];
        snprintf(raw_string, sizeof(raw_string),
            "#uooutlands␟vendorlocation␟Daddy␟new item name␟%d␟%d␟11331355␟0x449BAB01␟1322683753",
            x, y);

        // Encode to Base64 using glib
        gchar *encoded = g_base64_encode((const guchar *)raw_string, strlen(raw_string));
        gtk_text_buffer_set_text(buffer, encoded, -1);
        g_free(encoded);
    } else {
        gtk_text_buffer_set_text(buffer, "Encoded string will appear here...", -1);
    }
}


static void copy_to_clipboard(GtkWidget *button, gpointer user_data) {
    GtkTextView *text_view = GTK_TEXT_VIEW(user_data);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);

    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    gtk_clipboard_set_text(clipboard, text, -1);
    g_free(text);
}

void open_vendor_coords_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *table;
    GtkWidget *x_label, *y_label, *x_entry, *y_entry, *output_view, *copy_button;

    dialog = gtk_dialog_new_with_buttons("Vendor Coords Generator",
                                         NULL,
                                         GTK_DIALOG_MODAL,
                                         GTK_STOCK_CLOSE,
                                         GTK_RESPONSE_CLOSE,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    table = gtk_table_new(4, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 6);
    gtk_table_set_col_spacings(GTK_TABLE(table), 6);
    gtk_container_add(GTK_CONTAINER(content_area), table);

    x_label = gtk_label_new("X:");
    y_label = gtk_label_new("Y:");
    x_entry = gtk_entry_new();
    y_entry = gtk_entry_new();

    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_WORD_CHAR);

    GtkWidget *output_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(output_scroll), output_view);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(output_scroll, 300, 60);

    copy_button = gtk_button_new_with_label("Copy to Clipboard");

    gtk_table_attach(GTK_TABLE(table), x_label,      0, 1, 0, 1, GTK_FILL, GTK_FILL, 4, 4);
    gtk_table_attach(GTK_TABLE(table), x_entry,      1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 4, 4);
    gtk_table_attach(GTK_TABLE(table), y_label,      0, 1, 1, 2, GTK_FILL, GTK_FILL, 4, 4);
    gtk_table_attach(GTK_TABLE(table), y_entry,      1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 4, 4);
    gtk_table_attach(GTK_TABLE(table), output_scroll,0, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_FILL, 4, 4);
    gtk_table_attach(GTK_TABLE(table), copy_button,  0, 2, 3, 4, GTK_SHRINK, GTK_FILL, 4, 4);

    gtk_widget_show_all(dialog);

    // Set data for signals
    g_object_set_data(G_OBJECT(x_entry), "x_entry", x_entry);
    g_object_set_data(G_OBJECT(x_entry), "y_entry", y_entry);
    g_object_set_data(G_OBJECT(x_entry), "output_buffer", output_view);

    g_object_set_data(G_OBJECT(y_entry), "x_entry", x_entry);
    g_object_set_data(G_OBJECT(y_entry), "y_entry", y_entry);
    g_object_set_data(G_OBJECT(y_entry), "output_buffer", output_view);

    g_signal_connect(x_entry, "changed", G_CALLBACK(update_output), NULL);
    g_signal_connect(y_entry, "changed", G_CALLBACK(update_output), NULL);
    g_signal_connect(copy_button, "clicked", G_CALLBACK(copy_to_clipboard), output_view);
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}
