#define _GNU_SOURCE // Required to use recursive mutex
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Threads
#include <pthread.h>     
// Sockets
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT_DEFAULT            12345
#define THREADPOOL_SIZE         3
#define CONNECTION_BACKLOG_MAX  5

// Threadpool variables
int client_queue_size = 0;  // How many clients waiting to connect
struct request {
    int request_sockfd;     // The socket the client in the queue is connected to        
    struct request* next;   // Pointer to the next client in the queue
};
struct request* head_client_queue = NULL;   // HEAD of the linked list of the queue of clients
struct request* tail_client_queue = NULL;   // TAIL of the linked list of the queue of clients

// Threadpool mutex and conditions
pthread_mutex_t client_queue_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_cond_t client_queue_got_request = PTHREAD_COND_INITIALIZER;

/**
*   Error logging code.
**/
void error(char* message) {
    perror(message);
    exit(1);
}

/**
*   Add the client that is attempting to connect to the queue.
*   This is done by adding the socket the client is communicating from to a LinkedList. 
*   The Linked List was chosen over a Queue data structure as it allows a dynamic number
*   of clients to connect. 
*/
void client_queue_add(int client_sockfd) {
    struct request* client_request; // Pointer to the new client connect request
    
    // Create the client request structure
    client_request = (struct request*)malloc(sizeof(struct request));
    if(!client_request) {
        error("Error adding client to queue: out of memory");
    }
    client_request->request_sockfd = client_sockfd;
    client_request->next = NULL;

    // Lock the mutex for the queue
    pthread_mutex_lock(&client_queue_mutex);

    // Add the new client connection request to the end of the Linked List
    if(client_queue_size == 0) {
        head_client_queue = client_request;
        tail_client_queue = client_request;
    }else {
        tail_client_queue->next = client_request;
        tail_client_queue = client_request;
    }
    client_queue_size++;

    // Unlock the mutex for the queue
    pthread_mutex_unlock(&client_queue_mutex);

    // Signal the condition variable that the queue has a new request
    pthread_cond_signal(&client_queue_got_request);
}

/**
*   Remove and get the client at the head of the queue. 
*    
*   Return: an int representing the socket number in which the client is communicating to
**/
int client_queue_pop() {
    struct request* client_request; // Pointer to the new client connect request
   
    // Lock the mutex for the queue
    pthread_mutex_lock(&client_queue_mutex);

    // Attempt to get the head of the client queue
    if(client_queue_size > 0) {
        client_request = head_client_queue;
        head_client_queue = client_request->next;
        // If the queue is now empty
        if(head_client_queue == NULL) {
            tail_client_queue = NULL;
        }
        // Decrease the total number of clients in the queue
        client_queue_size--;
    }else {
        // If the client queue is empty
        client_request = NULL;
    }

    // Unlock the mutex for the queue
    pthread_mutex_unlock(&client_queue_mutex);

    // Get the sockfd from the client and free the memory allocated to the structure
    int client_sockfd = client_request->request_sockfd;
    free(client_request);

    // Return the socket the client is communicating at
    if(client_request == NULL) {
        return -1;
    }
    return client_sockfd;
}

int client_send_message(int sockfd, char* msg) {
    int size = send(sockfd, msg, strlen(msg), 0);

    return size;
}

int client_login(int sockfd) {
    client_send_message(sockfd, "===================================================\n");
    client_send_message(sockfd, "= Welcome to the online Minesweeper gaming system =\n");
    client_send_message(sockfd, "===================================================\n");
    client_send_message(sockfd, "\n");

    // Get the username from the user
    client_send_message(sockfd, "Username: ");
    char buffer[1024];
    bzero(buffer, sizeof(buffer));
    int usr_size = recv(sockfd, &buffer, sizeof(buffer), 0);
    printf("username: %s \n", buffer);

    // Get the password from the user
    client_send_message(sockfd, "Password: ");
    bzero(buffer, sizeof(buffer));
    int pass_size = recv(sockfd, &buffer, sizeof(buffer), 0);
    printf("password: %s \n", buffer);

    if(usr_size == -1 || pass_size == -1) {
        client_send_message(sockfd, "Username or password is incorrect");
    }

    return 1;
}

/**
*   The main function that each thread from the thread pool runs. 
*   It will attempt to connect to a client and then start playing the game until the client
*   closes its connection.
**/
void* handle_clients_loop() {    
    // Lock the mutex to the client queue
    pthread_mutex_lock(&client_queue_mutex);

    // Keep handling clients until server is shutdown
    while(1) {
        // Try to connect to a client in the queue if there are any
        if(client_queue_size > 0) {
            int client_sockfd = client_queue_pop();

            if(client_sockfd > -1) {
                // Unlock the mutex to the queue while this thread is connected to a client
                pthread_mutex_unlock(&client_queue_mutex);

                client_login(client_sockfd);

                close(client_sockfd);

                // Lock the mutex now that the client has disconnected and this thread is waiting
                pthread_mutex_lock(&client_queue_mutex);
            }
        }else {
            // Wait for a client to connect. The client queue mutex will be unlocked while waiting
            while(client_queue_size < 1) {
                pthread_cond_wait(&client_queue_got_request, &client_queue_mutex);
            }
        }
    }
}

/**
*   Initializes the server
**/
int main(int argc, char *argv[]) {
    int sockfd, newsockfd;              // Listen on sock_fd, new connection on newsockfd 
    int port_num;                       // The port number to listen on
    struct sockaddr_in server_addr;     // My address information 
	struct sockaddr_in client_addr;     // Client's address information

    // Create the threadpool that will handle the clients
    pthread_t  threadpool[THREADPOOL_SIZE];
    for(int i=0; i < THREADPOOL_SIZE; i++) {
        pthread_create(&threadpool[i], NULL, handle_clients_loop, NULL);
    }

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

    // Start an infinite loop that handles all the incoming connections
    while(1) {
        // Wait until a client sends message to the server
        socklen_t sockin_size = sizeof(struct sockaddr_in);
        newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sockin_size);
        if(newsockfd == -1) {
            error("Accept");
        }
        // Add the client to the queue
        client_queue_add(newsockfd);
        printf("Client connected. Socket: %d. Queue length: %d\n", newsockfd, client_queue_size);
    }

    return 0;
}