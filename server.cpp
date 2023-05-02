// ======================  Libraries =====================
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <netdb.h>
#include <map>
#include <thread>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/evp.h>


#define MAX_CONNECTIONS 50   // Maxmium client connections to the server
#define BUFFER_SIZE 1024
#define BROADCAST_TIME_LIMIT 1800
std::vector<std::thread> threads;
std::vector<int> Num_clients;
std::vector<std::string> all_clients;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
std::map<std::string, int> client_map;

//Password hasing 

std::string hash_password(const std::string &password)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    if (mdctx == nullptr)
    {
        std::cerr << "Error creating EVP_MD_CTX" << std::endl;
        return "";
    }
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(mdctx, password.c_str(), password.length()) != 1 ||
        EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1)
    {
        std::cerr << "Error computing hash" << std::endl;
        EVP_MD_CTX_free(mdctx);
        return "";
    }
    EVP_MD_CTX_free(mdctx);
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

int udp_socket;
void handle_new_client(int client_socket);
int msd = 0;


void accept_clients(int tcp_socket)
{
    int connection_count = 0;
    while (true)
    {
        if (connection_count >= MAX_CONNECTIONS)
        {
            std::cerr << "Maximum connections reached." << std::endl;
            break;
        }
        int client_socket, client_addr_len;
        struct sockaddr_in client_address;
        client_addr_len = sizeof(client_address);
        if ((client_socket = accept(tcp_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_addr_len)) < 0)
        {
            std::cerr << "Error accepting new connection." << std::endl;
            continue;
        }
        connection_count++;
        std::cout << "Port number: " << ntohs(client_address.sin_port) << ":" << inet_ntoa(client_address.sin_addr) << std::endl;
        std::thread client_thread(handle_new_client, client_socket);
        client_thread.detach();
        while (true)
        {
            std::string response = "Please choose an option: 1 for login or 2 for signup\n";
            send(client_socket, response.c_str(), response.length(), 0);
            char buffer[1024];
            int bytes_received = recv(client_socket, buffer, 1024, 0);
            std::string request(buffer, bytes_received);
            std::string choice(buffer, bytes_received);
            if (choice == "1")
            {
                response = "Please enter your username:\n";
                send(client_socket, response.c_str(), response.length(), 0);
                bytes_received = recv(client_socket, buffer, 1024, 0);
                if (bytes_received < 0)
                {
                    return;
                }
                std::string username(buffer, bytes_received);
                response = "Please enter your password:\n";
                send(client_socket, response.c_str(), response.length(), 0);
                bytes_received = recv(client_socket, buffer, 1024, 0);
                if (bytes_received < 0)
                {
                    return;
                }
                std::string password(buffer, bytes_received);
                std::string hashed_password = hash_password(password);
                std::ifstream file("usercreds.csv");
                std::string line;
                bool authenticated = false;
                while (std::getline(file, line))
                {
                    size_t pos = line.find(',');
                    if (pos != std::string::npos)
                    {
                        std::string file_username = line.substr(0, pos);
                        std::string file_password = line.substr(pos + 1);
                        if (file_username == username && file_password == hashed_password)
                        {
                            authenticated = true;
                            break;
                        }
                    }
                }
                file.close();
                if (authenticated)
                {
                    response = "successfully logged in !\n";
                    send(client_socket, response.c_str(), response.length(), 0);
                    msd++;
                    response = "Please enter anything and press enter to continue : \n";
                    send(client_socket, response.c_str(), response.length(), 0);
                    uint16_t port = ntohs(client_address.sin_port);
                    std::string portno = std::to_string(port);
                    all_clients.push_back("Client " + std::to_string(msd) + " : " + inet_ntoa(client_address.sin_addr) + ":" + portno);
                    std::string Ip = inet_ntoa(client_address.sin_addr);
                    std::string final_address = Ip + ":" + portno;
                    client_map.insert(std::make_pair(final_address, client_socket));
                    for (int users = 0; users < all_clients.size(); users++)
                    {
                        std::cout << all_clients[users] << " ";
                    }
                    Num_clients.push_back(client_socket);
                    break;
                }
                else
                {
                    response = "Invalid credentials. Please try again : \n";
                    send(client_socket, response.c_str(), response.length(), 0);
                }
            }
            else if (choice == "signup")
            {
                response = "Please enter a new unique username:\n";
                send(client_socket, response.c_str(), response.length(), 0);
                bytes_received = recv(client_socket, buffer, 1024, 0);
                if (bytes_received < 0)
                {
                    return;
                }
                std::string new_username(buffer, bytes_received);
                std::ifstream file("usercreds.csv");
                std::string line;
                bool username_exists = false;
                while (std::getline(file, line))
                {
                    size_t pos = line.find(',');
                    if (pos != std::string::npos)
                    {
                        std::string username_from_file = line.substr(0, pos);
                        if (new_username == username_from_file)
                        {
                            username_exists = true;
                            break;
                        }
                    }
                }
                file.close();
                if (username_exists)
                {
                    response = "Username already exists. Please choose another username.\n";
                    send(client_socket, response.c_str(), response.length(), 0);
                }
                else
                {
                    response = "Please enter a new unique password:\n";
                    send(client_socket, response.c_str(), response.length(), 0);
                    bytes_received = recv(client_socket, buffer, 1024, 0);
                    if (bytes_received < 0)
                    {
                        return;
                    }
                    std::string new_password(buffer, bytes_received);
                    response = "re enter your password:\n";
                    send(client_socket, response.c_str(), response.length(), 0);
                    bytes_received = recv(client_socket, buffer, 1024, 0);
                    if (bytes_received < 0)
                    {
                        return;
                    }
                    std::string confirmed_password(buffer, bytes_received);
                    if (new_password != confirmed_password)
                    {
                        response = "Passwords do not match. Please try again.\n";
                        send(client_socket, response.c_str(), response.length(), 0);
                    }
                    else
                    {
                        std::string hashed_password = hash_password(confirmed_password);
                        std::ofstream outfile;
                        outfile.open("usercreds.csv", std::ios_base::app);
                        outfile << new_username << "," << hashed_password << std::endl;
                        outfile.close();
                        response = "Signup successful. Please login to continue.\n";
                        send(client_socket, response.c_str(), response.length(), 0);
                    }
                }
            }
            else
            {
                response = "Invalid option. Please choose 1 for login or 2 for signup\n";
                send(client_socket, response.c_str(), response.length(), 0);
            }
        }
    }
}


