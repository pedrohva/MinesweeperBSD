#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Sockets
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "message.h"

/**
 * Sends a message to a client/server
 * 
 * The message codes are defined in the messages header file. Each one will be parsed by the client 
 * which will result in differing behaviours.
 * 
 * It has to wait on a response from the client so that multiple 
 * calls to this function don't result in a joined buffer. Since the client throws away anything
 * after a new line, if the buffer is joined, data can be lost.
 * 
 * The wait flag tells to pause until a message is received back
 **/
int send_message(int sockfd, char msg_code, char* msg) {
    // Concatenate the msg code and the message then transmit to the client
    char buffer[MESSAGE_MAX_SIZE];
    snprintf(buffer, sizeof buffer, "%c%s", msg_code, msg);
    int size = send(sockfd, buffer, strlen(buffer), 0);

    // MSGC_ACK and MSGC_DATA don't actually contain a string message to print so we don't need to wait
    if((msg_code != MSGC_ACK) && (msg_code != MSGC_DATA)) {
        // Wait for ACK from client
        recv(sockfd, &buffer, sizeof(buffer), 0);
    }

    return size;
}

/**
 * Waits until there is a message at the socket. 
 * 
 * The buffer passed to the function will be used to store the reply.
 * Returns the size of message received
 **/
int receive_message(int sockfd, char* buffer, int buffer_size) {
    // Clear out the buffer and wait for message
    bzero(buffer, buffer_size);
    int size = recv(sockfd, buffer, buffer_size, 0);

    return size;
}

/**
 * Wait for input from the user and then sends it to the server
 **/
void send_input(int sockfd) {
    char buffer[MESSAGE_MAX_SIZE];
    fgets(buffer, sizeof(buffer), stdin);
    int msg_size = send_message(sockfd, MSGC_DATA, buffer);
    if(msg_size < 0) {
        perror("Error with writing to socket");
    }
}