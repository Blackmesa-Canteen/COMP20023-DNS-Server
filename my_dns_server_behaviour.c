//
// Created by Shaotien Lee on 2021/5/4.
//

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "network_handler.h"
#include "utils.h"
#include "message_handler.h"
#include "my_response_handler.h"
#include "my_dns_server_behaviour.h"

#define TRUE 1
#define FALSE 0

void
my_dns_server_run(struct addrinfo *dns_server_info, int listen_socket_fd, fd_set *active_socket_fd_set, int maxfd) {

    /* The great Loop */
    for (;;) {

        /* --- Codes below are based on Practical 10's select-server-1.2.c ---- */
        /* Non-blocking can be done using SELECT */
        /* Monitor exceptions */
        fd_set socket_fds = (*active_socket_fd_set);
        if (select(FD_SETSIZE, &socket_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        /* loop all possible socket fd */
        for (int i = 0; i <= maxfd; ++i){
            /* check active */
            if (FD_ISSET(i, &socket_fds)) {
                /* create new socket if there is new incoming connection request */
                if (i == listen_socket_fd) {
                    struct sockaddr_in client_address_info;
                    socklen_t clilen = sizeof(client_address_info);
                    int newsockfd =
                            accept(listen_socket_fd, (struct sockaddr*)&client_address_info, &clilen);
                    if (newsockfd < 0)
                        perror("accept the listening fd");
                    else {
                        /* add the valid new coming socket to the set */
                        FD_SET(newsockfd, active_socket_fd_set);
                        /* update the maximum tracker */
                        if (newsockfd > maxfd)
                            maxfd = newsockfd;
                        /* DEBUG: print out the IP and the socket number */
                        char ip[INET_ADDRSTRLEN];
                        printf("new connection from %s on socket %d\n",
                               inet_ntop(client_address_info.sin_family, &client_address_info.sin_addr,
                                         ip, INET_ADDRSTRLEN),
                               newsockfd);
                    }
                }
                    /* --- Codes above are based on Practical 10's select-server-1.2.c --- */

                else {
                    /* dns request message is sent from the client */
                    /* Get Query message */
                    int query_type;
                    /* need to be freed */
                    unsigned char *domain_name;

                    /* i means one new_socket_fd */
                    dns_message_t *incoming_query_message = get_dns_message_ptr(i);

                    /* parse the request message*/
                    parse_dns_request_message_ptr(incoming_query_message, &query_type, &domain_name);

                    // do log
                    char log_str[256] = "requested ";
                    strcat(log_str, (char *) domain_name);
                    doLog(log_str);

                    /*
                     * if original_query is not AAAA, do not send to DNS
                     */
                    if (query_type != 28) {

                        Handle_non_AAAA_request(i, incoming_query_message, active_socket_fd_set);
                        free(domain_name);
                        continue;
                    }

                    /** if request is AAAA */
                    Handle_AAAA_request(dns_server_info,
                                        i,
                                        domain_name,
                                        incoming_query_message,
                                        active_socket_fd_set);
                }
            }
        }
    }
}

void
Handle_AAAA_request(struct addrinfo *dns_server_info, int new_socket_fd, unsigned char *domain_name,
                    dns_message_t *incoming_query_message, fd_set *active_socket_fd_set) {

    /* create dns socket */
    int dns_socket_fd = get_dns_connection(dns_server_info);
    int n;
    // do forwarding:
    /*
     * Use real response from DNS server
     * example: 8.8.8.8 53
     */
    // send message to real DNS server
    n = write(dns_socket_fd, incoming_query_message->original_msg, incoming_query_message->msg_size + 2);
    if (n < 0) {
        perror("send to DNS server socket error");
        exit(EXIT_FAILURE);
    }

    /*
     * Get DNS server's response
     */
    dns_message_t *dns_response_message = get_dns_message_ptr(dns_socket_fd);

    /* get answer info list */
    char **ip_text_list = NULL;
    int *type_list = NULL;
    int *size_list = NULL;
    int answer_num = 0;

    parse_dns_response_message_ptr(dns_response_message, &answer_num, &ip_text_list, &type_list, &size_list);

    /*
    * judge the first answer type
    * */
    if (type_list != NULL && type_list[0] == 28) {
        /**if the field is ipv6(AAAA) */
        char log_string[256] = "";
        strcat(log_string, (char *) domain_name);
        strcat(log_string, " is at ");
        strcat(log_string, ip_text_list[0]);
        doLog(log_string);

        // do forwarding the response to the client
        printf("write back upper DNS answer to client\n");
        n = write(new_socket_fd, dns_response_message->original_msg, dns_response_message->msg_size + 2);
        if (n <= 0) {
            if(n < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            } else {
                printf("client socket %d closed\n", new_socket_fd);
            }
            close(new_socket_fd);
            FD_CLR(new_socket_fd, active_socket_fd_set);
        }

    } else if (type_list != NULL && type_list[0] != 28) {
        /* If the ﬁrst answer in the response is not a AAAA ﬁeld,
         * then do not print a log entry (for any answer in the response) */
        printf("write back not AAAA-first DNS answer to client without log \n");
        n = write(new_socket_fd, dns_response_message->original_msg, dns_response_message->msg_size + 2);
        if (n <= 0) {
            if(n < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            } else {
                printf("client socket %d closed\n", new_socket_fd);
            }
            close(new_socket_fd);
            FD_CLR(new_socket_fd, active_socket_fd_set);
        }

    } else {
        /* If no answer at all, just forward the response, no log */
        // do forwarding the response to the client
        printf("write back empty-answer upper DNS answer to client\n");
        n = write(new_socket_fd, dns_response_message->original_msg, dns_response_message->msg_size + 2);
        if (n <= 0) {
            if(n < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            } else {
                printf("client socket %d closed\n", new_socket_fd);
            }
            close(new_socket_fd);
            FD_CLR(new_socket_fd, active_socket_fd_set);
        }
    }

    /* close client and dns fd */
    close(new_socket_fd);
    FD_CLR(new_socket_fd, active_socket_fd_set);
    close(dns_socket_fd);

    /*
     * Free things for one query session
     */
    free_dns_message_ptr(incoming_query_message);
    free_dns_message_ptr(dns_response_message);
    free(domain_name);
    for (int ii = 0; ii < answer_num; ii++) {
        free(ip_text_list[ii]);
        ip_text_list[ii] = NULL;

    }
    free(ip_text_list);
    ip_text_list = NULL;
    free(type_list);
    type_list = NULL;
    free(size_list);
    size_list = NULL;
}

void
Handle_non_AAAA_request(int new_socket_fd, dns_message_t *incoming_query_message, fd_set *active_socket_fd_set) {
    // if first query is not AAAA
    // we respond with Rcode 4 (“Not Implemented”) by our own, and log “<timestamp> unimplemented request”
    printf("not AAAA request\n");
    // find the RCODE part
    doLog("unimplemented request");

    /* We need send our own response now, after send the response*/
    int message_size = 0;
    unsigned char *unimplemented_response = generate_not_implemented_response(incoming_query_message,
                                                                              &message_size);
    // do sending
    printf("write back unimplemented response \n");
    int n = write(new_socket_fd, unimplemented_response, (message_size + 2));
    if (n <= 0) {
        if(n < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        } else {
            printf("client socket %d closed\n", new_socket_fd);
        }
        close(new_socket_fd);
        FD_CLR(new_socket_fd, active_socket_fd_set);
    }
    free(unimplemented_response);

    /* reset connections and continue */
    close(new_socket_fd);
    FD_CLR(new_socket_fd, active_socket_fd_set);
    free_dns_message_ptr(incoming_query_message);
}



