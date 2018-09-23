#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * Contains variables that help define message functionality
 **/
#define MESSAGE_MAX_SIZE    1024

// All messages are sent with a code. This code tells the receiver on how to process the message string.
#define MSGC_ACK            '1' // Used to state that a message was received (relevant for clients as it allows multiple lines to be received in separate messages)
#define MSGC_PRINT          '2' // The message sohuld be printed to the terminal
#define MSGC_INPUT          '3' // The message should be printed to the terminal and the user should be polled for input
#define MSGC_EXIT           '4' // The message should be printed to the terminal and the receiving process should exit
#define MSGC_DATA           '5' // The message sent contains data that should be placed in a variable (eg. the input from an user)
// NOTE: Every message sent requires the receiver to send a MSGC_ACK in response. Exceptions include messages sent with codes MSGC_ACK and MSGC_DATA

/**
 * Send a message to a client/server
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