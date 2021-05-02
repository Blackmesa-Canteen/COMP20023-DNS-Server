//
// Created by xiaotian on 2021/4/30.
//
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>

#include "message_handler.h"

#define TRUE 1
#define FALSE 0

/**
 * read from socket file descriptor, get dns_message struct
 * @param fd socket fd
 * @return dns_message struct
 */
dns_message_t *get_dns_message_ptr(int fd) {
    /**
     * Get message Size
     * */
    // size_head_buffer for first two bytes
    unsigned char *size_head_buffer = (unsigned char *) calloc(2, sizeof(unsigned char));
    if(size_head_buffer == NULL) {
        perror("size_head_buffer");
        exit(EXIT_FAILURE);
    }

    // read two bytes from fd
    int n = read(fd, size_head_buffer, 2);
    if (n < 0) {
        perror("read dns_head_buffer_error");
        exit(EXIT_FAILURE);
    }

    // get message_size from binary size_head_buffer
    int message_size = (size_head_buffer[0] << 8 | size_head_buffer[1]);

    printf("message size(except 2-byte head): %d\n", message_size);

    /**
     * Handle message
     * Except the head two bytes
     *
     * */
    unsigned char *incoming_msg_buffer = (unsigned char *) calloc(message_size, sizeof(unsigned char));
    if(incoming_msg_buffer == NULL) {
        perror("incoming_msg_buffer");
        exit(EXIT_FAILURE);
    }
    n = read(fd, incoming_msg_buffer, message_size);
    if (n < 0) {
        perror("read dns_msg_buffer");
        exit(EXIT_FAILURE);
    }

    /**
     * Reconstruct original query
     */
    unsigned char *original_message = (unsigned char *) calloc(message_size + 2, sizeof(unsigned char));
    if(original_message == NULL) {
        perror("original_message");
        exit(EXIT_FAILURE);
    }
    unsigned char *original_message_ptr = original_message;
    for (int i = 0; i < 2; i++) {
        original_message_ptr[0] = size_head_buffer[i];
        original_message_ptr++;
    }

    for (int i = 0; i < message_size; i++) {
        original_message_ptr[0] = incoming_msg_buffer[i];
        original_message_ptr++;
    }

    dns_message_t *new_dns_message_ptr = (dns_message_t *) calloc(1, sizeof(dns_message_t));
    if(new_dns_message_ptr == NULL) {
        perror("new_dns_message_ptr");
        exit(EXIT_FAILURE);
    }
    new_dns_message_ptr->size_head_buffer = size_head_buffer;
    new_dns_message_ptr->msg_buffer = incoming_msg_buffer;
    new_dns_message_ptr->msg_size = message_size;
    new_dns_message_ptr->original_msg = original_message;

    return new_dns_message_ptr;
}

/**
 * Free dns_message struct
 * @param message_ptr
 */
void free_dns_message_ptr(dns_message_t *message_ptr) {
    free(message_ptr->size_head_buffer);
    message_ptr->size_head_buffer = NULL;
    free(message_ptr->msg_buffer);
    message_ptr->msg_buffer = NULL;
    free(message_ptr->original_msg);
    message_ptr->original_msg = NULL;
    free(message_ptr);
    message_ptr = NULL;
}

/**
 * parse request
 * @param message_t_ptr
 * @param query_type
 * @param domain_name
 */
void parse_dns_request_message_ptr(dns_message_t *message_t_ptr,
                                   int *query_type,
                                   unsigned char **domain_name) {

    /* points to head of the dns message */
    unsigned char *ptr = message_t_ptr->msg_buffer;

    /* skip header ID*/
    ptr += 2;

    /* skip header configurations
     * R code here is important for returning to customer.
     * */
    ptr += 2;

    /* skip QDCOUNT */
    ptr += 2;

    /* skip ANCOUNT */
    ptr += 2;

    /* skip NSCOUNT ARCOUNT */
    ptr += 4;

    /* In Question section */
    /* get one question domin name from a query */
    /** we can assume that there is only one question */
    unsigned char *full_domain_buffer = (unsigned char *) calloc(128, sizeof(unsigned char));
    if(full_domain_buffer == NULL) {
        perror("full_domain_buffer");
        exit(EXIT_FAILURE);
    }
    unsigned char *full_ptr = full_domain_buffer;
    while (TRUE) {
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
            break;
        }
        full_ptr[0] = '.';
        full_ptr++;
    }
    /* Qtype: 28 means AAAA */
    /* get query type*/
    *query_type = ntohs(*((unsigned short *) ptr));
    printf("query type: %d\n", *query_type);
    ptr += 2;

    /* skip Qclass */
    ptr += 2;

    /* get query domin name*/
    *domain_name = full_domain_buffer;

    /**
     * End of question section
     * */
    /**
    * End of retrieve info from the customer's query,
     * ready to do forward;
    * */
}

