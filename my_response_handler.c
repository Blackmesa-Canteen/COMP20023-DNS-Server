//
// Created by Shaotien Lee on 2021/5/1.
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
#include "my_response_handler.h"

#define TRUE 1
#define FALSE 0

/**
 * Authority & Additional sections
 * based on example response raw files, size: 11 bytes
 */
unsigned char MESSAGE_TAIL[11] = {0x00, 0x00, 0x29, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int TAIL_SIZE = 11;

/** Not Implemented Response header configure: RCODE = 4; 1000 0001 1000 0100 => 8184 */
unsigned char NOT_IMPLEMENTED_RESPONSE_HEADER_CONFIGURE[2] = {0x81, 0x84};

/** Good Response header configure: 1000 0001 1000 0000 => 8180 */
unsigned char GOOD_RESPONSE_HEADER_CONFIGURE[2] = {0x81, 0x80};

/** DNS message's header size is 12 bytes*/
int HEADER_SIZE = 12;

unsigned char* generate_not_implemented_response(dns_message_t *request_message_t_ptr) {
    unsigned char* ptr = NULL;

    /**
     * Reconstruct a new `Header-Question` Sections,
     * based on incoming query information
     * */
    // clone header
    unsigned char* message_header = (unsigned char*) calloc(HEADER_SIZE, sizeof (unsigned char));
    if(message_header == NULL) {
        perror("allocate message_header in generate_not_implemented_response");
        exit(EXIT_FAILURE);
    }
    memcpy(message_header, request_message_t_ptr->msg_buffer, HEADER_SIZE);

    //modify QR & RCODE
    ptr = message_header;
    ptr += 2; // skip ID

    // ptr points to header configure part
    // modify the header configure part
    ptr[0] = NOT_IMPLEMENTED_RESPONSE_HEADER_CONFIGURE[0];
    ptr[1] = NOT_IMPLEMENTED_RESPONSE_HEADER_CONFIGURE[1];

    /* finished modify the header*/

    /* parse and reconstruct message question part */
    /* assume one question */

    // points two message start
   ptr = request_message_t_ptr->msg_buffer;

   // move pointer to the question part
   ptr += 12;

   // calc question size to allocate memory to hold the question
   int total_text_bytes = 0;
   int part_text_bytes = 0;
   while(TRUE) {
       part_text_bytes = (int) ptr[0];
       // +1 byte means add size-counter itself
       total_text_bytes += (part_text_bytes + 1);
        ptr += (part_text_bytes + 1);
        if(part_text_bytes == 0) break;
   }

   // one `00` is not a size-counter
    total_text_bytes -= 1;

   // Get total question bytes, 5 bytes includes: `end the QNAME`, `QTYPE`, `QCLASS`
   int total_question_bytes = total_text_bytes + 5;

   // set up question part for response
    unsigned char* message_question = (unsigned char*) calloc(total_question_bytes, sizeof (unsigned char));
    if(message_question == NULL) {
        perror("allocate message_question in generate_not_implemented_response");
        exit(EXIT_FAILURE);
    }
    // point ptr again to the question part from request message
    ptr = request_message_t_ptr->msg_buffer;
    ptr += 12;
    // copy question part to the new part;
    memcpy(message_question, ptr, total_question_bytes);

    /* get total response byte size except 2 bytes size head for tcp */
    int total_response_size = HEADER_SIZE + TAIL_SIZE + total_question_bytes;

    /* allocate response: +2 bytes means plus two-byte size header for tcp connection */
    unsigned char* response = (unsigned char*) calloc(total_response_size + 2, sizeof (unsigned char));
    if(response == NULL) {
        perror("allocate response in generate_not_implemented_response");
        exit(EXIT_FAILURE);
    }

    /* assemble response */
    ptr = response;
    unsigned char size_head[2];
    size_head[0] = (total_response_size + 2) >> 8;
    size_head[1] = (total_response_size + 2);
    memcpy(ptr, size_head, 2);
    ptr += 2;
    memcpy(ptr, message_header, HEADER_SIZE);
    ptr += HEADER_SIZE;
    memcpy(ptr, message_question, total_question_bytes);
    ptr += total_question_bytes;
    memcpy(ptr, MESSAGE_TAIL, TAIL_SIZE);

    free(message_header);
    free(message_question);

    return response;
}

void purify_pure_AAAA_response(dns_message_t *dns_response_message_t_ptr) {

}

