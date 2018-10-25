#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
// Sockets
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "message.h"

// The socket field descriptor
int sockfd;                         

/**
* Error catching code.
**/
void error(char* message) {
    perror(message);
    exit(1);
}

void signal_handler(int signal_num) {
    // Close the program safely if a SIGPIPE or SIGINT signal is received
    if(signal_num == SIGPIPE) {
        printf("Lost connection to the server. Disconnecting...\n");
        close(sockfd);
        exit(0);
    }
    if(signal_num == SIGINT) {
        close(sockfd);
        exit(0);
    }
}

/**
 * Prints a message to the console. Will stop when the cursor meets a 
 * new line character ('\n'). Note: the first new line character met will 
 * be printed as well.
 **/
void print_message(int sockfd, char* message) {
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
 * Attempts to connect to the server and then will open the main loop where it will attempt to play
 * minesweeper through the connection to the server
 **/
int main(int argc, char *argv[]) {
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
    
    // Bind the SIGPIPE signal to our signal handler
    signal(SIGPIPE, signal_handler);
    // Bind the SIGINT signal to our signal handler 
    signal(SIGINT, signal_handler);

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
        // Test if the socket is still alive
        int error_num = 0;
        socklen_t len = sizeof (error_num);
        getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &error_num, &len);
        if(error_num != 0) {
            // Tell our signal handler to act as if a SIGPIPE signal was sent
            signal_handler(SIGPIPE);
        }

        // Receive message from server
        int msg_size = receive_message(sockfd, buffer, sizeof(buffer));
        if(msg_size == -1) {
            error("Error with message received");
        }
        
        // Perform a certain function depending on the message code (see message.h for list of codes)
        char code = buffer[0];
        switch(code) {
            case MSGC_PRINT:
                // Print the message to the console
                print_message(sockfd, server_msg);
                // Sends an ACK to the server so that the next line - if any - can be sent
                send_message(sockfd, MSGC_ACK, "");
                break;
            case MSGC_INPUT:
                // Print the message to the console and then waits for input to send to the server
                print_message(sockfd, server_msg);
                // Sends an ACK to the server so that the next line - if any - can be sent
                send_message(sockfd, MSGC_ACK, "");
                send_input(sockfd);
                break;
            case MSGC_EXIT:
                // Print the message to the console and then exit out of the program
                print_message(sockfd, server_msg);
                // Sends an ACK to the server so that the next line - if any - can be sent
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