/**
 * parse response from upper DNS
 * @param message_t_ptr
 * @param answer_num
 * @param ip_text_list
 * @param type_list
 * @param size_list
 */
void parse_dns_response_message_ptr(dns_message_t *message_t_ptr,
                                    int* answer_num,
                                    char ***ip_text_list,
                                    int **type_list,
                                    int **size_list) {

    /* points to head of the dns message */
    unsigned char *ptr = message_t_ptr->msg_buffer;

    /* skip header ID*/
    ptr += 2;

    /* skip header configurations*/
    ptr += 2;

    /* skip QDCOUNT */
    ptr += 2;

    /* read ANCOUNT */
    int answer_account = ntohs(*((unsigned short *) ptr));
    printf("answer account: %d\n", answer_account);
    *answer_num = answer_account;

    // if no answer
    if(answer_account == 0) {
        *ip_text_list = NULL;
        *type_list = NULL;
        *size_list = NULL;
        return;
    }

    /* allocate space for answers list */
    *ip_text_list = (char **) calloc(answer_account, sizeof(char *));
    *type_list = (int *) calloc(answer_account, sizeof(int));
    *size_list = (int *) calloc(answer_account, sizeof(int));
    char **ip_text_list_ptr = *ip_text_list;
    int *type_list_ptr = *type_list;
    int *size_list_ptr = *size_list;

    ptr += 2;

    /* skip NSCOUNT & ARCOUNT */
    ptr += 4;

    /* In Question section */
    /* skip question's QNAME */
    /* Assume that there is only one query */
    while (TRUE) {
        int length = (int) ptr[0];
        ptr += length + 1;
        if (length == 0) {
            break;
        }
    }
    /* skip question's QTYPE & QCLASS */
    ptr += 4;

    /* In Answer section */
    /* there may be many answer */
    for (int i = 0; i < answer_account; i++) {

        /* skip NAME*/
        ptr += 2;

        /* TYPE */
        /* get answer type */
        unsigned short answer_type = ntohs(*((unsigned short *) ptr));
        type_list_ptr[0] = answer_type;
        printf("response type: %d\n", type_list_ptr[0]);
        type_list_ptr++;
        ptr += 2;

        /* skip CLASS */
        ptr += 2;

        /* skip TTL */
        ptr += 4;

        /* RDLENGTH */
        /* get answer length */
        unsigned short answer_length = ntohs(*((unsigned short *) ptr));
        size_list_ptr[0] = answer_length;
        size_list_ptr++;
        ptr += 2;

        /* RDDATA */
        /* get ip */
        if (answer_type == 28) {
            /* only need to show first IPv6 result*/
            unsigned char *ip_v6_address = (unsigned char *) calloc(answer_length, sizeof(unsigned char));
            char *ip_v6_text_address = (char *) calloc(INET6_ADDRSTRLEN, sizeof(char));
            memcpy(ip_v6_address, ptr, answer_length);
//                for(int j = 0; j < 16; j++) {
//                    printf("%x\n", ip_v6_address[j]);
//                }
            // transfer ipv6
            inet_ntop(AF_INET6, ip_v6_address, ip_v6_text_address, INET6_ADDRSTRLEN);
            printf("ipv6 address: %s\n", ip_v6_text_address);

            free(ip_v6_address);

            // put the ip text into ip list
            ip_text_list_ptr[0] = ip_v6_text_address;
            ip_text_list_ptr++;
        } else {
            // if the answer is not AAAA, store it as `#`
            char *other_text_address = (char *) calloc(2, sizeof(char));
            other_text_address[0] = '#';
            other_text_address[1] = '\0';

            ip_text_list_ptr[0] = other_text_address;
            ip_text_list_ptr++;
        }

        /* go to next answer */
        ptr += answer_length;
    }
    /**
     * End of answer section
     * */
}

unsigned char *reconstruct_original_message(dns_message_t *dns_message_ptr) {
    // size_head_buffer for first two bytes
    unsigned char *size_head_buffer = dns_message_ptr->size_head_buffer;

    // get message_size from binary size_head_buffer
    int message_size = (size_head_buffer[0] << 8 | size_head_buffer[1]);

    /**
     * Reconstruct original query
     */
    unsigned char *original_message = (unsigned char *) calloc(message_size + 2, sizeof(unsigned char));
    unsigned char *original_message_ptr = original_message;
    for (int i = 0; i < 2; i++) {
        original_message_ptr[0] = size_head_buffer[i];
        original_message_ptr++;
    }

    for (int i = 0; i < message_size; i++) {
        original_message_ptr[0] = dns_message_ptr->msg_buffer[i];
        original_message_ptr++;
    }

    return original_message;
}

