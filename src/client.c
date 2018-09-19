#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Includes for sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "message.h"

/**
* Error catching code.
**/
void error(char* message) {
    perror(message);
    exit(1);
}

/**
 * Prints a message to the console. Will stop when the cursor meets a 
 * new line character ('\n')
 **/
void print_message(char* message) {
    for(int i=0; i < strlen(message); i++) {
        char c = message[i];
        putchar(c);
        fflush(stdout);
        if(c == '\n') {
            return;
        }
    }
}

/**
 * Wait for input from the user and then sends it to the server
 **/
void send_input(int sockfd) {
    char buffer[1024];
    fgets(buffer, sizeof(buffer), stdin);
    int msg_size = send(sockfd, &buffer, strlen(buffer), 0);
    if(msg_size < 0) {
        error("Error with writing to socket");
    }
}

/**
 * Sends a message to the server.
 * The message codes are defined in the message header. These codes are parsed by the server which 
 * determines its behaviour when receiving this message
 **/
int send_message(int sockfd, char msg_code, char* msg) {
    char buffer[512];
    snprintf(buffer, sizeof buffer, "%c%s", msg_code, msg);
    int size = send(sockfd, buffer, strlen(buffer), 0);

    return size;
}

/**
 * Attempts to connect to the server and then will open the main loop where it will attempt to play
 * minesweeper through the connection to the server
 **/
int main(int argc, char *argv[]) {
    int sockfd;                         // The socket field descriptor
    int port_num;                       // The port number to send to
    struct sockaddr_in server_addr;     // The server's address information
    struct hostent *host;               // Defines the host computer on the network

    // Get the hostname and port number
    if(argc != 3) {
        error("Usage: server_hostname port_number\n");
    }
    port_num = atoi(argv[2]);
    host = gethostbyname(argv[1]);
    if(host == NULL) {
        error("Error with hostname");
    }

    // Generate the socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error("Socket generation");
	}

    // Set all values in the buffer to 0
    bzero((char *) &server_addr, sizeof(server_addr));

    // Generate the end points
    server_addr.sin_family = AF_INET;           // Host byte order
	server_addr.sin_port = htons(port_num);     // Short, network byte order 
	server_addr.sin_addr.s_addr = *((in_addr_t *)host->h_addr);    // Address of host

    // Establish a connection to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1) {
		error("Error while attempting to connect to server");
	}

    // Start main loop
    char buffer[MESSAGE_MAX_SIZE];
    char *server_msg = buffer + 1;
    while(1) {
        // Receive message from server
        bzero(buffer, sizeof(buffer));
        int msg_size = recv(sockfd, &buffer, sizeof(buffer), 0);
        if(msg_size == -1) {
            error("Error with message received");
        }
        
        char code = buffer[0];
        switch(code) {
            case MSGC_PRINT:
                // Print the message to the console
                print_message(server_msg);
                send_message(sockfd, MSGC_ACK, "");
                break;
            case MSGC_INPUT:
                // Print the message to the console and then waits for input to send to the server
                print_message(server_msg);
                send_message(sockfd, MSGC_ACK, "");
                send_input(sockfd);
                break;
            case MSGC_EXIT:
                // Print the message to the console and then exit out of the program
                print_message(server_msg);
                send_message(sockfd, MSGC_ACK, "");
                close(sockfd);
                exit(0);
                break;
            default:
                break;
        }
    }

    return 0;
}