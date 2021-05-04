#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "network_handler.h"
#include "my_dns_server_behaviour.h"

#define TRUE 1
#define FALSE 0

#define NONBLOCKING

// DNS default port 53
int main(int argc, char *argv[]) {

    // this server's listening port
    char *port_number = "8053";
    struct addrinfo *dns_server_info, *this_server_info;
    int listen_socket_fd;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port for dns\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /**
     * My dns server configurations
     */

    /* get address information */
    printf("DNS：%s, %s\n", argv[1], argv[2]);
    dns_server_info = get_dns_server_info(argv[1], argv[2]);
    this_server_info = get_this_server_info(port_number);

    /* reused listening socket */
    listen_socket_fd = get_listening_socket_fd(this_server_info);

    /* initialise active descriptors set */
    fd_set active_socket_fd_set;
    FD_ZERO(&active_socket_fd_set);
    FD_SET(listen_socket_fd, &active_socket_fd_set);
    /* record the maximum socket number */
    int maxfd = listen_socket_fd;

    /* run it */
    my_dns_server_run(dns_server_info, listen_socket_fd, &active_socket_fd_set, maxfd);

    /* at the end of the program */
    freeaddrinfo(this_server_info);
    freeaddrinfo(dns_server_info);
    close(listen_socket_fd);

    return 0;
}