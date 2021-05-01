#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "network_handler.h"
#include "utils.h"
#include "message_handler.h"

// DNS default port 53
int main(int argc, char *argv[]) {

    /**
     * DEBUG file name
     * */
    char* request_file_name = "cloudflare.com.req.raw";
    char* response_file_name = "cloudflare.com.res.raw";

    char *port_number = "8053";
    struct addrinfo *dns_server_info, *this_server_info;
    int listen_socket_fd, dns_socket_fd;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port for dns\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /** get dns connection */
    printf("DNS：%s, %s\n",argv[1], argv[2] );
    dns_server_info = get_dns_server_info(argv[1], argv[2]);
    dns_socket_fd = get_dns_connection(dns_server_info);

    /** setup localhost listening */
    this_server_info = get_this_server_info(port_number);
    listen_socket_fd = get_listening_socket_fd(this_server_info);

    /**
     * Debug: using file to simulate incoming request
     * */
    int fd;
    fd = open(request_file_name, 0);
    if (fd == -1) {
        printf("error open file \n");
        exit(EXIT_FAILURE);
    }

    /**
     * Get Query message
     * */
    int query_type;
    /* need to be freed */
    unsigned char* domain_name;
    dns_message_t *incoming_query_message = get_dns_message_ptr(fd);
    freeaddrinfo(this_server_info);
    /* close fd */
    close(fd);

    /* parse the request message*/
    parse_dns_request_message_ptr(incoming_query_message, &query_type, &domain_name);

    /**
     * judge whether original_query should be send to DNS
     */
    if (query_type != 28) {
        // if first query is not AAAA
        // we respond with Rcode 4 (“Not Implemented”) by our own
        // and log “<timestamp> unimplemented request”
        printf("not AAAA request\n");
        // find the RCODE part
        doLog("unimplemented request");


    } else {
        // forward to DNS server, then get query result
        char log_str[256] = "requested ";
        strcat(log_str, (char *) domain_name);
        doLog(log_str);
    }

    /**
   *
   * Debug: using file to simulate response from dns
   *
   * */
//    fd = open(response_file_name, 0);
//    if (fd == -1) {
//        printf("error open file \n");
//        exit(EXIT_FAILURE);
//    }

    /**
     * Use real response from 8.8.8.8 DNS
     */
    // send message to real DNS server
    int n = write(dns_socket_fd, incoming_query_message->original_msg, incoming_query_message->msg_size + 2);
    if(n < 0) {
        perror("send to DNS server socket error");
        exit(EXIT_FAILURE);
    }

    /**
     * Get DNS server's response
     */
     // from file
//    dns_message_t *dns_response_message = get_dns_message_ptr(fd);
//    close(fd);
    // from real server
    dns_message_t *dns_response_message = get_dns_message_ptr(dns_socket_fd);
    freeaddrinfo(dns_server_info);
    close(dns_socket_fd);

    /* get answer info list */
    char* *ip_text_list = NULL;
    int* type_list = NULL;
    int* size_list = NULL;
    int answer_num = 0;

    parse_dns_response_message_ptr(dns_response_message, &answer_num, &ip_text_list, &type_list, &size_list);

    /**
     * judge the first answer type whether is AAAA field
     * */
    if(type_list != NULL && type_list[0] == 28) {
        //if the field is ipv6(AAAA)
        char log_str[256] = "";
        strcat(log_str, (char *) domain_name);
        strcat(log_str, " is at ");
        strcat(log_str, ip_text_list[0]);
        doLog(log_str);
    } else {
        /**
         * If the ﬁrst answer in the response is not a AAAA ﬁeld,
         * then do not print a log entry (for any answer in the response)
         * */
    }

    /**
     * Debug: print all text ip
     */
    for(int i = 0; i < answer_num; i++) {
        printf("answer %d: %s\n", i, ip_text_list[i]);
    }

    free_dns_message_ptr(incoming_query_message);
    free_dns_message_ptr(dns_response_message);
    free(domain_name);
    for(int i = 0; i < answer_num; i++) {
        free(ip_text_list[i]);
        ip_text_list[i] = NULL;

    }
    free(ip_text_list);
    ip_text_list = NULL;
    free(type_list);
    type_list = NULL;
    free(size_list);
    size_list = NULL;


    return 0;
}







