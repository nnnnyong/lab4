#include <gtk/gtk.h>
#include <stdio.h>

void quit ( GtkWidget *window, gpointer data) {
    gtk_main_quit();
}

int main (int argc, char *argv[]) {
    GtkWidget *window, *dialog;
    gint result;
    char username[10] = "홍길동";

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size ( GTK_WINDOW(window), 200, 80);
    g_signal_connect ( window, "destroy", G_CALLBACK (quit), NULL);

    dialog = gtk_message_dialog_new (GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s님, 이제 그만 주무시겠습니까?", username); 

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy(dialog);

    switch (result) {
        case GTK_RESPONSE_YES:
            printf("네, 안녕히 주무세요.\n");
            break;

        case GTK_RESPONSE_NO:
            printf("일찍 주무세요.\n");
            break;
        default:
            printf("선택하지 않았습니다.\n");
    }

    gtk_widget_destroy(dialog);

    return 0;
}
