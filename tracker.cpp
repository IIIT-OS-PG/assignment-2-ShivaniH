/*-------------------------------------
| Standard header files              |
-------------------------------------*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>

/*******

/*-------------------------------------
| POSIX header files              |
-------------------------------------
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  //has getaddrinfo()
#include <arpa/inet.h>  //for inet_pton

*******/

/*-------------------------------------
|   Vani-chan's header files          |
-------------------------------------*/

#include "vaninet.hpp"

using namespace std;

/*----------------------------------------------------------------------------------------------------------------------------------------
                                            |       MAIN        |
----------------------------------------------------------------------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    map<string, vector<string>> fileToPeerMap;  //which peers have which files
    map<string, vector<string>> peerToGroupMap;     //which groups a particular peer is present in
    //make another map for usernames and passwords?

    const char *trackerInfoFile = argv[1];
    char* trackerNum = argv[2];

    const char *trackersIP;
    unsigned int trackersPort;

    vector<pair<string, string>> trackerInfo;

    ifstream fin;  string line;
    fin.open(trackerInfoFile);

    int i = 0;
    while(fin)
    {
        getline(fin, line);
        int posOfSpace = line.find(" ");
        trackerInfo[i] = make_pair(line.substr(0,posOfSpace), line.substr(posOfSpace+1));   //there are posOfSpace characters before the space
        ++i;
    }

    if(trackerNum == "1")
    {
        trackersIP = trackerInfo[0].first.c_str();
        trackersPort = stoi(trackerInfo[0].second);
    }
    else {
        trackersIP = trackerInfo[1].first.c_str();
        trackersPort = stoi(trackerInfo[1].second);
    }

    int trackerSocket, peerHandlingSocket;

    trackerSocket = setUpSocket(trackersIP, trackersPort);

    if(listen(trackerSocket, 10) == -1)
    {
        perror("Couldn't listen on tracker port\n");
    }

    struct sockaddr_storage incomingAddress;
    socklen_t incomingAddrSize = sizeof(incomingAddress);

    while(true)
    {
        peerHandlingSocket = accept(trackerSocket, (struct sockaddr*)&incomingAddress, &incomingAddrSize);
        if(peerHandlingSocket == -1)
        {
            perror("Couldn't get a new socket descriptor for handling new request\n");
        }
        cout<<"Waiting for connections . . . \n";
    }

    return 0;
}
