#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Includes for sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT_DEFAULT            12345
#define CONNECTION_BACKLOG_MAX  5

/**
*   Error catching code.
**/
void error(char* message) {
    perror(message);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;              // Listen on sock_fd, new connection on newsockfd 
    int port_num;                       // The port number to listen on
    struct sockaddr_in server_addr;     // My address information 
	struct sockaddr_in client_addr;     // Client's address information

    // Get port number for server to listen on
	if(argc != 2) {
        port_num = PORT_DEFAULT;
	} else {
        port_num = atoi(argv[1]);
    }

    // Generate the socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error("Socket generation");
	}

    // Set all values in the buffer to 0
    bzero((char *) &server_addr, sizeof(server_addr));

    // Generate the end points
    server_addr.sin_family = AF_INET;               // Host byte order
	server_addr.sin_port = htons(port_num);         // Short, network byte order 
	server_addr.sin_addr.s_addr = INADDR_ANY;       // Auto-fill with my IP 

    // Binds the socket to listen on to this server's address
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		error("Binding socket");
	}

    // Start listening
    if(listen(sockfd, CONNECTION_BACKLOG_MAX) == -1) {
		error("Listen");
	}
    printf("Server is listening...\n");

    // Wait until a client sends message to the server
    socklen_t sockin_size = sizeof(struct sockaddr_in);
    newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sockin_size);
    if(newsockfd == -1) {
        error("Accept");
    }

    // Receive message from client
    char buffer[256];
    bzero(buffer, 256);
    int msg_size = recv(newsockfd, &buffer, sizeof(buffer), 0);
    if(msg_size < 0) {
        error("Receiving message");
    }

    printf("Message received: %s\n", buffer);

    return 0;
}