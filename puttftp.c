#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define WRQ_OPCODE 2
#define DATA_OPCODE 3
#define ACK_OPCODE 4
#define BUFFER_SIZE 516 // Max TFTP packet size (512 bytes data + 4 bytes header)
#define DATA_SIZE 512   // Maximum size of data in a TFTP DATA packet
#define PORT "69"


// Function prototypes
void send_wrq(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, const char *filename);
void send_data(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, FILE *input_file);
int receive_ack(int sockfd, struct sockaddr *server_addr, socklen_t *server_addr_len, int block_number);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        const char *usage_msg = "Usage: program <server_address> <file_name>\n";
        write(STDERR_FILENO, usage_msg, strlen(usage_msg));
        return 1;
    }

    const char *server_address = argv[1];
    const char *file_name = argv[2];

// Resolve server address
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(server_address, PORT, &hints, &server_info) != 0) {
        const char *err_msg = "Error resolving server address\n";
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        return 1;
    }

 // Create a socket
    int sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (sockfd == -1) {
        const char *err_msg = "Error creating socket\n";
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        freeaddrinfo(server_info);
        return 1;
    }
    
     // Open the file to be sent
    FILE *input_file = fopen(file_name, "rb");
    if (!input_file) {
        const char *err_msg = "Error opening file\n";
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        close(sockfd);
        freeaddrinfo(server_info);
        return 1;
    }

    // Send WRQ
    send_wrq(sockfd, server_info->ai_addr, server_info->ai_addrlen, file_name);

    // Send data packets
    send_data(sockfd, server_info->ai_addr, server_info->ai_addrlen, input_file);
    
     // Cleanup
    fclose(input_file);
    close(sockfd);
    freeaddrinfo(server_info);

    return 0;
}

// Function to send a Write Request (WRQ) to the server
void send_wrq(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, const char *filename) {
    char buffer[BUFFER_SIZE];
    int offset = 0;
// Add opcode (2 bytes, network byte order)
    buffer[offset++] = 0;
    buffer[offset++] = WRQ_OPCODE;

    // Add filename (null-terminated string)
    strcpy(&buffer[offset], filename);
    offset += strlen(filename) + 1;

    // Add mode (null-terminated string)
    strcpy(&buffer[offset], "octet");
    offset += strlen("octet") + 1;
    
    // Send the WRQ packet
    if (sendto(sockfd, buffer, offset, 0, server_addr, server_addr_len) == -1) {
        const char *err_msg = "Error sending WRQ\n";
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        exit(1);
    }

    const char *msg = "WRQ sent successfully\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}
// Function to send data packets
void send_data(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, FILE *input_file) {
    char buffer[BUFFER_SIZE];
    int block_number = 1;

    while (1) {
        // Read data from the file
        size_t bytes_read = fread(&buffer[4], 1, DATA_SIZE, input_file);

// Check for end of file or read error
        if (bytes_read == 0 && feof(input_file)) {
            break;
        }
        if (ferror(input_file)) {
            const char *err_msg = "Error reading file\n";
            write(STDERR_FILENO, err_msg, strlen(err_msg));
            exit(1);
        }
        
        // Build the DATA packet
        buffer[0] = 0;
        buffer[1] = DATA_OPCODE;
        buffer[2] = (block_number >> 8) & 0xFF; // High byte of block number
        buffer[3] = block_number & 0xFF;        // Low byte of block number
        // Send the DATA packet
        if (sendto(sockfd, buffer, bytes_read + 4, 0, server_addr, server_addr_len) == -1) {
            const char *err_msg = "Error sending DATA packet\n";
            write(STDERR_FILENO, err_msg, strlen(err_msg));
            exit(1);
        }

        const char *msg = "DATA packet sent\n";
        write(STDOUT_FILENO, msg, strlen(msg));
  // Wait for the ACK
        if (!receive_ack(sockfd, server_addr, &server_addr_len, block_number)) {
            const char *err_msg = "Error receiving ACK\n";
            write(STDERR_FILENO, err_msg, strlen(err_msg));
            exit(1);
        }

        block_number++;
    }

    const char *msg = "File transfer complete\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}
// Function to receive an ACK for a specific block number
int receive_ack(int sockfd, struct sockaddr *server_addr, socklen_t *server_addr_len, int block_number) {
    char buffer[BUFFER_SIZE];

    // Receive the ACK packet
    ssize_t received_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, server_addr, server_addr_len);
    if (received_len == -1) {
        return 0; // Error receiving ACK
    }
      // Check the opcode
    int opcode = (buffer[0] << 8) | buffer[1];
    if (opcode != ACK_OPCODE) {
        return 0; // Unexpected packet received
    }

    // Check the block number
    int received_block_number = (buffer[2] << 8) | buffer[3];
    if (received_block_number != block_number) {
        return 0; // Unexpected block number
    }

    return 1; // ACK received successfully
}
