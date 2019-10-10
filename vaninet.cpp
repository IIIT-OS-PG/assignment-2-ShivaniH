#include <algorithm>
#include <vector>
#include <iostream>
#include <map>

#include "vaninet.hpp"

using namespace std;

/**
 * SETTING UP SOCKET
 */

pair<int, struct sockaddr_in> setUpSocket(const char *IP, int port)
{
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &(address.sin_addr));

    if(port <= 1024)
    {
        perror("Cannot use a port number <= 1024 \n");
        perror("Assigning another port . . . ");
        port = 0;   //So that the system selects the lowest unused port automatically
    }
    address.sin_port = htons(port);

    int socketRet = socket(AF_INET, SOCK_STREAM, 0);
    if(socketRet == -1)
    {
        perror("Could not create socket!\n");
        exit(1);
    }

    int bindSuccess = bind(socketRet, (const sockaddr *)&address, sizeof(sockaddr_in));
    if(bindSuccess < -1)
    {
        perror("Could not bind socket to specified address!\n");
        exit(1);
    }

    pair<int, struct sockaddr_in> retPair = make_pair(socketRet, address);
    return retPair;
}

/** 
 * SEND DATA
 */

int sendData(char *buffer, long long int sendLength, int sendersSocket)
{
    //cout<<"send length is "<<sendLength<<"\n";
    char *ptrToBuffer = buffer;

    //cout<<"sending these bytes \n"<<buffer<<"\n";
    long long int bytesSent;
    while(sendLength > 0)
    {
        bytesSent = send(sendersSocket, ptrToBuffer, sizeof(ptrToBuffer), 0);
        if(bytesSent < 1)
        {
            return(-1);  
        }
        //cout<<ptrToBuffer<<" bytes sent\n";
        //cout<<bytesSent<<" bytes were sent\n";
        ptrToBuffer += bytesSent;
        sendLength -= bytesSent; 
    }

    return 0;
}

/**
 * RECEIVE DATA
 */ 

char* receiveData(long long int bufferSize, int receiversSocket)
{
    char *bufferStorage = (char*)malloc(sizeof(char)*bufferSize);
    //char bufferStorage[bufferSize];
    long long int bytesRecvd = 1;     //just an initial value
    long long int totalBytes = 0;

    char *buffer;
    buffer = bufferStorage;

    while(bytesRecvd > 0 && totalBytes <= bufferSize)
    {
        bytesRecvd = TEMP_FAILURE_RETRY(recv(receiversSocket, bufferStorage, 8, 0));
        if(bytesRecvd == -1)
        {
            perror("Error while receiving data ");
            exit(1);
        }
        bufferStorage += bytesRecvd;
        //cout<<"got "<<bytesRecvd<<" bytes\n";
        totalBytes += bytesRecvd;
        //cout<<"received these bytes: "<<bufferStorage<<"\n";
    }

    return buffer;
}
 
