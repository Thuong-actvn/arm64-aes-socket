#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/network_utils.h"

int create_server_socket(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        return -1;
    }

    printf("Server dang lang nghe tren port %d...\n", port);
    return server_fd;
}

int accept_client(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    if (client_fd < 0) {
        perror("accept failed");
        return -1;
    }

    printf("Client da ket noi: %s\n", inet_ntoa(client_addr.sin_addr));
    return client_fd;
}

int connect_to_server(const char *ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int send_all(int sockfd, const void *buf, size_t len) {
    size_t total_sent = 0;
    const char *ptr = (const char*)buf;

    while (total_sent < len) {
        ssize_t sent = send(sockfd, ptr + total_sent, len - total_sent, 0);
        if (sent <= 0) {
            perror("send failed");
            return -1;
        }
        total_sent += sent;
    }
    return 0;
}

int recv_all(int sockfd, void *buf, size_t len) {
    size_t total_recv = 0;
    char *ptr = (char*)buf;

    while (total_recv < len) {
        ssize_t received = recv(sockfd, ptr + total_recv, len - total_recv, 0);
        if (received <= 0) {
            perror("recv failed or connection closed");
            return -1;
        }
        total_recv += received;
    }
    return 0;
}