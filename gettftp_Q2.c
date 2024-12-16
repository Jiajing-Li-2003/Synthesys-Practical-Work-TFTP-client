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

int main(int argc, char *argv[]) {
    // Step 1: Validate arguments
    if (validate_arguments(argc) != 0) {
        return 1; // Exit if arguments are invalid
    }

    // Step 2: Extract server address and file name from arguments
    const char *server_address = argv[1];
    const char *file_name = argv[2];

	// Step 3: Resolve the server address
    struct addrinfo *server_info = resolve_server_address(server_address, PORT);
    if (server_info == NULL) {
        return 1; // Exit if address resolution failed
    }

    // Step 4: Print success message (for now)
    write(STDOUT_FILENO, SUCCESS_MSG, strlen(SUCCESS_MSG));


    // Step 5: Free the allocated memory for address info
    freeaddrinfo(server_info);

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

