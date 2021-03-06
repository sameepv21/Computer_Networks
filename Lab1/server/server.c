/*
    Name: Sameep Vani
    Roll Number: AU1940049
    Email: sameep.v@ahduni.edu.in
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#define SERVER_PORT 8089
#define MAX_PENDING 5
#define SERVER_IP "127.0.0.1"
#define MAX_LINE 1024

void handle_error(char *error_msg, int socket_fd)
{
    close(socket_fd);
    perror(error_msg);
    exit(EXIT_FAILURE);
}

int main()
{
    struct sockaddr_in sin;
    char buf[MAX_LINE];               // Buffer
    printf("%ld\n", sizeof(buf));
    int sockfd, new_sockfd;           // Sockets
    int len;                          // For storing the length of the msg
    bzero((char *)&sin, sizeof(sin)); // Initialize structure

    /* Modify required variables */
    sin.sin_family = AF_INET;                   // Address Family, Internet
    sin.sin_addr.s_addr = inet_addr(SERVER_IP); // Ip Address in network format
    sin.sin_port = htons(SERVER_PORT);          // Port in network format

    /* Create a socket */
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (sockfd < 0) // Check for error
    {
        handle_error("Socket Failed", sockfd);
    }

    /* Bind the socket with particular port and address */
    if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        handle_error("Bind Failed", sockfd);
    }

    /* Mark socket as passive listening using listen() */
    int error_check = listen(sockfd, MAX_PENDING);
    if (error_check < 0) // Check for error
    {
        handle_error("Listen Failed", sockfd);
    }

    /* Accept the incoming requests */
    while (1)
    {
        new_sockfd = accept(sockfd, (struct sockaddr *)&sin, &len);
        if (new_sockfd < 0) // Error Check
        {
            handle_error("Accept Failed", sockfd);
        }

        /* Send "Hello" from server */
        char *initial_greetings = "Hello";
        if (send(new_sockfd, initial_greetings, MAX_LINE, 0) < 0)
        {
            handle_error("Send Failed", new_sockfd);
        }

        /* Recieve request from client */
        while (len = recv(new_sockfd, buf, MAX_LINE, 0))
        {
            // fputs(buf, stdout);
            if (buf[0] == 'B' && buf[1] == 'y' && buf[2] == 'e' && strlen(buf) < 5) // Terminate the socket if client says bye.
            {
                printf("Bye: Closing Connection\n");
                close(new_sockfd);
                break;
            }
            else
            {
                char *fileName = buf;

                /* Define structure for stat system call */
                int len_of_file = strlen(fileName); // Get the len of file name
                fileName[len_of_file - 1] = '\0';   // Make the last char to indicate the end of the string

                // printf("%s %ld\n", fileName, strlen(fileName));
                
                FILE *fd = fopen(fileName, "rb"); // Open the file in read only mode
                
                if (fd == NULL) // Send file not found through the network
                {
                    char *file_not_found = "File not found";
                    if (send(new_sockfd, file_not_found, MAX_LINE, 0) < 0) // Send error over the socket
                    {
                        handle_error("Sending File Failed!", new_sockfd);
                    }
                }

                char *ok = "OK";
                if (send(new_sockfd, ok, MAX_LINE, 0) < 0) // Send "ok" over the socket
                {
                    handle_error("Sending Ok Failed!", new_sockfd);
                }

                bzero(buf, MAX_LINE);
                long int total_bytes = 0;
                int bytes_read;
                while (!feof(fd)) // Continue the loop until all the bytes of the file is read.
                {
                    bytes_read = fread(buf, 1, MAX_LINE - 1, fd);
                    buf[MAX_LINE - 1] = '\0';
                    // printf("%s\n", buf);
                    if (ferror(fd) != 0)
                    {
                        handle_error("Error in fgets()", new_sockfd);
                    }
                    printf("Bytes Send: %d\n", bytes_read);
                    // printf("BUF CONTENTS: %s\n", buf);
                    total_bytes += bytes_read;
                    if (send(new_sockfd, buf, sizeof(buf), 0) < 0) // Send the read data over the socket
                    {
                        handle_error("File Send Failed!", new_sockfd);
                    }

                    bzero(buf, MAX_LINE); // Erase the previous data
                }
                printf("Total Bytes Sent: %ld\n", total_bytes);
                if (ferror(fd) != 0)
                {
                    handle_error("Error in fgets()", new_sockfd);
                }
                if (feof(fd))
                {
                    char *over = "EOF";
                    printf("SENDING EOF\n");
                    send(new_sockfd, over, MAX_LINE, 0);
                    fclose(fd); // Close the file descriptor
                }
            }
        }
    }
    return 0;
}