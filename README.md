**Project Title: P2P Communication System and Broadcast**

**Introduction**

This project is a Peer-to-Peer (P2P) communication system that allows clients to communicate with each other directly without involving the server in the message exchange process. The server is responsible for managing client connections, authentication, sharing active client information, and broadcasting messages on behalf of a client. The application also keeps a record of registered users in a CSV file, and user passwords are hashed using SHA-256 for security purpose.

**Features**

1. User authentication through login or signup.
2. Password hashing using SHA-256.
3. List of active clients.
4. Broadcast messages to all connected clients.
5. Establish a peer-to-peer connection to chat with other users.

**Prerequisites**

- A C++ compiler (such as g++).
- OpenSSL library for password hashing.

**Execution steps**

**How to Compile and Run the Server**

1. Compile the server code using the following command:

**g++ -o server server.cpp -lssl -lcrypto -lpthread**

1. Run the server using the following command:

**./server**

1. The server will start listening for connections on port 4400.
2. Compile the client code using the following command:

**g++ -o client client.cpp**

1. Run the server using the following command:

**./client**

1. Follow the prompts and proceed with the execution

**Usage**

1. Compile the code using a C++ compiler.
2. Run the compiled server application on a computer.
3. Run the compiled client application on one or more computers.
4. when the server code is compiled it will start the server on port 4400.
5. when the client code is compiled the client will have 2 options 1 for login and 2 for signup.
6. Client can either signup or login, if the client selects option 1 then the client will be asked to enter username and password 
7. If the client selects option 2 then the client need to give a unique username and unique password and re-enter the password for confirmation
8. the clients data will be saved in a csv file called "usercreds.csv" in the directory which will be used for client validation
9. After successful login, client will have 4 options 1. to list the active clients connected to the server 2.to broadcast a message 3.to send a peer to peer message to the client who is connected to the server 
10. If the client selects option 1, then the list of active clients connected to the server will be displayed in the following format: <br>
     <br>Client 1: 127.0.0.1:35596<br>
     Client 2: 126.2.2.2:45678<br>
     .                        <br>
     .                        <br>
     Client n: .....          <br>
<br>
11. If the client selects the option2, then the user will be prompted to enter message that need to be broadcasted, when the client types the message and hit enter then the message will be broadcasted to all the clients connected to the server and active within 30 mintues. <br>
12. Clients can send messages between themselves without the server <br>
13. If clients want to send peer to peer messages then the client need to type the message in the following command <br>
    <br>"msg_Message-to-be-sent_IP-addr&PORT" <br>
  Ex: msg_HEllo-world_127.0.0.1:35596 <br>
    <br>the IP and port number of the client can be taken from the list of clients ( option 1 )  <br> <br>

**Code Explanation**

The code consists of the following main parts:

1. Libraries: Standard C++ libraries are included for handling sockets, threads, and other necessary functionality.
2. Global Variables and Constants: Constants for the maximum number of connections and buffer size are defined, along with global variables to manage clients and threads.
3. Main Functions:
  - **hash\_password** : Function to hash a given password using the SHA-256 algorithm.
  - **accept\_clients** : Function to accept incoming client connections and handle their authentication.
  - **main** : Main function that sets up the server socket and starts the threads for accepting clients.
  - **handle\_new\_client** : Function to handle each connected client, providing them with the available options to interact with other users.
4. Server Setup:
  - Create a TCP socket.
  - Bind the socket to the desired address and port.
  - Listen for incoming connections.
5. Client Handling:
  - Authenticate users through login or signup.
  - Provide options for listing active clients, broadcasting messages, and establishing peer-to-peer connections.

**Screenshots**

**1. Client Login/Signup:**

**Signup:**

![signup.png](https://github.com/vamshinilagiri2/Peer-to-peer-messaging-broadcast-messaging/blob/main/screenshots/signup.png?raw=true)

**Login:**

![login.png](https://github.com/vamshinilagiri2/Peer-to-peer-messaging-broadcast-messaging/blob/main/screenshots/login.png?raw=true)

**2. List of Active Clients:**

![list.png](https://github.com/vamshinilagiri2/Peer-to-peer-messaging-broadcast-messaging/blob/main/screenshots/client-list.png?raw=true)

**3. Broadcasting a Message:**

**3.1 Client sending broadcast message :**

![broadcast-1.png](https://github.com/vamshinilagiri2/Peer-to-peer-messaging-broadcast-messaging/blob/main/screenshots/broadcast-1.png?raw=true)

**3.2 other client receiving broadcasted message :**

![broadcast-2.png](https://github.com/vamshinilagiri2/Peer-to-peer-messaging-broadcast-messaging/blob/main/screenshots/broadcast-2.png?raw=true)

**4. Establishing a Peer-to-Peer Connection:**

**4.1: Client sending a p2p message using the command:**

Command:

**msg\_"your-message"\_"IP-and-port-from-list-of-client"**

Example:

1. **msg\_CNproject\_127.0.0.1:35596**

**2. msg\_this-message-is-sent-through-p2p-communication\_127.0.0.1:35596**

![p2p-1.png](https://github.com/vamshinilagiri2/Peer-to-peer-messaging-broadcast-messaging/blob/main/screenshots/p2p-1.png?raw=true)

**4.2** Client receiving the p2p message:

![p2p-2.png](https://github.com/vamshinilagiri2/Peer-to-peer-messaging-broadcast-messaging/blob/main/screenshots/p2p-2.png?raw=true)
