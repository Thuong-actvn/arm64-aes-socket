#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <stddef.h>

#define AES_KEY_SIZE 32   // AES-256 = 32 byte key
#define AES_IV_SIZE  16   // AES block size = 16 byte IV
#define AES_BLOCK_SIZE 16

// Sinh IV ngẫu nhiên (16 byte)
void generate_iv(unsigned char *iv);

// Mã hoá buffer, trả về độ dài ciphertext (có padding)
// out phải được cấp phát trước, tối thiểu in_len + AES_BLOCK_SIZE
int encrypt_buffer(const unsigned char *in, int in_len,
                    const unsigned char *key, const unsigned char *iv,
                    unsigned char *out);

// Giải mã buffer, trả về độ dài plaintext gốc
int decrypt_buffer(const unsigned char *in, int in_len,
                    const unsigned char *key, const unsigned char *iv,
                    unsigned char *out);

#endif