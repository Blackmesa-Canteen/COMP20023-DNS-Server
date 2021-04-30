#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "network_handler.h"
#include "utils.h"
#include "message_handler.h"

int main(int argc, char *argv[]) {

    /**
     * DEBUG file name
     * */
    char* request_file_name = "1.comp30023.a.req.raw";
    char* response_file_name = "none.comp30023.res.raw";

    char *port_number = "8053";
    struct addrinfo *dns_server_info;
    int listen_socket_fd, dns_socket_fd;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port for dns\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /** get dns connection */
    dns_server_info = get_dns_server_info(argv[1], argv[2]);
    dns_socket_fd = get_dns_connection(dns_server_info);

    /** setup localhost listening */
    listen_socket_fd = get_listening_socket_fd(port_number);

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
    fd = open(response_file_name, 0);
    if (fd == -1) {
        printf("error open file \n");
        exit(EXIT_FAILURE);
    }

    /**
     * Get DNS server's response
     */
    dns_message_t *dns_response_message = get_dns_message_ptr(fd);
    close(fd);

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
        free(ip_text_list);
        ip_text_list = NULL;
    }
    free(type_list);
    free(size_list);

    return 0;
}







