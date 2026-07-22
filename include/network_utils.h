#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

// Tạo socket server, bind port, listen. Trả về server_fd (hoặc -1 nếu lỗi)
int create_server_socket(int port);

// Chờ và chấp nhận 1 client kết nối. Trả về client_fd
int accept_client(int server_fd);

// Kết nối tới server (dùng cho client). Trả về socket_fd (hoặc -1 nếu lỗi)
int connect_to_server(const char *ip, int port);

// Gửi đầy đủ n byte qua socket (xử lý trường hợp gửi thiếu)
int send_all(int sockfd, const void *buf, size_t len);

// Nhận đầy đủ n byte qua socket (xử lý trường hợp nhận thiếu)
int recv_all(int sockfd, void *buf, size_t len);

#endif