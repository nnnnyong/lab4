#include <gtk/gtk.h>

GtkWidget *entry;
int op1, op2;
char operator;

void quit(GtkWidget *window, gpointer data) {
    gtk_main_quit();
}

void clear() {
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void clicked_number(GtkWidget *widget, gpointer data) {
    const gchar *button_label = gtk_button_get_label(GTK_BUTTON(widget));
    gchar *current_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    gchar *new_text = g_strconcat(current_text, button_label, NULL);

    gtk_entry_set_text(GTK_ENTRY(entry), new_text);
    g_free(current_text);
    g_free(new_text);
}

void click_operator(GtkWidget *widget, gpointer data) {
    const gchar *button_label = gtk_button_get_label(GTK_BUTTON(widget));
    gchar *current_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    op1 = atoi(current_text);
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    operator = button_label[0];
    g_free(current_text);
}

void calculate_result() {
    gchar *current_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    op2 = atoi(current_text);
    g_free(current_text);

    int result;
    switch (operator) {
        case '+':
            result = op1 + op2;
            break;
        case '-':
            result = op1 - op2;
            break;
        case '*':
            result = op1 * op2;
            break;
        case '/':
            if (op2 != 0) {
                result = op1 / op2;
            } else {
                gtk_entry_set_text(GTK_ENTRY(entry), "0 devide");
                return;
            }
            break;
        default:
            return;
    }

    gchar result_text[50];
    g_snprintf(result_text, sizeof(result_text), "%d", result);
    gtk_entry_set_text(GTK_ENTRY(entry), result_text);
}

void create_button(GtkWidget *box, GtkWidget *button, const gchar *button_label) {
    button = gtk_button_new_with_label(button_label);
    g_signal_connect(button, "clicked", G_CALLBACK(clicked_number), NULL);
    gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
}

int main(int argc, char *argv[]) {
    // GTK 초기화
    gtk_init(&argc, &argv);

    // 윈도우 생성
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK+ Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(vbox), entry);

    GtkWidget *button = gtk_button_new_with_label("clear");
    g_signal_connect(button, "clicked", G_CALLBACK(clear), NULL);
    gtk_container_add(GTK_CONTAINER(vbox), button); 

    GtkWidget *hbox1, *hbox2, *hbox3, *hbox4;

    hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    create_button(hbox1, button, "7");
    create_button(hbox1, button, "8");
    create_button(hbox1, button, "9");
    button = gtk_button_new_with_label("/");
    g_signal_connect(button, "clicked", G_CALLBACK(click_operator), NULL);
    gtk_container_add(GTK_CONTAINER(hbox1), button);
    gtk_container_add(GTK_CONTAINER(vbox), hbox1);

    hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    create_button(hbox2, button, "4");
    create_button(hbox2, button, "5");
    create_button(hbox2, button, "6");
    button = gtk_button_new_with_label("*");
    g_signal_connect(button, "clicked", G_CALLBACK(click_operator), NULL);
    gtk_container_add(GTK_CONTAINER(hbox2), button);
    gtk_container_add(GTK_CONTAINER(vbox), hbox2);

    hbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    create_button(hbox3, button, "1");
    create_button(hbox3, button, "2");
    create_button(hbox3, button, "3");
    button = gtk_button_new_with_label("-");
    g_signal_connect(button, "clicked", G_CALLBACK(click_operator), NULL);
    gtk_container_add(GTK_CONTAINER(hbox3), button);
    gtk_container_add(GTK_CONTAINER(vbox), hbox3);

    hbox4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    create_button(hbox4, button, "0");
    create_button(hbox4, button, "");
    button = gtk_button_new_with_label("=");
    g_signal_connect(button, "clicked", G_CALLBACK(calculate_result), NULL);
    gtk_container_add(GTK_CONTAINER(hbox4), button);
    button = gtk_button_new_with_label("+");
    g_signal_connect(button, "clicked", G_CALLBACK(click_operator), NULL);
    gtk_container_add(GTK_CONTAINER(hbox4), button);
    gtk_container_add(GTK_CONTAINER(vbox), hbox4);

    // 윈도우와 그 안의 위젯들을 화면에 표시
    gtk_widget_show_all(window);

    // 메인 이벤트 루프 실행
    gtk_main();

    return 0;
}
