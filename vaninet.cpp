#include <algorithm>
#include <vector>
#include <iostream>
#include <map>

#include "vaninet.hpp"

using namespace std;

void dump( string s)
{
  for (unsigned int n=0; n<s.length(); ++n)
  {
    char c = s[n];
    cout << (int) c << ",";
  }
  cout << endl;

}

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
        perror("Cannot use a port number <= 1024 ");
        perror("Assigning another port . . . ");
        port = 0;   //So that the system selects the lowest unused port automatically
    }
    address.sin_port = htons(port);

    int socketRet = socket(AF_INET, SOCK_STREAM, 0);
    if(socketRet == -1)
    {
        perror("Could not create socket!");
        exit(1);
    }

    int bindSuccess = bind(socketRet, (const sockaddr *)&address, sizeof(sockaddr_in));
    if(bindSuccess < -1)
    {
        perror("Could not bind socket to specified address!");
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

    //cout << "VC: " << buffer << "\n";

    //PKB
    send(sendersSocket, &sendLength, sizeof(long long int), 0);

    //cout<<"sending these bytes \n"<<buffer<<"\n";
    long long int bytesSent;
    while(sendLength > 0)
    {
        bytesSent = send(sendersSocket, ptrToBuffer, sendLength, 0);
        //cout << bytesSent << "\n";
        if(bytesSent < 1)
        {
            return(-1);  
        }
        //cout<<ptrToBuffer<<" bytes sent\n";
        //cout<<bytesSent<<" bytes were sent\n";
        ptrToBuffer += bytesSent;
        sendLength -= bytesSent; 
    }
    //cout << "VC Final\n";
    return 0;
}

/**
 * RECEIVE DATA
 */ 

char* receiveData(long long int bufferSize, int receiversSocket)
{
    //PKB
    recv(receiversSocket, &bufferSize, sizeof(long long int), 0);
    //cout << "PKB: " << bufferSize << "\n";
    
    char *bufferStorage = (char*)malloc(sizeof(char)*bufferSize);
    //char bufferStorage[bufferSize];
    long long int bytesRecvd = 1;     //just an initial value
    long long int totalBytes = 0;

    char *buffer;
    buffer = bufferStorage;

    while(bytesRecvd > 0 && totalBytes < bufferSize)
    {
        bytesRecvd = recv(receiversSocket, bufferStorage, bufferSize, 0);
        //cout << "bytes received = " << bytesRecvd << "\n";
        if(bytesRecvd == -1)
        {
            perror("Error while receiving data ");
            exit(1);
        }
        bufferStorage += bytesRecvd;
        //cout<<"got "<<bytesRecvd<<" bytes\n";
        totalBytes += bytesRecvd;
        //cout << "TB: " << totalBytes << "\n";
        //cout<<"received these bytes: "<<buffer<<"\n";
        //cout<<"dump for receiver buffer: \n";
        //dump(string(buffer));
    }
    //cout << "VC: " << buffer << "\n";
    return buffer;
}
 
