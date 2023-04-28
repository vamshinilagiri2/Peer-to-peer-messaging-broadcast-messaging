#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

//Code to handle multiple clients 

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port = std::atoi(argv[1]);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Failed to create server socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("Failed to set socket options");
        return 1;
    }
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Failed to bind server socket");
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("Failed to listen for incoming connections");
        return 1;
    }
    std::cout << "Server started on port " << port << "\n";

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(server_socket, &active_fds);
    char buffer[BUFFER_SIZE];

    while (1)
    {
        read_fds = active_fds;

        if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Failed to select sockets");
            return 1;
        }
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
            FD_SET(client_socket, &active_fds);

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "New connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << "\n";
        }
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