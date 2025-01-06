#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> // For inet_ntop

// Define constants
#define ERROR_MSG "Error: Unable to resolve server address.\n"
#define SUCCESS_MSG "Address resolved successfully.\n"
#define USAGE_MSG "Usage: gettftp <server_address> <file_name>\n"
#define PORT "69" // TFTP standard port
#define IPSTR_LEN INET6_ADDRSTRLEN // Length for IP string representation

// Function prototypes
void print_error(const char *msg);
int validate_arguments(int argc);
struct addrinfo *resolve_server_address(const char *server_address, const char *port);
void display_server_ip(struct addrinfo *res, const char *server_address);

int main(int argc, char *argv[]) {
    // Step 1: Validate command-line arguments
    if (validate_arguments(argc) != 0) {
        return 1; // Exit if arguments are invalid
    }

    // Step 2: Extract server address and file name from arguments
    const char *server_address = argv[1];
    const char *file_name = argv[2];

    // Step 3: Resolve the server address
    struct addrinfo *server_info = resolve_server_address(server_address, PORT);
    if (server_info == NULL) {
        return 1; // Exit if address resolution fails
    }

    // Step 4: Display the server's IP address
    display_server_ip(server_info, server_address);

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
    hints.ai_socktype = SOCK_DGRAM;  // TFTP uses UDP
    hints.ai_protocol = IPPROTO_UDP; // Explicitly specify UDP

    // Call getaddrinfo
    int status = getaddrinfo(server_address, port, &hints, &res);
    if (status != 0) {
        print_error(ERROR_MSG);
        return NULL; // Return NULL to indicate failure
    }

    return res; // Return the resolved address info
}

// Function to display the server's IP address
void display_server_ip(struct addrinfo *res, const char *server_address) {
    char ipstr[IPSTR_LEN]; // Buffer to hold the IP address string
    void *addr;

    // Check if the address is IPv4 or IPv6
    if (res->ai_family == AF_INET) { // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
        addr = &(ipv4->sin_addr);
    } else { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
        addr = &(ipv6->sin6_addr);
    }

    // Convert the IP address to a string
    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));

    // Write the IP address to standard output
    write(STDOUT_FILENO, SUCCESS_MSG, strlen(SUCCESS_MSG));
    write(STDOUT_FILENO, ipstr, strlen(ipstr));
    write(STDOUT_FILENO, "\n", 1);
}


