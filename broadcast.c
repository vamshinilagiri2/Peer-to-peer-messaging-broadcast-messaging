#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    // Check for command line arguments
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Failed to create server socket");
        return 1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("Failed to set socket options");
        return 1;
    }

    // Bind socket to address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Failed to bind server socket");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("Failed to listen for incoming connections");
        return 1;
    }

    printf("Server started on port %d\n", port);

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(server_socket, &active_fds);
    char buffer[BUFFER_SIZE];

    while (1)
    {
        read_fds = active_fds;

        // Wait for activity on any socket
        if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Failed to select sockets");
            return 1;
        }

        // Check for new connections
        if (FD_ISSET(server_socket, &read_fds))
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
            if (client_socket == -1)
            {
                perror("Failed to accept client connection");
                return 1;
            }

            // Add client socket to active fds
            FD_SET(client_socket, &active_fds);

            // Print client address
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("New connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        }

        // Check for activity on client sockets
        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (i != server_socket && FD_ISSET(i, &read_fds))
            {
                int n = read(i, buffer, BUFFER_SIZE);

                if (n == -1)
                {
                    perror("Failed to read from client socket");
                    return 1;
                }
                else if (n == 0)
                {
                    // Client disconnected
                    close(i);
                    FD_CLR(i, &active_fds);
                }
                else
                {
                    // Broadcast message to all clients except sender
                    for (int j = 0; j < FD_SETSIZE; j++)
                    {
                        if (j != server_socket && j != i && FD_ISSET(j, &active_fds))
                        {
                            if (write(j, buffer, n) == -1)
                            {
                                perror("Failed to write to client socket");
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}