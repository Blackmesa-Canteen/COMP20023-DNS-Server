//
// Created xiaotian on 2021/4/30.
//

#ifndef COMP30023_2021_PROJECT_2_MASTER_MESSAGE_HANDLER_H
#define COMP30023_2021_PROJECT_2_MASTER_MESSAGE_HANDLER_H
typedef struct dns_message {
    unsigned char* size_head_buffer;
    unsigned char* msg_buffer;
    int msg_size;
    unsigned char* original_msg;

} dns_message_t;

typedef dns_message_t* dns_message_t_ptr;

dns_message_t* get_dns_message_ptr(int fd);
void free_dns_message_ptr(dns_message_t* message_ptr);
unsigned char* reconstruct_original_message(dns_message_t* dns_message_ptr);
void parse_dns_request_message_ptr(dns_message_t* message_t_ptr,
                                   int* query_type,
                                   unsigned char **domain_name);

void parse_dns_response_message_ptr(dns_message_t* message_t_ptr,
                                    int* answer_num,
                                   char* **ip_text_list,
                                   int* *type_list,
                                   int* *size_list);


#endif //COMP30023_2021_PROJECT_2_MASTER_MESSAGE_HANDLER_H
