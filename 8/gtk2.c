#include <gtk/gtk.h>

void quit() {
    gtk_main_quit();
}

void button_clicked(GtkWidget *entry, GtkWidget *text_entry) {
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(text_entry));
    g_print("%s\n", text);
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *button;
    GtkWidget *vbox;
    GtkWidget *text_entry;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);

    button = gtk_button_new_with_label("버튼");
    text_entry = gtk_entry_new();

    g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), text_entry);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    gtk_container_add(GTK_CONTAINER(vbox), text_entry);
    gtk_container_add(GTK_CONTAINER(vbox), button);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
