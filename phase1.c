//#include <netdb.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <fcntl.h>
//
//#include "network_handler.h"
//#include "utils.h"
//#include "message_handler.h"
//#include "my_response_handler.h"
//
//// DNS default port 53
//int main(int argc, char *argv[]) {
//
//    /**
//     * DEBUG file name
//     * */
//    char* request_file_name = "1.comp30023.req.raw";
//    char* response_file_name = "cloudflare.com.res.raw";
//
//    // this server's listening port
//    char *port_number = "8053";
//    struct addrinfo *dns_server_info, *this_server_info;
//    int listen_socket_fd, dns_socket_fd;
//
//    if (argc < 3) {
//        fprintf(stderr, "usage %s hostname port for dns\n", argv[0]);
//        exit(EXIT_FAILURE);
//    }
//
//    /** get address information */
//    printf("DNS：%s, %s\n",argv[1], argv[2] );
//    dns_server_info = get_dns_server_info(argv[1], argv[2]);
//    this_server_info = get_this_server_info(port_number);
//
//
//    /** create socket file descriptors */
//    listen_socket_fd = get_listening_socket_fd(this_server_info);
//    dns_socket_fd = get_dns_connection(dns_server_info);
//
//    /**
//     * Debug: using file to simulate incoming request
//     * */
//    int fd;
//    fd = open(request_file_name, 0);
//    if (fd == -1) {
//        printf("error open file \n");
//        exit(EXIT_FAILURE);
//    }
//
//    /**
//     * Get Query message
//     * */
//    int query_type;
//    /* need to be freed */
//    unsigned char* domain_name;
//    dns_message_t *incoming_query_message = get_dns_message_ptr(fd);
//    /* close fd */
//    close(fd);
//
//    /* parse the request message*/
//    parse_dns_request_message_ptr(incoming_query_message, &query_type, &domain_name);
//
//    /**
//     * judge whether original_query should be send to DNS
//     */
//    if (query_type != 28) {
//        // if first query is not AAAA
//        // we respond with Rcode 4 (“Not Implemented”) by our own
//        // and log “<timestamp> unimplemented request”
//        printf("not AAAA request\n");
//        // find the RCODE part
//        doLog("unimplemented request");
//
//        /**
//         * We need send our own response now, after send the response, continue the great loop
//         * */
//        unsigned char* unimplemented_response = generate_not_implemented_response(incoming_query_message);
//        // do sending
//        // then:
//        free(unimplemented_response);
//
//    } else {
//        // forward to DNS server, then get query result
//        // do log
//        char log_str[256] = "requested ";
//        strcat(log_str, (char *) domain_name);
//        doLog(log_str);
//
//        // do forwarding:
//    }
//
//    /**
//   *
//   * Debug: using file to simulate response from dns
//   *
//   * */
//    dns_socket_fd = open(response_file_name, 0);
//    if (dns_socket_fd == -1) {
//        printf("error open file \n");
//        exit(EXIT_FAILURE);
//    }
//
//    /**
//     * Use real response from DNS server
//     * example: 8.8.8.8 53
//     *
//     */
//    // send message to real DNS server
////    int n = write(dns_socket_fd, incoming_query_message->original_msg, incoming_query_message->msg_size + 2);
////    if(n < 0) {
////        perror("send to DNS server socket error");
////        exit(EXIT_FAILURE);
////    }
//
//    /**
//     * Get DNS server's response
//     */
//    // from file
//    dns_message_t *dns_response_message = get_dns_message_ptr(dns_socket_fd);
//    close(dns_socket_fd);
//    // from real server
////    dns_message_t *dns_response_message = get_dns_message_ptr(dns_socket_fd);
////    close(dns_socket_fd);
//
//    /* get answer info list */
//    char* *ip_text_list = NULL;
//    int* type_list = NULL;
//    int* size_list = NULL;
//    int answer_num = 0;
//
//    parse_dns_response_message_ptr(dns_response_message, &answer_num, &ip_text_list, &type_list, &size_list);
//
//    /**
//     * judge the first answer type
//     * */
//    if(type_list != NULL && type_list[0] == 28) {
//        /**if the field is ipv6(AAAA) */
//        char log_str[256] = "";
//        strcat(log_str, (char *) domain_name);
//        strcat(log_str, " is at ");
//        strcat(log_str, ip_text_list[0]);
//        doLog(log_str);
//
//        // do forwarding the response to the client
//
//    } else if (type_list != NULL && type_list[0] != 28) {
//        /** If the ﬁrst answer in the response is not a AAAA ﬁeld,  then do not print a log entry (for any answer in the response) */
//
//        /**
//     * Debug: print unimplemented response
//     */
//        unsigned char* unimplemented_response = generate_not_implemented_response(incoming_query_message);
//        // do forwarding the response to the client
//        printf("-----------unimplemented response----------\n");
//        for(int i = 0; i < 42; i++) {
//            printf("%x\n", unimplemented_response[i]);
//        }
//        free(unimplemented_response);
//    } else {
//        /** If no answer at all, just forward the response, no log */
//        // do forwarding the response to the client
//    }
//
//    /**
//     * Debug: print all text ip
//     */
//    for(int i = 0; i < answer_num; i++) {
//        printf("answer %d: %s\n", i, ip_text_list[i]);
//    }
//
//    /**
//     * Free things for one query session
//     */
//    free_dns_message_ptr(incoming_query_message);
//    free_dns_message_ptr(dns_response_message);
//    free(domain_name);
//    for(int i = 0; i < answer_num; i++) {
//        free(ip_text_list[i]);
//        ip_text_list[i] = NULL;
//
//    }
//    free(ip_text_list);
//    ip_text_list = NULL;
//    free(type_list);
//    type_list = NULL;
//    free(size_list);
//    size_list = NULL;
//
//
//    // at the end of the program
//    freeaddrinfo(this_server_info);
//    freeaddrinfo(dns_server_info);
//
//    return 0;
//}
//
//
//
//
//
//
//
