#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Define constants
#define ERROR_MSG "Error: Unable to resolve server address.\n"
#define SUCCESS_MSG "Address resolved successfully.\n"
#define USAGE_MSG "Usage: gettftp <server_address> <file_name>\n"
#define SERVER_ADDRESS_BUFFER 256
#define PORT "69" // Server ensea uses port 69

// Function prototypes
void print_error(const char *msg);
int validate_arguments(int argc);
struct addrinfo *resolve_server_address(const char *server_address, const char *port);
int create_socket(struct addrinfo *server_info);


int main(int argc, char *argv[]) {
    // Step 1: Validate the number of arguments
    if (argc != 2) {
        write(STDERR_FILENO, USAGE_MSG, strlen(USAGE_MSG));
        return 1;
    }

    // Step 2: Resolve the server address
    const char *server_address = argv[1];
    struct addrinfo *server_info = resolve_server_address(server_address, PORT);
    if (server_info == NULL) {
        return 1; // Exit if address resolution failed
    }

	// Step 3: Create and reserve a socket
    int socket_fd = create_socket(server_info);
    if (socket_fd == -1) {
        freeaddrinfo(server_info); // Free the address info before exiting
        return 1; // Exit if socket creation failed
    }

    // Step 4: Print success message
    write(STDOUT_FILENO, SUCCESS_MSG, strlen(SUCCESS_MSG));

    // Step 5: Free the address info and close the socket
    freeaddrinfo(server_info);
    close(socket_fd);

    return 0;
}


// Function to print error messages to standard error
void print_error(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}
// Function to validate the number of command-line arguments

int validate_arguments(int argc) {
    if (argc != 3) {
        print_error(USAGE_MSG);
        return 1; // Return error if incorrect number of arguments
    }
    return 0; // Return success
}


// Function to resolve the server address using getaddrinfo
struct addrinfo *resolve_server_address(const char *server_address, const char *port) {
    struct addrinfo hints, *res;

    // Initialize hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Use IPv4
    hints.ai_socktype = SOCK_DGRAM;  // Use UDP (TFTP uses UDP)
    hints.ai_protocol = IPPROTO_UDP; // Explicitly specify UDP

// Call getaddrinfo
    int status = getaddrinfo(server_address, port, &hints, &res);
    if (status != 0) {
        print_error(ERROR_MSG);
        print_error(gai_strerror(status)); // Print the error message from getaddrinfo
        print_error("\n");
        return NULL; // Return NULL to indicate failure
    }

    return res; // Return the resolved address info
}

// Function to create and reserve a socket using the resolved server address
int create_socket(struct addrinfo *server_info) {
    // Create a socket using the information from server_info
    int sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (sockfd == -1) {
        print_error(ERROR_MSG);
        return -1; // Return -1 if socket creation failed
    }

    return sockfd; // Return the socket file descriptor
}
