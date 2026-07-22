#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>                          // MỚI: cần cho time()
#include "../include/network_utils.h"
#include "../include/crypto_utils.h"

#define PORT 8888
#define MAX_FILE_SIZE (10 * 1024 * 1024 + 1024)

// Key AES-256 cố định (PHẢI GIỐNG HỆT bên client)
unsigned char AES_KEY[AES_KEY_SIZE] = "MySecretKey_32Bytes_ForAES256!!";

int main() {
    int server_fd = create_server_socket(PORT);
    if (server_fd < 0) return 1;

    unsigned long file_sequence = 0;

    while (1) {
        int client_fd = accept_client(server_fd);
        if (client_fd < 0) continue;

        // Nhận độ dài ciphertext
        long cipher_len;
        if (recv_all(client_fd, &cipher_len, sizeof(cipher_len)) < 0) {
            close(client_fd);
            continue;
        }

        if (cipher_len <= 0 ||
            cipher_len > MAX_FILE_SIZE ||
            cipher_len % AES_BLOCK_SIZE != 0) {
            printf("Do dai ciphertext khong hop le, bo qua.\n");
            close(client_fd);
            continue;
        }

        // Nhận IV
        unsigned char iv[AES_IV_SIZE];
        if (recv_all(client_fd, iv, AES_IV_SIZE) < 0) {
            close(client_fd);
            continue;
        }

        // Nhận ciphertext
        unsigned char *ciphertext = malloc(cipher_len);
        if (!ciphertext) {
            perror("Khong the cap phat bo nho cho ciphertext");
            close(client_fd);
            continue;
        }

        if (recv_all(client_fd, ciphertext, cipher_len) < 0) {
            free(ciphertext);
            close(client_fd);
            continue;
        }

        printf("Nhan duoc ciphertext: %ld byte\n", cipher_len);

        // Dinh dang file: [cipher_len][IV][ciphertext]
        time_t received_at = time(NULL);
        unsigned long current_sequence = file_sequence++;
        char encrypted_filename[96];
        snprintf(encrypted_filename, sizeof(encrypted_filename),
                 "encrypted_%ld_%lu.bin", (long)received_at, current_sequence);

        FILE *encrypted_fp = fopen(encrypted_filename, "wb");
        if (!encrypted_fp) {
            perror("Khong the tao file ciphertext");
            free(ciphertext);
            close(client_fd);
            continue;
        }

        fwrite(&cipher_len, sizeof(cipher_len), 1, encrypted_fp);
        fwrite(iv, 1, AES_IV_SIZE, encrypted_fp);
        fwrite(ciphertext, 1, cipher_len, encrypted_fp);
        fclose(encrypted_fp);
        free(ciphertext);

        // Doc lai tu file de giai ma
        encrypted_fp = fopen(encrypted_filename, "rb");
        if (!encrypted_fp) {
            perror("Khong the mo lai file du lieu ma hoa");
            close(client_fd);
            continue;
        }

        fread(&cipher_len, sizeof(cipher_len), 1, encrypted_fp);
        fread(iv, 1, AES_IV_SIZE, encrypted_fp);
        ciphertext = malloc(cipher_len);
        fread(ciphertext, 1, cipher_len, encrypted_fp);
        fclose(encrypted_fp);

        unsigned char *plaintext = malloc(cipher_len);
        int plain_len =
            decrypt_buffer(ciphertext, cipher_len, AES_KEY, iv, plaintext);
        free(ciphertext);

        printf("Giai ma thanh cong: %d byte\n", plain_len);

        // Lưu ra file — tên theo timestamp, tránh ghi đè khi gửi nhiều file
        char save_filename[64];                                          // MỚI
        snprintf(save_filename, sizeof(save_filename),                   // MỚI
                 "received_%ld.bin", (long)time(NULL));                  // MỚI
        FILE *fp = fopen(save_filename, "wb");                           // SỬA
        fwrite(plaintext, 1, plain_len, fp);
        fclose(fp);

        printf("Da luu file giai ma tai %s\n", save_filename);           // SỬA

        free(plaintext);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
