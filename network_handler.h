//
// Created by xiaotian on 2021/4/30.
//

#ifndef COMP30023_2021_PROJECT_2_MASTER_NETWORK_HANDLER_H
#define COMP30023_2021_PROJECT_2_MASTER_NETWORK_HANDLER_H
int get_listening_socket_fd(struct addrinfo *this_server_info);
struct addrinfo* get_dns_server_info(char* host, char* port);
struct addrinfo* get_this_server_info(char* port);
int get_dns_connection(struct addrinfo *dns_server_info);
void forward_domin_request(int dns_sockfd, const unsigned char* original_query, int message_size);
unsigned char* get_domin_result(int dns_sockfd);
#endif //COMP30023_2021_PROJECT_2_MASTER_NETWORK_HANDLER_H
