//
// Created xiaotian on 2021/4/30.
//

#ifndef COMP30023_2021_PROJECT_2_MASTER_MESSAGE_HANDLER_H
#define COMP30023_2021_PROJECT_2_MASTER_MESSAGE_HANDLER_H
typedef struct dns_message {
    unsigned char* size_head_buffer;
    unsigned char* msg_buffer;
} dns_message_t;

typedef dns_message_t* dns_message_t_ptr;

dns_message_t* get_dns_message_ptr(int fd);
void free_dns_message_ptr(dns_message_t* message_ptr);
unsigned char* reconstruct_original_message(dns_message_t* dns_message_ptr);


#endif //COMP30023_2021_PROJECT_2_MASTER_MESSAGE_HANDLER_H
