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

#define bigValue 1000000     // Intentionally reduced value to get rid of bad address error
#define MAX_NUM_THREADS 10
#define maxSHA 655360

using namespace std;

pthread_t threads[MAX_NUM_THREADS];

/**
 * Data structs for tracking data
 */

map<string, vector<string>> fileToPeerMap;  //which peers have which files
map<string, vector<string>> peerToGroupMap;     //which groups a particular peer is present in
//make another map for usernames and passwords?


/*----------------------------------------------------------------------------------------------------------------------------------------
                                   |       THREAD FOR HANDLING REQUESTS FROM PEERS        |
----------------------------------------------------------------------------------------------------------------------------------------*/

void *serviceRequests(void* socketFD)
{
    int *sockFDPTR = (int*) socketFD;
    int sockFD = *sockFDPTR;
    char requestBuffer[2048];   //2KB size buffer for storing requests from peers(clients)
    
    memcpy(requestBuffer, receiveData(2048, sockFD), 2048);     //Receive request from peer

    string command = string(requestBuffer);

    if(command == "create_user")
    {

    }
    else if(command == "login")
    {

    }
    else if(command == "create_group​")
    {

    }
    else if(command == "join_group​")
    {

    }
    else if(command == "leave_group")
    {

    }
    else if(command == "list_requests")
    {

    }
    else if(command == "accept_request​")
    {

    }
    else if(command == "list_groups")
    {

    }
    else if(command == "list_files")
    {

    }
    else if(command == "upload_file")
    {
        char fileName[100], fileSHA[maxSHA], peerIP[16], groupID[20], peerPort[10], fileSize[30];

        memcpy(fileName, receiveData(100, sockFD), 100);

    }
    else if(command == "download_file​")
    {

    }
    else if(command == "Show_downloads")
    {

    }
    else if(command == "stop_share")
    {

    }
    else {
       cout<<"Invalid command received by tracker!\n";
       exit(1);
    }

    pthread_exit(NULL);

}

/*----------------------------------------------------------------------------------------------------------------------------------------
                                            |       MAIN        |
----------------------------------------------------------------------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    
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

    /**
     * Get details of a file from a peer(client) -- Peer shares a file
     */

    char *fileDetails = (char*)malloc(sizeof(char)*bigValue);
    //char fileDetails[bigValue];

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

        int *peerHandlingSocketPTR = &peerHandlingSocket;

        if ( pthread_create(&threads[i], NULL, serviceRequests, (void*)peerHandlingSocketPTR) < 0 )
        {
            perror("From tracker: Couldn't create a thread for handling request");
            exit(1);
        }

        //TODO: How to QUIT?

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
