#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Code to handle multiple clients and user authentication

using namespace std;

struct user
{
    string username;
    string password;
};
bool addUser(const string &username, const string &password)
{
    // Open the user database file
    ofstream userFile("users.csv", ios::app);
    if (!userFile.is_open())
    {
        cerr << "Failed to open user database file" << endl;
        return false;
    }

    // Check if username already exists in the file
    ifstream checkFile("users.csv");
    string line;
    while (getline(checkFile, line))
    {
        size_t pos = line.find(",");
        string storedUsername = line.substr(0, pos);
        if (username == storedUsername)
        {
            cerr << "Username " << username << " already exists in the database" << endl;
            checkFile.close();
            userFile.close();
            return false;
        }
    }
    checkFile.close();

    // Write the username and password to the file
    userFile << username << "," << password << endl;

    // Check if an error occurred while writing to the file
    if (userFile.fail())
    {
        cerr << "Failed to write to user database file" << endl;
        userFile.close();
        return false;
    }

    // Close the file
    userFile.close();
    return true;
}

bool authenticateUser(const string &username, const string &password)
{
    // Open the user database file
    ifstream userFile("users.csv");

    if (!userFile.is_open())
    {
        cerr << "Failed to open user database file" << endl;
        return false;
    }

    string line;
    while (getline(userFile, line))
    {
        // Split the line into username and password
        size_t pos = line.find(",");
        string storedUsername = line.substr(0, pos);
        string storedPassword = line.substr(pos + 1);

        // Check if the username and password match
        if (username == storedUsername && password == storedPassword)
        {
            userFile.close();
            return true;
        }
    }

    // Close the file
    userFile.close();
    return false;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port = atoi(argv[1]);
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
    memset(&server_addr, 0, sizeof(server_addr));
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

    cout << "Server started on port " << port << "\n";

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(server_socket, &active_fds);
    char buffer[BUFFER_SIZE];
    int num_clients = 0;

    while (true)
    {
        read_fds = active_fds;
        // Wait for activity on any socket
        if (select(FD_SETSIZE, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            perror("Failed to select sockets");
            return 1;
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

        // Check for new connections
        if (FD_ISSET(server_socket, &read_fds))
        {
            if (num_clients == MAX_CLIENTS)
            {
                // Send error message to client
                char error_msg[] = "Max number of clients reached. Please try again later.\n";
                if (write(server_socket, error_msg, strlen(error_msg)) == -1)
                {
                    perror("Failed to send error message to client");
                    return 1;
                }

                // Reject new connection
                close(accept(server_socket, NULL, NULL));
            }
            else
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
                num_clients++;

                // Print client address
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                printf("New connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

                // Prompt user to login or sign up
                // Prompt user to login or sign up
                char msg[] = "Welcome to the chat server. Please enter your choice:\n1. Login\n2. Sign up\n";
                if (write(client_socket, msg, strlen(msg)) == -1)
                {
                    perror("Failed to send prompt to client");
                    return 1;
                }

                // Get user choice
                if (read(client_socket, buffer, BUFFER_SIZE) == -1)
                {
                    perror("Failed to read from client socket");
                    return 1;
                }
                int choice = atoi(buffer);

                // Process user choice
                if (choice == 1)
                {
                    // User chose to login
                    // TODO: Implement login logic
                }
                else if (choice == 2)
                {
                    // User chose to sign up
                    // Get username and password from client
                    string username, password;
                    if (read(client_socket, buffer, BUFFER_SIZE) == -1)
                    {
                        perror("Failed to read from client socket");
                        return 1;
                    }
                    username = buffer;
                    if (read(client_socket, buffer, BUFFER_SIZE) == -1)
                    {
                        perror("Failed to read from client socket");
                        return 1;
                    }
                    password = buffer;

                    // Add user to the database
                    if (addUser(username, password))
                    {
                        cout << "User " << username << " added to the database" << endl;
                        char msg[] = "User added to database\n";
                        if (write(client_socket, msg, strlen(msg)) == -1)
                        {
                            perror("Failed to send message to client");
                            return 1;
                        }
                    }
                    else
                    {
                        cerr << "Failed to add user " << username << " to the database" << endl;
                        char msg[] = "Failed to add user to database\n";
                        if (write(client_socket, msg, strlen(msg)) == -1)
                        {
                            perror("Failed to send message to client");
                            return 1;
                        }
                    }
                }
                else
                {
                    // Invalid choice
                    char msg[] = "Invalid choice. Please try again.\n";
                    if (write(client_socket, msg, strlen(msg)) == -1)
                    {
                        perror("Failed to send message to client");
                        return 1;
                    }
                }
            }
        }


    }
            // End while loop and close server socket
        close(server_socket);
        return 0;
}