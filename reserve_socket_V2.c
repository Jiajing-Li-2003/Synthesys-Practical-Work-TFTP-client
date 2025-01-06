#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Define constants
#define PORT 8080
#define BUFFER_SIZE 1024
#define ERROR_MSG_SOCKET "Error: Unable to create socket.\n"
#define ERROR_MSG_CONNECT "Error: Connection failed.\n"
#define ERROR_MSG_SEND "Error: Sending failed.\n"
#define ERROR_MSG_RECEIVE "Error: Receiving failed.\n"

// Function to print error messages
void print_error(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));  // Write error message to stderr
}

// Function to create a socket
int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        print_error(ERROR_MSG_SOCKET);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Function to prepare server address
void prepare_server_addr(struct sockaddr_in *server_addr) {
    server_addr->sin_family = AF_INET; // Use IPv4
    server_addr->sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    server_addr->sin_port = htons(PORT); // Port number
}

// Function to connect to the server
void connect_to_server(int sockfd, struct sockaddr_in *server_addr) {
    if (connect(sockfd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        print_error(ERROR_MSG_CONNECT);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

// Function to send a message to the server
void send_message(int sockfd, const char *message) {
    if (send(sockfd, message, strlen(message), 0) < 0) {
        print_error(ERROR_MSG_SEND);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

// Function to receive a message from the server
void receive_message(int sockfd, char *buffer) {
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        print_error(ERROR_MSG_RECEIVE);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0'; // Terminate the received string
}

// Main function
int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Step 1: Create the socket
    sockfd = create_socket();

    // Step 2: Prepare the server address
    prepare_server_addr(&server_addr);

    // Step 3: Connect to the server
    connect_to_server(sockfd, &server_addr);

    // Step 4: Send a message to the server
    const char *message = "Hello, Server!";
    send_message(sockfd, message);

    // Step 5: Receive the response from the server
    receive_message(sockfd, buffer);

    // Step 6: Write the server response to stdout (you can replace this if needed)
    write(STDOUT_FILENO, "Server response: ", 17);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    // Step 7: Close the socket
    close(sockfd);
    return 0;
}
