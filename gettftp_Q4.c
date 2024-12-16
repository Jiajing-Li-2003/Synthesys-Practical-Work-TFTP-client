#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define RRQ_OPCODE 1
#define ACK_OPCODE 4
#define DATA_OPCODE 3
#define BUFFER_SIZE 516 // Max TFTP packet size (512 bytes data + 4 bytes header)
#define PORT "69"

// Function prototypes
void send_rrq(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, const char *filename);
void receive_data(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, FILE *output_file);
void write_message(const char *message, int error);

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
struct timeval timeout;
timeout.tv_sec = 5;  // Timeout de 5 secondes
timeout.tv_usec = 0; // Pas de microsecondes supplémentaires

if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    const char *err_msg = "Error setting socket timeout\n";
    write(STDERR_FILENO, err_msg, strlen(err_msg));
    close(sockfd);
    freeaddrinfo(server_info);
    return 1;
}

    // Send RRQ
    send_rrq(sockfd, server_info->ai_addr, server_info->ai_addrlen, file_name);
    
     // Open a file to save the received data
    FILE *output_file = fopen(file_name, "wb");
    if (!output_file) {
        const char *err_msg = "Error opening file\n";
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        close(sockfd);
        freeaddrinfo(server_info);
        return 1;
    }

    // Receive data
    receive_data(sockfd, server_info->ai_addr, server_info->ai_addrlen, output_file);
    
    // Cleanup
    fclose(output_file);
    close(sockfd);
    freeaddrinfo(server_info);

    return 0;
}

// Function to send a Read Request (RRQ) to the server
void send_rrq(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, const char *filename) {
    char buffer[BUFFER_SIZE];
    int offset = 0;
    // Add opcode (2 bytes, network byte order)
    buffer[offset++] = 0;
    buffer[offset++] = RRQ_OPCODE;

    // Add filename (null-terminated string)
    strcpy(&buffer[offset], filename);
    offset += strlen(filename) + 1;

    // Add mode (null-terminated string)
    strcpy(&buffer[offset], "octet");
    offset += strlen("octet") + 1;
     // Send the RRQ packet
    if (sendto(sockfd, buffer, offset, 0, server_addr, server_addr_len) == -1) {
        const char *err_msg = "Error sending RRQ\n";
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        exit(1);
    }

    const char *msg = "RRQ sent successfully\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}
// Function to receive data packets and send ACKs
void receive_data(int sockfd, struct sockaddr *server_addr, socklen_t server_addr_len, FILE *output_file) {
    char buffer[BUFFER_SIZE];
    char ack[4];
    int block_number = 1;

    while (1) {
        // Receive a data packet
        ssize_t received_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, server_addr, &server_addr_len);
        if (received_len == -1) {
            const char *err_msg = "Error receiving data\n";
            write(STDERR_FILENO, err_msg, strlen(err_msg));
            exit(1);
        }

        // Check the opcode
        int opcode = (buffer[0] << 8) | buffer[1];
        if (opcode != DATA_OPCODE) {
            const char *err_msg = "Unexpected packet received\n";
            write(STDERR_FILENO, err_msg, strlen(err_msg));
            exit(1);
        }
         // Extract the block number
        int received_block_number = (buffer[2] << 8) | buffer[3];
        if (received_block_number != block_number) {
            const char *err_msg = "Unexpected block number\n";
            write(STDERR_FILENO, err_msg, strlen(err_msg));
            exit(1);
        }

        // Write data to the file
        fwrite(&buffer[4], 1, received_len - 4, output_file);
        
         // Send ACK
        ack[0] = 0;
        ack[1] = ACK_OPCODE;
        ack[2] = buffer[2];
        ack[3] = buffer[3];
        if (sendto(sockfd, ack, sizeof(ack), 0, server_addr, server_addr_len) == -1) {
            const char *err_msg = "Error sending ACK\n";
            write(STDERR_FILENO, err_msg, strlen(err_msg));
            exit(1);
        }

const char *ack_msg = "ACK sent successfully\n";
        write(STDOUT_FILENO, ack_msg, strlen(ack_msg));

        // Check if this is the last packet (less than 512 bytes of data)
        if (received_len < BUFFER_SIZE) {
            const char *msg = "File transfer complete\n";
            write(STDOUT_FILENO, msg, strlen(msg));
            break;
        }

        // Increment block number
        block_number++;
    }
}