using namespace std;
int main()
{
    int tcp_socket;
    struct sockaddr_in tcp_address;
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Error creating TCP socket." << std::endl;
        exit(EXIT_FAILURE);
    }
    tcp_address.sin_family = AF_INET;
    tcp_address.sin_addr.s_addr = htonl(INADDR_ANY);
    tcp_address.sin_port = htons(4400); // port number 4400 as mentioned in project description
    if (bind(tcp_socket, (struct sockaddr *)&tcp_address, sizeof(tcp_address)) < 0)
    {
        std::cerr << "Error binding TCP socket." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (listen(tcp_socket, MAX_CONNECTIONS) < 0)
    {
        std::cerr << "Error listening for connections." << std::endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        threads.push_back(std::thread(accept_clients, tcp_socket));
    }
    for (auto &t : threads)
    {
        t.join();
    }
    return 0;
}
void handle_new_client(int client_socket)
{
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, 1024, 0);
    std::string option(buffer, bytes_received);
    while (true)
    {
        std::string response = "Please Choose an option:\n1. List of active clients\n2. Broadcast a message\n3.To establish peer connection enter 3_address";
        send(client_socket, response.c_str(), response.length(), 0);
        bytes_received = recv(client_socket, buffer, 1024, 0);
        if (bytes_received < 0)
        {
            return;
        }
        size_t dpos = option.find("_");
        std::string first_segment = option.substr(0, dpos);
        std::string second_segment = option.substr(dpos + 1);
        size_t dpos2 = second_segment.find("_");
        std::string first_segment2 = second_segment.substr(0, dpos2);
        std::string second_segment2 = second_segment.substr(dpos2 + 1);
        if (option == "1")
        {
            std::string active_clients_list = "Active Clients:\n";
            for (const auto &client : all_clients)
            {
                active_clients_list += client + "\n";
            }
            send(client_socket, active_clients_list.c_str(), active_clients_list.length(), 0);
        }
        else if (option == "2")
        {
            std::string response = "Enter the message to broadcast:\n";
            send(client_socket, response.c_str(), response.length(), 0);
            bytes_received = recv(client_socket, buffer, 1024, 0);
            if (bytes_received < 0)
            {
                return;
            }
            std::string msg(buffer, bytes_received);
            std::string broadcast_message = "Broadcasted Message ::\n" + msg;
            for (int client_socket : Num_clients)
            {
                send(client_socket, broadcast_message.c_str(), broadcast_message.size(), 0);
            }
            response = "Message broadcasted successfully!\n";
            send(client_socket, response.c_str(), response.length(), 0);
        }
        else if (first_segment == "p2p")
        {
            std::cerr << "Entered 3rd segment." << std::endl;
            response = first_segment2;
            std::string serialized_map;
            for (const auto &pair : client_map)
            {
                serialized_map += pair.first + "," + std::to_string(pair.second) + "\n";
            }
            int target_client_index = client_map[second_segment2];
            std::cout << " client is : " << second_segment2 << "\n";
            std::cout << " Message is : " << first_segment2 << "\n";
            send(target_client_index, response.c_str(), response.length(), 0);
        }
        else
        {
            std::string response = "Invalid option. Terminating connection...\n";
            send(client_socket, response.c_str(), response.length(), 0);
            close(client_socket);
            return;
        }
    }
}
