#define _GNU_SOURCE // Required to use recursive mutex
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
// Threads
#include <pthread.h>     
// Sockets
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "message.h"

#define PORT_DEFAULT            12345   // The port to listen to when no other option is given
#define THREADPOOL_SIZE         3       // How many working threads will be handling clients at one time
#define CONNECTION_BACKLOG_MAX  20      // The maximum number of connections the server will support
#define RNG_SEED_DEFAULT        42      // The seed used for the random number generator

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

// File read mutex
pthread_mutex_t file_read_mutex = PTHREAD_MUTEX_INITIALIZER;

// Contains the states that the game can be in when being played
enum game_state {
    MAIN_MENU,
    PLAYING,
    HIGHSCORE,
    EXIT
};

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

/**
 * Sends a message to a client. 
 * 
 * The message codes are defined in the messages header file. Each one will be parsed by the client 
 * which will result in differing behaviours.
 * 
 * It has to wait on a response from the client so that multiple 
 * calls to this function don't result in a joined buffer. Since the client throws away anything
 * after a new line, if the buffer is joined, data can be lost.
 **/
int send_message(int sockfd, char msg_code, char* msg) {
    // Concatenate the msg code and the message then transmit to the client
    char buffer[512];
    snprintf(buffer, sizeof buffer, "%c%s", msg_code, msg);
    int size = send(sockfd, buffer, strlen(buffer), 0);

    // Wait for ACK from client
    recv(sockfd, &buffer, sizeof(buffer), 0);

    return size;
}

/**
 * Checks the Authentication text file to see if there is any username-password pair that
 * matches the one passed to this function
 * 
 * Returns a 1 if there is a match
 **/
int client_login_verification(char* username, char* password) {
    // Remove the newline character from the username and password if necessary
    if((strlen(username)-1 > 0) && (username[strlen(username)-1] == '\n')) {
        username[strlen(username)-1] = '\0';
    }
    if((strlen(password)-1 > 0) && (password[strlen(password)-1] == '\n')) {
        password[strlen(password)-1] = '\0';
    }
    
    // Temporary variables that will hold the username and password of each line in the file
    char user[1024];
    char pass[1024];

    // Lock mutex in order to access the authentication file
    //pthread_mutex_lock(&file_read_mutex);

    // Open the authentication file to check for usernames and passwords
    FILE *fp=fopen("Authentication.txt", "r");

    // Get rid of the header in the text file
    fscanf(fp, "%s%s", user, pass); 
    // Iterate through the authentication file
    while(fscanf(fp, "%s%s", user, pass) != EOF) {
        // Check if the username and password given match any on the file
        if(strcmp(username, user) == 0) {
            if(strcmp(password, pass) == 0) {
                // Username and password matched
                return 1;
            }
        }
    }

    // Unlock the mutex to the file so other threads can access it 
    //pthread_mutex_unlock(&file_read_mutex);

    // Username and password did not match
    return 0;
}

/**
 * Displays the welcome screen and prompts the user to type their username or password
 * 
 * Returns a 1 if login was successful
 **/
int client_login(int sockfd) {
    // Display the welcome banner
    send_message(sockfd, MSGC_PRINT, "===================================================\n");
    send_message(sockfd, MSGC_PRINT, "= Welcome to the online Minesweeper gaming system =\n");
    send_message(sockfd, MSGC_PRINT, "===================================================\n");
    send_message(sockfd, MSGC_PRINT, "\n");

    // Get the username from the user
    char username[1024];
    bzero(username, sizeof(username));
    send_message(sockfd, MSGC_INPUT, "Username: ");
    int usr_size = recv(sockfd, &username, sizeof(username), 0);

    // Get the password from the user
    char password[1024];
    bzero(password, sizeof(password));
    send_message(sockfd, MSGC_INPUT, "Password: ");
    int pass_size = recv(sockfd, &password, sizeof(password), 0);

    if(usr_size == -1 || pass_size == -1) {
        return 0;
    }

    // Verify if the username and password matches any on the file. Return 1 if it does.
    return client_login_verification(username, password);
}

void draw_main_menu(int sockfd) {
    send_message(sockfd, MSGC_PRINT, "Welcome to the Minesweeper gaming system.\n");
    send_message(sockfd, MSGC_PRINT, "\n");
    send_message(sockfd, MSGC_PRINT, "Please enter a selection:\n");
    send_message(sockfd, MSGC_PRINT, "<1> Play Minesweeper\n");
    send_message(sockfd, MSGC_PRINT, "<2> Show Leaderboard\n");
    send_message(sockfd, MSGC_PRINT, "<3> Quit\n");
    send_message(sockfd, MSGC_INPUT, "Selection Option (1-3): ");
}

void draw(enum game_state state, int sockfd) {
    switch(state) {
        case MAIN_MENU:
            draw_main_menu(sockfd);
            break;
        default:
            break;
    }
}

void update(enum game_state state, int sockfd) {
    char buffer[1024];
    bzero(buffer, sizeof(buffer));
    recv(sockfd, &buffer, sizeof(buffer), 0);
}

/**
 * The game is played through this loop. It uses a state machine in order to decide what screen to
 * show and which function to call.
 * As this function is called by multiple threads, data relating to the game must be local.
 **/
void game_loop(int sockfd) {
    // Initialize the minesweeper game
    enum game_state state = MAIN_MENU;

    // Start the game loop that plays Minesweeper
    while(state != EXIT) {
        // Draw the screen representing the game state to the terminal
        draw(state, sockfd);

        // Update the game logic (including waiting for input)
        update(state, sockfd);
    }
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

                // Display the welcome banner and check if the client's username and password are authorized to proceed
                if(client_login(client_sockfd)) {
                    // The client has authorization to play the game
                    send_message(client_sockfd, MSGC_PRINT, "\n");
                    send_message(client_sockfd, MSGC_PRINT, "Login successful\n");
                    send_message(client_sockfd, MSGC_PRINT, "\n");
                    game_loop(client_sockfd);
                }else {
                    // The username and password were wrong
                    send_message(client_sockfd, MSGC_PRINT, "\n");
                    send_message(client_sockfd, MSGC_EXIT, "Username or password is incorrect. Disconnecting...\n");
                }

                // Close the socket linking to the client, freeing this thread to connect to another client
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

    // Ignore the SIGPIPE signal that is sent if the client is closed while we're reading or sending data
    signal(SIGPIPE, SIG_IGN);

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