#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/network_utils.h"
#include "../include/crypto_utils.h"

unsigned char AES_KEY[AES_KEY_SIZE] = "MySecretKey_32Bytes_ForAES256!!";

typedef struct {
    GtkWidget *window;
    GtkWidget *file_label;
    GtkWidget *ip_entry;
    GtkWidget *port_entry;
    GtkWidget *status_label;
    char selected_file[512];
} AppWidgets;

// Hàm xử lý khi bấm nút "Chọn file"
static void on_choose_file(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Chon file de gui", GTK_WINDOW(app->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        strncpy(app->selected_file, filename, sizeof(app->selected_file) - 1);
        gtk_label_set_text(GTK_LABEL(app->file_label), filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Hàm xử lý khi bấm nút "Gửi"
static void on_send_file(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;

    if (strlen(app->selected_file) == 0) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Vui long chon file truoc!");
        return;
    }

    const char *ip = gtk_entry_get_text(GTK_ENTRY(app->ip_entry));
    const char *port_str = gtk_entry_get_text(GTK_ENTRY(app->port_entry));
    int port = atoi(port_str);

    gtk_label_set_text(GTK_LABEL(app->status_label), "Dang gui...");
    // Ép GTK vẽ lại UI ngay (vì tác vụ sau có thể mất chút thời gian)
    while (gtk_events_pending()) gtk_main_iteration();

    FILE *fp = fopen(app->selected_file, "rb");
    if (!fp) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Loi: Khong mo duoc file!");
        return;
    }

    struct stat st;
    stat(app->selected_file, &st);
    long filesize = st.st_size;

    unsigned char *plaintext = malloc(filesize);
    fread(plaintext, 1, filesize, fp);
    fclose(fp);

    unsigned char iv[AES_IV_SIZE];
    generate_iv(iv);

    unsigned char *ciphertext = malloc(filesize + AES_BLOCK_SIZE);
    int cipher_len = encrypt_buffer(plaintext, filesize, AES_KEY, iv, ciphertext);

    int sockfd = connect_to_server(ip, port);
    if (sockfd < 0) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Loi: Khong ket noi duoc server!");
        free(plaintext);
        free(ciphertext);
        return;
    }

    long cipher_len_long = cipher_len;
    send_all(sockfd, &cipher_len_long, sizeof(cipher_len_long));
    send_all(sockfd, iv, AES_IV_SIZE);
    send_all(sockfd, ciphertext, cipher_len);

    close(sockfd);
    free(plaintext);
    free(ciphertext);

    char msg[256];
    snprintf(msg, sizeof(msg), "Da gui thanh cong! (%ld byte -> %d byte ma hoa)", filesize, cipher_len);
    gtk_label_set_text(GTK_LABEL(app->status_label), msg);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppWidgets app;
    memset(app.selected_file, 0, sizeof(app.selected_file));

    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "Client - Gui File Ma Hoa");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 450, 250);
    gtk_container_set_border_width(GTK_CONTAINER(app.window), 15);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);

    // Nút chọn file
    GtkWidget *choose_btn = gtk_button_new_with_label("Chon File");
    gtk_box_pack_start(GTK_BOX(vbox), choose_btn, FALSE, FALSE, 0);
    g_signal_connect(choose_btn, "clicked", G_CALLBACK(on_choose_file), &app);

    // Label hiển thị file đã chọn
    app.file_label = gtk_label_new("Chua chon file nao");
    gtk_box_pack_start(GTK_BOX(vbox), app.file_label, FALSE, FALSE, 0);

    // Ô nhập IP
    GtkWidget *ip_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *ip_label = gtk_label_new("Server IP:");
    app.ip_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app.ip_entry), "127.0.0.1");
    gtk_box_pack_start(GTK_BOX(ip_box), ip_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ip_box), app.ip_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), ip_box, FALSE, FALSE, 0);

    // Ô nhập Port
    GtkWidget *port_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *port_label = gtk_label_new("Port:");
    app.port_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app.port_entry), "8888");
    gtk_box_pack_start(GTK_BOX(port_box), port_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(port_box), app.port_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), port_box, FALSE, FALSE, 0);

    // Nút Gửi
    GtkWidget *send_btn = gtk_button_new_with_label("Gui File");
    gtk_box_pack_start(GTK_BOX(vbox), send_btn, FALSE, FALSE, 0);
    g_signal_connect(send_btn, "clicked", G_CALLBACK(on_send_file), &app);

    // Label trạng thái
    app.status_label = gtk_label_new("San sang");
    gtk_box_pack_start(GTK_BOX(vbox), app.status_label, FALSE, FALSE, 0);

    gtk_widget_show_all(app.window);
    gtk_main();

    return 0;
}