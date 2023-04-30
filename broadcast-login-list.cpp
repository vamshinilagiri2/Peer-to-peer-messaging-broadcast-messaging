#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <algorithm>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define DELIMITER '\n'

using namespace std;

struct user
{
    string username;
    string password;
};

struct client_info
{
    int fd;
    string ip;
    int port;
    bool authenticated; // Add this field.
};

bool addUser(const string &username, const string &password)
{
    ofstream userFile("users.csv", ios::app);
    if (!userFile.is_open())
    {
        cerr << "Failed to open user database file" << endl;
        return false;
    }

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
    userFile << username << "," << password << endl;
    if (userFile.fail())
    {
        cerr << "Failed to write to user database file" << endl;
        userFile.close();
        return false;
    }

    userFile.close();
    return true;
}

bool authenticateUser(const string &username, const string &password)
{
    ifstream userFile("users.csv");
    if (!userFile.is_open())
    {
        cerr << "Failed to open user database file" << endl;
        return false;
    }

    string line;
    while (getline(userFile, line))
    {
        size_t pos = line.find(",");
        string storedUsername = line.substr(0, pos);
        string storedPassword = line.substr(pos + 1);
        if (username == storedUsername && password == storedPassword)
        {
            userFile.close();
            return true;
        }
    }
    userFile.close();
    return false;
}
string readString(int socket)
{
    string result;
    char c;
    while (read(socket, &c, 1) == 1 && c != DELIMITER)
    {
        result += c;
    }
    return result;
}

void writeString(int socket, const string &msg)
{
    string data = msg + DELIMITER;
    write(socket, data.c_str(), data.length());
}
void sendClientList(int client_socket, const vector<client_info> &clients)
{
    string client_list = "Connected clients:";
    // Iterate through the connected clients.
    for (const client_info &client : clients)
    {
        if (client.authenticated)
        {
            client_list += "\n" + client.ip + ":" + to_string(client.port);
        }
    }

    client_list += "\n";
    writeString(client_socket, client_list);
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
    vector<client_info> clients;
    while (true)
    {
        read_fds = active_fds;
        if (select(FD_SETSIZE, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            perror("Failed to select sockets");
            return 1;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == server_socket)
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
                    int client_port = ntohs(client_addr.sin_port);
                    printf("New connection from %s:%d\n", client_ip, client_port);

                    clients.push_back({client_socket, client_ip, client_port, false});
                    sendClientList(client_socket, clients);

                    writeString(client_socket, "Welcome to the chat server. Please enter your choice:\n1. Login\n2. Sign up\n");
                }
                else
                {
                    string message = readString(i);
                    if (message.empty())
                    { // The client has disconnected.
                        // Remove the disconnected client from the list.
                        for (auto it = clients.begin(); it != clients.end(); ++it)
                        {
                            if (it->fd == i)
                            {
                                clients.erase(it);
                                break;
                            }
                        }
                        // Close the socket and remove it from the active file descriptor set.
                        close(i);
                        FD_CLR(i, &active_fds);
                    }
                    else
                    {

                        auto client_it = find_if(clients.begin(), clients.end(), [i](const client_info &client)
                                                 { return client.fd == i; });

                        // If the client is not authenticated, process the login or signup request.
                        if (client_it != clients.end() && !client_it->authenticated)
                        {
                            int choice = stoi(message);
                            if (choice == 1)
                            {
                                string username = readString(i);
                                string password = readString(i);

                                if (authenticateUser(username, password))
                                {
                                    cout << "User " << username << " logged in" << endl;
                                    writeString(i, "Login successful");
                                    client_it->authenticated = true;
                                    sendClientList(i, clients);
                                }
                                else
                                {
                                    cerr << "Failed to authenticate user " << username << endl;
                                    writeString(i, "Login failed");
                                }
                            }
                            else if (choice == 2)
                            {
                                string username = readString(i);
                                string password = readString(i);

                                if (addUser(username, password))
                                {
                                    cout << "User " << username << " added to the database" << endl;
                                    writeString(i, "User added to database");
                                }
                                else
                                {
                                    cerr << "Failed to add user " << username << " to the database" << endl;
                                    writeString(i, "Failed to add user to database");
                                }
                            }
                            else
                            {
                                writeString(i, "Invalid choice. Please try again.");
                            }
                        }
                        else
                        {
                            // If the client is authenticated, broadcast the message.
                            for (const client_info &client : clients)
                            {
                                if (client.fd != server_socket && client.fd != i && client.authenticated)
                                {
                                    writeString(client.fd, message);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
