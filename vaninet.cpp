#include "vaninet.hpp"


int setUpSocket(const char *IP, int port)
{
    struct sockaddr_in serverAddress;

    serverAddress.sin_family = AF_UNSPEC;
    inet_pton(AF_INET, IP, &(serverAddress.sin_addr));

    if(port <= 1024)
    {
        perror("Cannot use a port number <= 1024 \n");
        perror("Assigning another port . . . ");
        port = 0;   //So that the system selects the lowest unused port automatically
    }
    serverAddress.sin_port = htons(port);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1)
    {
        perror("Could not create server socket!\n");
        exit(1);
    }

    int bindSuccess = bind(serverSocket, (const sockaddr *)&serverAddress, sizeof(sockaddr_in));
    if(bindSuccess < -1)
    {
        perror("Could not bind socket to specified address!\n");
        exit(1);
    }

    return serverSocket;
}