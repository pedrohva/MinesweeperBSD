#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * Contains variables that help define message functionality
 **/

#define MESSAGE_MAX_SIZE    2048

#define MSGC_ACK            '1'
#define MSGC_PRINT          '2'
#define MSGC_INPUT          '3'
#define MSGC_EXIT           '4'
#define MSGC_DATA           '5'

/**
 * Sends a message to a client/server
 * 
 * The message codes are defined in the messages header file. Each one will be parsed by the client 
 * which will result in differing behaviours.
 * 
 * It has to wait on a response from the client so that multiple 
 * calls to this function don't result in a joined buffer. Since the client throws away anything
 * after a new line, if the buffer is joined, data can be lost.
 **/
int send_message(int sockfd, char msg_code, char* msg);

/**
 * Send message (msg) prompting for input. Then waits until a reply is received. 
 * 
 * The buffer passed to the function will be used to store the reply.
 **/
int receive_message(int sockfd, char* buffer, int buffer_size);

/**
 * Wait for input from the user and then sends it to the client/server
 **/
void send_input(int sockfd);

#endif // MESSAGE_H