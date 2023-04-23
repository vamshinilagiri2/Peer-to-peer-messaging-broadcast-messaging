#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to %s:%d\n", argv[1], atoi(argv[2]));

    // Send and receive messages
    while (1) {
        printf("Enter message to send: ");
        fgets(buffer, BUFSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline character

        if (strcmp(buffer, "quit") == 0) {
            break;
        }

        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("Failed to send message");
            exit(EXIT_FAILURE);
        }

        int recv_len = recv(sockfd, buffer, BUFSIZE, 0);
        if (recv_len < 0) {
            perror("Failed to receive message");
            exit(EXIT_FAILURE);
        } else if (recv_len == 0) {
            printf("Server disconnected\n");
            break;
        } else {
            buffer[recv_len] = '\0';
            printf("Received message: %s\n", buffer);
        }
    }

    // Close the connection
    close(sockfd);

    return 0;
}
