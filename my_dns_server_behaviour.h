//
// Created by Shaotien Lee on 2021/5/4.
//
#include "message_handler.h"

#ifndef COMP30023_2021_PROJECT_2_MASTER_MY_DNS_SERVER_BEHAVIOUR_H
#define COMP30023_2021_PROJECT_2_MASTER_MY_DNS_SERVER_BEHAVIOUR_H
void
Handle_non_AAAA_request(int new_socket_fd, dns_message_t *incoming_query_message, fd_set *active_socket_fd_set);

void Handle_AAAA_request(struct addrinfo *dns_server_info, int new_socket_fd, unsigned char *domain_name,
                         dns_message_t *incoming_query_message, fd_set *active_socket_fd_set);

void my_dns_server_run(struct addrinfo *dns_server_info, int listen_socket_fd, fd_set *active_socket_fd_set, int maxfd);

#endif //COMP30023_2021_PROJECT_2_MASTER_MY_DNS_SERVER_BEHAVIOUR_H
