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
    struct sockaddr_in self_addr, peer_addr;
    char buffer[BUFSIZE];

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Bind to a specific IP and port
    memset(&self_addr, 0, sizeof(self_addr));
    self_addr.sin_family = AF_INET;
    self_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change this to the IP address of your machine
    self_addr.sin_port = htons(8000); // Change this to a port number of your choice
    if (bind(sockfd, (struct sockaddr *) &self_addr, sizeof(self_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, 1) < 0) {
        perror("Failed to listen for incoming connections");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for incoming connection...\n");

    // Accept the incoming connection
    socklen_t peer_addr_size = sizeof(peer_addr);
    int connfd = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    if (connfd < 0) {
        perror("Failed to accept incoming connection");
        exit(EXIT_FAILURE);
    }

    printf("Connection established with %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    // Send and receive messages
    while (1) {
        int recv_len = recv(connfd, buffer, BUFSIZE, 0);
        if (recv_len < 0) {
            perror("Failed to receive message");
            exit(EXIT_FAILURE);
        } else if (recv_len == 0) {
            printf("Peer disconnected\n");
            break;
        } else {
            buffer[recv_len] = '\0';
            printf("Received message: %s\n", buffer);
        }

        printf("Enter message to send: ");
        fgets(buffer, BUFSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline character

        if (strcmp(buffer, "quit") == 0) {
            break;
        }

        if (send(connfd, buffer, strlen(buffer), 0) < 0) {
            perror("Failed to send message");
            exit(EXIT_FAILURE);
        }
    }

    // Close the connection
    close(connfd);
    close(sockfd);

    return 0;
}
