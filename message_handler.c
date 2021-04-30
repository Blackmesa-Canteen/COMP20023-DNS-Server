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

#include "message_handler.h"

dns_message_t* get_dns_message_ptr(int fd) {
    /**
     * Get message Size
     * */
    // size_head_buffer for first two bytes
    unsigned char* size_head_buffer = (unsigned char*) calloc(2, sizeof (unsigned char));

    // read two bytes from fd
    read(fd, size_head_buffer, 2);

    // get message_size from binary size_head_buffer
    int message_size = (size_head_buffer[0] << 8 | size_head_buffer[1]);

    printf("message size(except 2-byte head): %d\n", message_size);

    /**
     * Handle message
     * Except the head two bytes
     *
     * */
    unsigned char* incoming_msg_buffer = (unsigned char*) calloc(message_size, sizeof (unsigned char));
    int n = read(fd, incoming_msg_buffer, message_size);
    if (n < 0) {
        perror("read incoming_msg_buffer");
        exit(EXIT_FAILURE);
    }

    dns_message_t* new_dns_message_ptr = (dns_message_t*) calloc(1, sizeof(dns_message_t));
    new_dns_message_ptr->size_head_buffer = size_head_buffer;
    new_dns_message_ptr->msg_buffer = incoming_msg_buffer;

    return new_dns_message_ptr;
}

void free_dns_message_ptr(dns_message_t* message_ptr) {
    free(message_ptr->size_head_buffer);
    message_ptr->size_head_buffer = NULL;
    free(message_ptr->msg_buffer);
    message_ptr->msg_buffer = NULL;
    free(message_ptr);
    message_ptr = NULL;
}

unsigned char* reconstruct_original_message(dns_message_t* dns_message_ptr) {
    // size_head_buffer for first two bytes
    unsigned char* size_head_buffer = dns_message_ptr->size_head_buffer;

    // get message_size from binary size_head_buffer
    int message_size = (size_head_buffer[0] << 8 | size_head_buffer[1]);

    /**
     * Reconstruct original query
     */
    unsigned char* original_message = (unsigned char*) calloc(message_size + 2, sizeof (unsigned char));
    unsigned char* original_message_ptr = original_message;
    for(int i = 0; i < 2; i++) {
        original_message_ptr[0] = size_head_buffer[i];
        original_message_ptr++;
    }

    for(int i = 0; i < message_size; i++) {
        original_message_ptr[0] = dns_message_ptr->msg_buffer[i];
        original_message_ptr++;
    }

    return original_message;
}

