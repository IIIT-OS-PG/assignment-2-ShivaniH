/*-------------------------------------
| Standard header files              |
-------------------------------------*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <string.h>

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
     * Data structs for tracking data
     */

    map<string, vector<string>> fileToPeerMap;  //which peers have which files
    map<string, vector<string>> peerToGroupMap;     //which groups a particular peer is present in
    //make another map for usernames and passwords?


    /**
     * Setting up tracker
     */

    const char *trackerInfoFile = argv[1];
    string trackerNum = argv[2];

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

    fin.close();

    cout<<"Read tracker info. . . \n";
    cout<<"trackerNum is "<<trackerNum<<"\n";
    if(trackerNum == "1")
    {
        trackersIP = trackerInfo[0].first.c_str();
        trackersPort = stoi(trackerInfo[0].second);
    }
    else {
        trackersIP = trackerInfo[1].first.c_str();
        trackersPort = stoi(trackerInfo[1].second);
    }

    cout<<"Extracted info from tracker_info...\n";

    int trackerSocket, peerHandlingSocket;

    trackerSocket = setUpSocket(trackersIP, trackersPort).first;
    //fcntl(trackerSocket, F_SETFL, O_NONBLOCK);    <-- This is to make the socket non-blocking

    cout<<"Finished setting up tracker socket...\n";

    if(listen(trackerSocket, 10) == -1)
    {
        perror("Couldn't listen on tracker port");
    }

    struct sockaddr_storage incomingAddress;
    socklen_t incomingAddrSize = sizeof(incomingAddress);

    /**
     * The tracker now sits and waits to service peers(clients)
     */

    char trackerBuffer[2048];   //2KB size buffer for storing requests from peers(clients)
    int bufferSize = 2048;

    /**
     * Get details of a file from a peer(client) -- Peer shares a file
     */

    long long int bigValue = numeric_limits<long long int>::max();
    char *fileDetails = (char*)malloc(sizeof(char)*bigValue);

    while(true)
    {
        cout<<"Tracker waiting for connections . . . \n";
        cout<<"tracker IP = "<<trackersIP<<" tracker port = "<<trackersPort<<"\n";
        peerHandlingSocket = accept(trackerSocket, (struct sockaddr*)&incomingAddress, (socklen_t *)&incomingAddrSize);
        if(peerHandlingSocket == -1)
        {
            perror("Couldn't get a new socket descriptor for handling new request");
            exit(1);
        }

        //TODO: Make a separate thread here to receive and handle requests

        //The buffersize that you send to the receiveData function depends on what kind of request you're handling

        /*
        UNCOMMENT THIS LATER
        cout<<"Going to receive data now...\n";

        memcpy(fileDetails, receiveData(bigValue, peerHandlingSocket), bigValue);

        cout<<"Got from client: "<<fileDetails<<"\n";
        */
    }

    close(trackerSocket);
    return 0;
}
