#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <arpa/inet.h>

#include "network_handler.h"
#include "utils.h"
#include "message_handler.h"
#include "my_response_handler.h"

#define TRUE 1
#define FALSE 0

// DNS default port 53
int main(int argc, char *argv[]) {

    // this server's listening port
    char *port_number = "8053";
    struct addrinfo *dns_server_info, *this_server_info;
    int listen_socket_fd, dns_socket_fd;

    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    int new_socket_fd = -1;
    int n;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port for dns\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /** get address information */
    printf("DNS：%s, %s\n",argv[1], argv[2] );
    dns_server_info = get_dns_server_info(argv[1], argv[2]);
    this_server_info = get_this_server_info(port_number);


    /** create socket file descriptors */
    /* reused listening socket */
    listen_socket_fd = get_listening_socket_fd(this_server_info);

    for(;;) {
        /** Use real request from client */
        // Get back a new file descriptor to communicate on
        client_addr_size = sizeof client_addr;

        // judge the socket status
        // if the socket is closed by client, we can create new socket, else, we use the old socket
        if(IsSocketClosed(new_socket_fd)) {
            new_socket_fd = accept(listen_socket_fd, (struct sockaddr*)&client_addr, &client_addr_size);
            if(new_socket_fd < 0) {
                perror("accept from client");
                exit(EXIT_FAILURE);
            }
        }

        /** Get Query message */
        int query_type;
        /* need to be freed */
        unsigned char* domain_name;

        dns_message_t *incoming_query_message = get_dns_message_ptr(new_socket_fd);

        /* parse the request message*/
        parse_dns_request_message_ptr(incoming_query_message, &query_type, &domain_name);

        /**
         * if original_query is not AAAA, do not send to DNS
         */
        if (query_type != 28) {
            // if first query is not AAAA
            // we respond with Rcode 4 (“Not Implemented”) by our own
            // and log “<timestamp> unimplemented request”
            printf("not AAAA request\n");
            // find the RCODE part
            doLog("unimplemented request");

            /**
             * We need send our own response now, after send the response, continue the great loop
             * */
            int message_size = 0;
            unsigned char* unimplemented_response = generate_not_implemented_response(incoming_query_message, &message_size);

            // do sending
            printf("write back unimplemented response \n");
            n = write(new_socket_fd, unimplemented_response, (message_size + 2));
            if (n < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            free(unimplemented_response);

            /* reset connections and continue */

            // we shouldn't unilaterally close customer's connection？
            // check whether the client closed the connection
            if(IsSocketClosed(new_socket_fd)) {
                close(new_socket_fd);
                new_socket_fd = -1;
            }
            continue;
        }

        /** if request is AAAA */
        // forward to DNS server, then get query result
        /* create dns socket */
        dns_socket_fd = get_dns_connection(dns_server_info);

        // do log
        char log_str[256] = "requested ";
        strcat(log_str, (char *) domain_name);
        doLog(log_str);

        // do forwarding:
        /**
         * Use real response from DNS server
         * example: 8.8.8.8 53
         *
         */
        // send message to real DNS server
        n = write(dns_socket_fd, incoming_query_message->original_msg, incoming_query_message->msg_size + 2);
        if(n < 0) {
            perror("send to DNS server socket error");
            exit(EXIT_FAILURE);
        }

        /**
         * Get DNS server's response
         */
        dns_message_t *dns_response_message = get_dns_message_ptr(dns_socket_fd);

        /* get answer info list */
        char* *ip_text_list = NULL;
        int* type_list = NULL;
        int* size_list = NULL;
        int answer_num = 0;

        parse_dns_response_message_ptr(dns_response_message, &answer_num, &ip_text_list, &type_list, &size_list);

        /**
         * judge the first answer type
         * */
        if(type_list != NULL && type_list[0] == 28) {
            /**if the field is ipv6(AAAA) */
            char log_string[256] = "";
            strcat(log_string, (char *) domain_name);
            strcat(log_string, " is at ");
            strcat(log_string, ip_text_list[0]);
            doLog(log_string);

            // do forwarding the response to the client
            printf("write back upper DNS answer to client\n");
            n = write(new_socket_fd, dns_response_message->original_msg, dns_response_message->msg_size + 2);
            if (n < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }

        } else if (type_list != NULL && type_list[0] != 28) {
            /** If the ﬁrst answer in the response is not a AAAA ﬁeld,  then do not print a log entry (for any answer in the response) */
            int response_size = 0;
            unsigned char* unimplemented_response = generate_not_implemented_response(incoming_query_message, &response_size);
            // do forwarding the response to the client
            printf("non-AAAA answer, write back unimplemented to client\n");
            n = write(new_socket_fd, unimplemented_response, response_size + 2);
            if (n < 0) {
                perror("write non-AAAA unimplemented answer");
                exit(EXIT_FAILURE);
            }
            free(unimplemented_response);
        } else {
            /** If no answer at all, just forward the response, no log */
            // do forwarding the response to the client
            // do forwarding the response to the client
            printf("write back empty-answer upper DNS answer to client\n");
            n = write(new_socket_fd, dns_response_message->original_msg, dns_response_message->msg_size + 2);
            if (n < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }

        /* close client and dns fd */
        if(IsSocketClosed(new_socket_fd)) {
            close(new_socket_fd);
            new_socket_fd = -1;
        }
        close(dns_socket_fd);

        /**
         * Free things for one query session
         */
        free_dns_message_ptr(incoming_query_message);
        free_dns_message_ptr(dns_response_message);
        free(domain_name);
        for(int ii = 0; ii < answer_num; ii++) {
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

    // at the end of the program
    freeaddrinfo(this_server_info);
    freeaddrinfo(dns_server_info);
    close(listen_socket_fd);

    return 0;
}
