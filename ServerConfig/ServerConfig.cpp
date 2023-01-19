#include <iostream>
#include <winsock2.h>
#include <string>
#include <Ws2tcpip.h>

#define SERVER
class comms
{
public:
    // Initialize connection
    virtual void init() = 0;
    // Send data
    virtual void sendData(char* buffer) = 0;
    // Receive data
    virtual char* recvData() = 0;
    // Close connection
    virtual void closeConnection() = 0;

protected:
    // Winsock data
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server;
    sockaddr_in client;
    int clientSize;
};

class serverComms : public comms
{
public:
    SOCKET clientSock;
    void init() override
    {
        // Initialize Winsock
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            throw std::runtime_error("WSAStartup has failed: " + std::to_string(iResult));
        }

        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Socket creation has failed: " + std::to_string(WSAGetLastError()));
        }

        // Fill in server information
        server.sin_family = AF_INET;
        iResult = inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr));
        if (iResult <= 0) {
            throw std::runtime_error("inet_pton could not be established: " + std::to_string(WSAGetLastError()));
        }
        server.sin_port = htons(55555);

        // Bind socket to server
        iResult = bind(sock, (sockaddr*)&server, sizeof(server));
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("Binding to server has failed: " + std::to_string(WSAGetLastError()));
        }

        // Listen for incoming connections
        iResult = listen(sock, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("Listen for connection has failed: " + std::to_string(WSAGetLastError()));
        }
    }

    char* recvData() override
    {
        // Accept a client socket
        clientSize = sizeof(client);
        SOCKET clientSock = accept(sock, (sockaddr*)&client, &clientSize);
        if (clientSock == INVALID_SOCKET) {
            throw std::runtime_error("Accepting connection failed: " + std::to_string(WSAGetLastError()));
        }

        // Receive data from the client
        char buffer[256];
        int iResult = recv(clientSock, buffer, 256, 0);
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("Receiving connection failed: " + std::to_string(WSAGetLastError()));
        }

        return buffer;
    }

    void sendData(char* buffer) override
    {
        // Send data to the client
        int iResult = send(clientSock, buffer, (int)strlen(buffer), 0);
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("Sending failed: " + std::to_string(WSAGetLastError()));
        }
    }

    void closeConnection() override
    {
        // Close the client socket and cleanup Winsock
        closesocket(clientSock);
        WSACleanup();
    }
};

class clientComms : public comms
{
public:
    SOCKET clientSock; 
    void init() override
    {
        // Initialize Winsock
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            throw std::runtime_error("WSAStartup has failed: " + std::to_string(iResult));
        }

        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Socket creation has failed: " + std::to_string(WSAGetLastError()));
        }

        // Fill in server information
        server.sin_family = AF_INET;
        iResult = inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr));
        if (iResult <= 0) {
            throw std::runtime_error("inet_pton could not be established: " + std::to_string(WSAGetLastError()));
        }
        server.sin_port = htons(55555);

        // Connect to server
        iResult = connect(sock, (sockaddr*)&server, sizeof(server));
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("Connect tto server has failed: " + std::to_string(WSAGetLastError()));
        }
    }

    char* recvData() override
    {
        // Receive data from the server
        char buffer[256];
        int iResult = recv(sock, buffer, 256, 0);
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("Receiving failed: " + std::to_string(WSAGetLastError()));
        }
        return buffer;
    }

    void sendData(char* buffer) override
    {
        // Send data to the server
        int iResult = send(sock, buffer, (int)strlen(buffer), 0);
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("Sending failure: " + std::to_string(WSAGetLastError()));
        }
    }

    void closeConnection() override
    {
        // Close the socket and cleanup Winsock
        closesocket(sock);
        WSACleanup();
    }
};

int main()
{
#ifdef SERVER
    serverComms server;
    server.init();
    char* msg;
    while (1)
    {
        msg = server.recvData();
        std::cout << "Client: " << msg << std::endl;
        server.sendData(msg);
    }
    server.closeConnection();
#else
    clientComms client;
    client.init();
    char message[256];
    bool exit = false;
    while (!exit)
    {
        //prompt the user to enter a message
        std::cout << "Enter a message to send to the server or type 'exit' to quit: ";
        std::cin.getline(message, 256);
        if (strcmp(message, "exit") == 0)
        {
            exit = true;
            break;
        }
        // send message to the server
        client.sendData(message);
        // receive response from the server
        std::cout << "Server Received: " << client.recvData() << std::endl;
    }
    client.closeConnection();
#endif
    return 0;
}
