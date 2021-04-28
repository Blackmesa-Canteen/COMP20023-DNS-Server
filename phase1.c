#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char* port_number = "8053";
    struct addrinfo listen_hints, dns_hints, *this_server_info, *dns_server_info;
    int re, listen_socket_fd, dns_socket_fd;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port for dns\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // create address for upper dns
    memset(&dns_hints, 0, sizeof dns_hints);
    // ip v4
    dns_hints.ai_family = AF_INET;
    // TCP
    dns_hints.ai_socktype = SOCK_STREAM;

    // set address info for dns
    if ((getaddrinfo(argv[1], argv[2], &dns_hints, &dns_server_info)) < 0) {
        perror("getaddrinfo for upper dns");
        exit(EXIT_FAILURE);
    }

    // create address for this server to listen to
    memset(&listen_hints, 0, sizeof listen_hints);
    // ip v4
    listen_hints.ai_family = AF_INET;
    // TCP
    listen_hints.ai_socktype = SOCK_STREAM;
    // listen accept
    listen_hints.ai_flags = AI_PASSIVE;

    // set address info for listening
    if(getaddrinfo(NULL, port_number, &listen_hints, &this_server_info) < 0){
        perror("getaddrinfo for this server");
        exit(EXIT_FAILURE);
    }

    // create upper dns socket

    // create listening socket
    listen_socket_fd = socket(this_server_info->ai_family, this_server_info->ai_socktype, this_server_info->ai_protocol);
    if (listen_socket_fd < 0) {
        perror("listen socket");
        exit(EXIT_FAILURE);
    }

    // Reuse port if possible
    re = 1;
    if (setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
        perror("listen setsockopt");
        exit(EXIT_FAILURE);
    }
    // Bind address to the socket
    if (bind(listen_socket_fd, this_server_info->ai_addr, this_server_info->ai_addrlen) < 0) {
        perror("listen bind");
        exit(EXIT_FAILURE);
    }

    // listen on socket
    if (listen(listen_socket_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return 0;
}
