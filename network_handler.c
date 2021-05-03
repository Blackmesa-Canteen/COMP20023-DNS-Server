#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "network_handler.h"
/**
 * set a listening socket fd on a specific port
 * @param port "8053"
 * @return listened socket fd
 */
int get_listening_socket_fd(struct addrinfo *this_server_info) {
    int listen_socket_fd;
    int re;

    // create listening socket
    listen_socket_fd = socket(this_server_info->ai_family, this_server_info->ai_socktype,
                              this_server_info->ai_protocol);
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

    return listen_socket_fd;
}


/**
 * get dns server address info
 * @param host DNS ipv4 address (main() argument1)
 * @param port port number (main() argument2)
 * @return DNS_server_info used to set up connection
 */
struct addrinfo* get_dns_server_info(char* host, char* port) {
    struct addrinfo dns_hints, *dns_server_info;
    // create address for upper dns
    memset(&dns_hints, 0, sizeof dns_hints);
    // ip v4
    dns_hints.ai_family = AF_INET;
    // TCP
    dns_hints.ai_socktype = SOCK_STREAM;

    // set address info for dns
    if ((getaddrinfo(host, port, &dns_hints, &dns_server_info)) < 0) {
        perror("getaddrinfo for upper dns");
        exit(EXIT_FAILURE);
    }

    return dns_server_info;
}

/**
 * get this server's connection info
 * @param port local part
 * @return addrinfo*
 */
struct addrinfo* get_this_server_info(char* port) {
    struct addrinfo listen_hints, *this_server_info;

    // create address for this server to listen to
    memset(&listen_hints, 0, sizeof listen_hints);
    // ip v4
    listen_hints.ai_family = AF_INET;
    // TCP
    listen_hints.ai_socktype = SOCK_STREAM;
    // listen accept
    listen_hints.ai_flags = AI_PASSIVE;

    // set address info for listening
    if (getaddrinfo(NULL, port, &listen_hints, &this_server_info) < 0) {
        perror("getaddrinfo for this server");
        exit(EXIT_FAILURE);
    }

    return this_server_info;
}


/**
 * get DNS socket_fd
 * @param dns_server_info
 * @return
 */
int get_dns_connection(struct addrinfo *dns_server_info) {
    struct addrinfo *rp;
    int sockfd;

    // Connect to first valid result
    for (rp = dns_server_info; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1)
            continue;

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; // success

        close(sockfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "client: failed to connect to DNS server\n");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

/**
 * forward query from client to the dns
 * @param dns_sockfd socketfd
 * @param original_query original query message from client
 * @param message_size query size(except 2 bytes header)
 */
void forward_domin_request(int dns_sockfd, const unsigned char* original_query, int message_size){
    int n;
    unsigned char buffer[message_size + 2];

    // enter query info into sending buffer
    memset(buffer, 0, message_size + 2);
    for(int i = 0; i < message_size + 2; i++) {
        // copy query message into sending buffer
        buffer[i] = original_query[i];
    }

    n = write(dns_sockfd, buffer, message_size + 2);
    if(n < 0) {
        perror("forwarding socket error");
        exit(EXIT_FAILURE);
    }
}

/**
 * get dns's answer response binary array, return it
 * @param sockfd
 */
unsigned char* get_domin_result(int dns_sockfd) {
    // size_head_buffer for first two bytes
    unsigned char size_head_buffer[2];

    // read two bytes from fd
    read(dns_sockfd, size_head_buffer, 2);

    // get message_size from binary size_head_buffer
    int message_size = (size_head_buffer[0] << 8 | size_head_buffer[1]);

    printf("dns server response message part size: %d\n", message_size);

    // read the real message
    unsigned char dns_msg_buffer[message_size];
    int n = read(dns_sockfd, dns_msg_buffer, message_size);
    if (n < 0) {
        perror(" error in get domin result socket");
        exit(EXIT_FAILURE);
    }

    /**
     * Reconstruct original response message
     */
    unsigned char* original_response = (unsigned char*) calloc(message_size + 2, sizeof (unsigned char));
    unsigned char* original_response_ptr = original_response;
    for(int i = 0; i < 2; i++) {
        original_response_ptr[0] = size_head_buffer[i];
        original_response_ptr++;
    }

    for(int i = 0; i < message_size; i++) {
        original_response_ptr[0] = dns_msg_buffer[i];
        original_response_ptr++;
    }

    return original_response;
}
