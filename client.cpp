#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <iostream>

#define MAX_MESSAGE_LENGTH 127

int sSocket;

void *readServer(void *arg);
void *sendToServer(void *arg);

int main()
{
    sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sSocket == -1)
    {
        perror("Failed to create socket");
        return -1;
    }
    printf("Socket created successfully\n");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(4400);

    int r = connect(sSocket, (struct sockaddr *)&addr, sizeof addr);
    if (r == -1)
    {
        perror("Failed to connect to server");
        return -1;
    }
    printf("Connected to server successfully\n");

    pthread_t readThread, sendThread;
    pthread_create(&readThread, NULL, readServer, NULL);
    pthread_create(&sendThread, NULL, sendToServer, NULL);

    pthread_join(readThread, NULL);
    pthread_join(sendThread, NULL);

    close(sSocket);
    return 0;
}
void *readServer(void *arg)
{
    char message[MAX_MESSAGE_LENGTH + 1];
    while (1)
    {
        int r = recv(sSocket, message, MAX_MESSAGE_LENGTH, 0);
        if (r > 0)
        {
            message[r] = '\0';
            // std::cout << "Is peer value is :  " << strcmp(message,"peer") << "\n";
            printf("%s\n", message);
            if (strcmp(message, "peer") == 10)
            {
                std::cout << "Started a new thread with client : " << message << "\n";
                // perror();
            }
        }
        else
        {
            perror("Failed to receive message from server");
            break;
        }
    }
    return NULL;
}
void *sendToServer(void *arg)
{
    char message[MAX_MESSAGE_LENGTH + 1];
    while (1)
    {
        // printf("\nEnter Message > ");
        fgets(message, MAX_MESSAGE_LENGTH + 1, stdin);
        int len = strlen(message);
        if (len > 0 && message[len - 1] == '\n')
        {
            message[len - 1] = '\0';
        }

        if (len > 1)
        { // Check if the message has more than just a newline character
            if (send(sSocket, message, strlen(message), MSG_NOSIGNAL) == -1)
            {
                perror("Failed to send message to server");
                break;
            }
        }
    }
    return NULL;
}
