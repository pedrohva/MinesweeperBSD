#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Includes for sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/**
*   Error catching code.
**/
void error(char* message) {
    perror(message);
    exit(1);
}

void receive_message(int sockfd) {
    // Receive message from server
    int msg_size = 0;
    char buffer[1024];
    while(1) {
        bzero(buffer, sizeof(buffer));
        msg_size = recv(sockfd, &buffer, sizeof(buffer), 0);
        if(msg_size == -1) {
            error("Error with message received");
        }
        printf("%s\n", buffer);
        if(msg_size < sizeof(buffer)) {
            break;
        }
    }
}

void send_input(int sockfd) {
    char buffer[1024];
    fgets(buffer, sizeof(buffer), stdin);
    int msg_size = send(sockfd, &buffer, strlen(buffer), 0);
    if(msg_size < 0) {
        error("Error with writing to socket");
    }
}

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

    // Log in
    receive_message(sockfd);
    send_input(sockfd);
    receive_message(sockfd);
    send_input(sockfd);
    receive_message(sockfd);

    return 0;
}