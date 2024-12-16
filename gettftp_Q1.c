#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Define constants for error messages and buffer sizes
#define USAGE_MSG "Usage: gettftp <server_address> <file_name>\n" //right command line
#define ARG_ERROR_MSG "Error: Incorrect number of arguments provided.\n"
#define SERVER_ADDRESS_BUFFER 256
#define FILE_NAME_BUFFER 256

int main(int argc, char *argv[]) {
    // Send an error if the number of arguments is incorrect (program name + 2 arguments)
    if (argc != 3) {
        write(STDERR_FILENO, ARG_ERROR_MSG, strlen(ARG_ERROR_MSG));
        write(STDERR_FILENO, USAGE_MSG, strlen(USAGE_MSG));
        return 1;
    }
    
    // Buffers to store server address and file name
	//argv[0] content program's name (./gettftp).
	//argv[1] server's adress (exemple:192.168.1.1).
	//argv[2] file's name (example: fichier.txt).
    
    char server_address[SERVER_ADDRESS_BUFFER];
    char file_name[FILE_NAME_BUFFER];

    strncpy(server_address, argv[1], SERVER_ADDRESS_BUFFER - 1);
    server_address[SERVER_ADDRESS_BUFFER - 1] = '\0'; // Null-terminate to prevent overflow

    strncpy(file_name, argv[2], FILE_NAME_BUFFER - 1);
    file_name[FILE_NAME_BUFFER - 1] = '\0'; // Null-terminate to prevent overflow

	//Show the arguments written
    write(STDOUT_FILENO, "Server Address: ", 16);
    write(STDOUT_FILENO, server_address, strlen(server_address));
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "File Name: ", 11);
    write(STDOUT_FILENO, file_name, strlen(file_name));
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}
