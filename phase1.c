#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "network_handler.h"
#include "utils.h"
#include "message_handler.h"

int main(int argc, char *argv[]) {
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
     *
     * Debug: using file to simulate incoming request
     *
     * */
    int fd;
    fd = open("1.comp30023.req.raw", 0);
    if (fd == -1) {
        printf("error open file \n");
        exit(EXIT_FAILURE);
    }

    /**
     * Get Query Size
     * */
    dns_message_t* incoming_query_message = get_dns_message_ptr(fd);
    // size_head_buffer for first two bytes
    unsigned char size_head_buffer[3];

    // read two bytes from fd
    read(fd, size_head_buffer, 2);

    // get message_size from binary size_head_buffer
    int message_size = (size_head_buffer[0] << 8 | size_head_buffer[1]);

    printf("size: %d\n", message_size);

    /**
     * Handle query message
     * Except the head two bytes
     *
     * */
//    unsigned char incoming_msg_buffer[message_size + 1];
    unsigned char incoming_msg_buffer[message_size + 1];
    // point ptr to the head of incoming_msg_buffer
    unsigned char *ptr = incoming_msg_buffer;
//    int n = read(fd, incoming_msg_buffer, message_size);
    int n = read(fd, incoming_msg_buffer, message_size);
    close(fd); /* close fd */
    int question_account, answer_account, query_type;
    if (n < 0) {
        perror("read incoming_msg_buffer");
        exit(EXIT_FAILURE);
    }

    /* skip header ID*/
    ptr += 2;
    /* skip header configurations
     * R code here is important for returning to customer.
     * */
    ptr += 2;
    /* get QDCOUNT */
    question_account = ntohs(*((unsigned short *) ptr));
    printf("question account: %d\n", question_account);
    ptr += 2;
    /* get ANCOUNT */
    answer_account = ntohs(*((unsigned short *) ptr));
    ptr += 2;
    /* skip NSCOUNT ARCOUNT */
    ptr += 4;

    /* In Question section */
    /* get all questions from a query */
    /** we can assume that there is only one question */
    unsigned char **full_domain_buffer_list = (unsigned char **) calloc(question_account, sizeof(unsigned char *));
    unsigned char **full_domain_buffer_list_ptr = full_domain_buffer_list;

    /* get all questions query type*/
    int* domain_query_type_list = (int*) calloc(question_account, sizeof(int));
    int* domain_query_type_list_ptr = domain_query_type_list;

    for (int i = 0; i < question_account; i++) {

        unsigned char *full_domain_buffer = (unsigned char *) calloc(128, sizeof(unsigned char));
        unsigned char *full_ptr = full_domain_buffer;
        while (1) {
            int part_len = (int) ptr[0];
            ptr++;
            for (int j = 0; j < part_len; j++) {
                full_ptr[0] = ptr[0];
                ptr++;
                full_ptr++;
            }
            if (part_len == 0) {
                // end for one question
                // change the last `.` into `/0`
                full_ptr--;
                full_ptr[0] = '\0';
                printf("%s\n", full_domain_buffer);

                full_domain_buffer_list_ptr[0] = full_domain_buffer;
                full_domain_buffer_list_ptr++;
                break;
            }
            full_ptr[0] = '.';
            full_ptr++;
        }
        /* Qtype: 28 means AAAA */
        /* Store query type into a list*/
        query_type = ntohs(*((unsigned short *) ptr));
        printf("query type: %d\n", query_type);
        domain_query_type_list_ptr[0] = query_type;
        domain_query_type_list_ptr++;
        ptr += 2;
        /* skip Qclass */
        ptr += 2;
    }
    /**
     * End of question section
     * */
    /**
    * End of retrieve info from the customer's query,
     * ready to do forward;
    * */

    /**
     * Reconstruct original query
     */
    unsigned char* original_query = (unsigned char*) calloc(message_size+2, sizeof (unsigned char));
     unsigned char* original_query_ptr = original_query;
     for(int i = 0; i < 2; i++) {
         original_query_ptr[0] = size_head_buffer[i];
         original_query_ptr++;
     }

     for(int i = 0; i < message_size; i++) {
         original_query_ptr[0] = incoming_msg_buffer[i];
         original_query_ptr++;
     }

//     for(int i = 0; i < message_size + 2; i++) {
//         printf("original query: %x\n", original_query[i]);
//     }

     /**
      * judge whether original_query should be send to DNS
      */
      if(domain_query_type_list[0] != 28) {
          // if first query is not AAAA
          // we respond with Rcode 4 (“Not Implemented”) by our own
          // and log “<timestamp> unimplemented request”
          printf("not AAAA request\n");
          // find the RCODE part
          doLog("unimplemented request");


      } else {
          // forward to DNS server, then get query result
          char log_str[256] = "requested ";
          strcat(log_str, (char*)full_domain_buffer_list[0]);
          doLog(log_str);
      }


    /**
   *
   * Debug: using file to simulate response from dns
   *
   * */
    fd = open("1.comp30023.res.raw", 0);
    if (fd == -1) {
        printf("error open file \n");
        exit(EXIT_FAILURE);
    }

    return 0;
}







