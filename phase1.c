#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    char *port_number = "8053";
    struct addrinfo listen_hints, dns_hints, *this_server_info, *dns_server_info;
    int re, listen_socket_fd, dns_socket_fd;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port for dns\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // create address for upper dns
    memset(&dns_hints, 0, sizeof dns_hints);
    // ip v4
    dns_hints.ai_family = AF_INET;
    // TCP
    dns_hints.ai_socktype = SOCK_STREAM;

    // set address info for dns
    if ((getaddrinfo(argv[1], argv[2], &dns_hints, &dns_server_info)) < 0) {
        perror("getaddrinfo for upper dns");
        exit(EXIT_FAILURE);
    }

    // create address for this server to listen to
    memset(&listen_hints, 0, sizeof listen_hints);
    // ip v4
    listen_hints.ai_family = AF_INET;
    // TCP
    listen_hints.ai_socktype = SOCK_STREAM;
    // listen accept
    listen_hints.ai_flags = AI_PASSIVE;

    // set address info for listening
    if (getaddrinfo(NULL, port_number, &listen_hints, &this_server_info) < 0) {
        perror("getaddrinfo for this server");
        exit(EXIT_FAILURE);
    }

    // create upper dns socket

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

    // buffer for first two bytes
    unsigned char buffer[3];

    // read two bytes from fd
    read(fd, buffer, 2);

    // get message_size from binary buffer
    int message_size = (buffer[0] << 8 | buffer[1]);

    printf("size: %d\n", message_size);

    /**
     * Handle query message
     * Except the head two bytes
     *
     * */
//    unsigned char incoming_msg_buffer[message_size + 1];
    unsigned char incoming_msg_buffer[53];
    // point ptr to the head of incoming_msg_buffer
    unsigned char* ptr = incoming_msg_buffer;
//    int n = read(fd, incoming_msg_buffer, message_size);
    int n = read(fd, incoming_msg_buffer, 52);
    close(fd); /* close fd */
    int test = buffer[1];
    int question_account, answer_account, query_type;
    if (n < 0) {
        perror("read incoming_msg_buffer");
        exit(EXIT_FAILURE);
    }

    /* skip header ID*/
    ptr += 2;
    /* skip header configurations*/
    ptr += 2;
    /* get QDCOUNT */
    question_account = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    /* get ANCOUNT */
    answer_account = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    /* skip NSCOUNT ARCOUNT */
    ptr += 4;

    /* In Question section */
    /* get all questions from a query */
    unsigned char** full_domain_buffer_list = (unsigned char**) calloc(question_account, sizeof (unsigned char*));
    unsigned char** full_domain_buffer_list_ptr = full_domain_buffer_list;

    for(int i = 0; i < question_account; i++) {

        unsigned char*  full_domain_buffer = (unsigned char*) calloc(128, sizeof (unsigned char));
        unsigned char* full_ptr = full_domain_buffer;
        while (1) {
            int part_len = (int) ptr[0];
            ptr++;
            for(int j = 0; j < part_len; j++) {
                full_ptr[0] = ptr[0];
                ptr++;
                full_ptr++;
            }
            if(part_len == 0) {
                // end for one question
                // change the last `.` into `/0`
                full_ptr--;
                full_ptr[0] = '\0';
                printf("%s", full_domain_buffer);

                full_domain_buffer_list_ptr[0] = full_domain_buffer;
                full_domain_buffer_list_ptr++;
                break;
            }
            full_ptr[0] = '.';
            full_ptr++;
        }
    }
    /**
     * End of retrieve domain string from query message
     * Then get Q Type
     * */
     /* Q type: 28 means AAAA */
     query_type = ntohs(*((unsigned short*)ptr));
     ptr += 2;

    /**
    * End of retrieve info from the customer's query,
     * ready to do forward;
    * */


    return 0;
}


