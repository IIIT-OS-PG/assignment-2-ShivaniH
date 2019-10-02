/*-------------------------------------
| Standard header files              |
-------------------------------------*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>

/*-------------------------------------
|   Vani-chan's header files          |
-------------------------------------*/

#include "../vaninet.hpp"      //POSIX headers are in here

using namespace std;

/*----------------------------------------------------------------------------------------------------------------------------------------
                                            |       MAIN        |
----------------------------------------------------------------------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    /**
     * Setting up the peer
     */

    string peerIPPort = argv[1];
    int posOfColon = peerIPPort.find(":");

    const char* peerIP = peerIPPort.substr(0,posOfColon).c_str();
    unsigned int peerPort = stoi(peerIPPort.substr(posOfColon+1));

    pair<int, struct sockaddr_in> socketPair = setUpSocket(peerIP, peerPort);
    int peerSocket = socketPair.first;
    struct sockaddr_in peerAddress = socketPair.second;

    cout<<"Finished setting up peer socket...\n";

    /** 
     * Setting up connection to a tracker
     */

    const char *trackerInfoFile = argv[2];

    const char *trackersIP;
    unsigned int trackersPort;

    vector<pair<string, string>> trackerInfo;

    ifstream fin;  string line;
    fin.open(trackerInfoFile);

    cout<<"Opened tracker info. . . \n";
    int i = 0;
    while(fin)
    {
        getline(fin, line);
        int posOfSpace = line.find(" ");
        string s1 = line.substr(0,posOfSpace);
        string s2 = line.substr(posOfSpace+1);
        trackerInfo.push_back(make_pair(s1, s2));   //there are posOfSpace characters before the space
        ++i;
    }

    cout<<"Read tracker info. . . \n";

    trackersIP = trackerInfo[0].first.c_str();
    trackersPort = stoi(trackerInfo[0].second);

    struct sockaddr_in trackerAddress;

    trackerAddress.sin_family = AF_INET;
    if(inet_pton(AF_INET, trackersIP, &(trackerAddress.sin_addr)) )
    trackerAddress.sin_port = htons(trackersPort);

    if(connect(peerSocket, (const sockaddr *)&trackerAddress, sizeof(trackerAddress)) < 0)   //try to send connection request to tracker 1
    {
        cout<<"Connection with tracker 1 faiiiiled\n";
        //tracker 1 down, so contact tracker 2
        trackersIP = trackerInfo[1].first.c_str();
        trackersPort = stoi(trackerInfo[1].second);

        inet_pton(AF_INET, trackersIP, &(trackerAddress.sin_addr));
        trackerAddress.sin_port = htons(trackersPort);

        int shouldCertainlyConnect = connect(peerSocket, (const sockaddr *)&trackerAddress, sizeof(trackerAddress));
        if(shouldCertainlyConnect < 0)
        {
            perror("Both trackers down!");
            cout<<"Connection with any tracker faiiiiled\n";
            exit(1);
        }
        else {
        cout<<"Connected to tracker 2\n";
        cout<<"tracker IP = "<<trackersIP<<" tracker port = "<<trackersPort<<"\n";
    }
    }
    else {
        cout<<"Connected to tracker 1\n";
        cout<<"tracker IP = "<<trackersIP<<" tracker port = "<<trackersPort<<"\n";
    }

    /**
     * Talking to Tracker
     */ 

    char *ptr = (char*)malloc(sizeof(char)*16);
    ptr = "peer says meow\n"; 

    cout<<"Going to send data now...\n";
    cout<<"Peer ip = "<<peerIP<<" peer port = "<<ntohs(peerAddress.sin_port)<<"\n";
    
    int sendLength = 16;
    int bytesSent;
    while(sendLength > 0)
    {
        cout<<"got in here\n";
        bytesSent = send(peerSocket, ptr, sizeof(ptr), 0);
        if(bytesSent < 1)
        {
            perror("Couldn't send any data!");   
        }
        cout<<bytesSent<<" amount of data sent to tracker\n";
        ptr += bytesSent;
        sendLength -= bytesSent; 
    }
            
    close(peerSocket);
    return 0;
}