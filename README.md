The goal of the lab is to create a client TFTP which can exchange files.

1) We created the variable argv which stores the data entered by the user.
 strncpy(server_address, argv[1], SERVER_ADDRESS_BUFFER - 1);
 strncpy(file_name, argv[2], FILE_NAME_BUFFER - 1);
The strncpy function take the argument and write it in the server_adress or file_name.

We then show it in the terminal


2) We wreated a independant function print_error, 'validate_arguments' (already written in Q1) which assure the right number of arguments,display_server_ip (information of the server obtain thanks to getaddrinfo).
   
