/*-------------------------------------
| Standard header files              |
-------------------------------------*/

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>

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

map<string, vector<pair<string,string>>> fileToPeerMap;  //which peers have which files
map<string, vector<string>> fileToFileInfoMap;
map<vector<string>, vector<string>> peerToGroupMap;     //which groups a particular peer is present in
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
        //cout<<"fileName = "<<fileName<<"\n";
        memcpy(fileSize, receiveData(30, sockFD), 30);
        //cout<<"fileSize = "<<fileSize<<"\n";
        memcpy(peerIP, receiveData(16, sockFD), 16);
        //cout<<"peerIP = "<<peerIP<<"\n";
        memcpy(peerPort, receiveData(10, sockFD), 10);
        //cout<<"peerPort = "<<peerPort<<"\n";
        memcpy(fileSHA, receiveData(maxSHA, sockFD), maxSHA);
        //cout<<"fileSHA = "<<fileSHA<<"\n";
        //memcpy(groupID, receiveData(100, sockFD), 100);

        cout<<"Got file name : "<<fileName<<"\n";
        cout<<"Got file size : "<<fileSize<<"\n";
        cout<<"Got peerIP : "<<peerIP<<"\n";
        cout<<"Got peerPort : "<<peerPort<<"\n";
        cout<<"Got file SHA : "<<fileSHA<<"\n\n";

        vector<pair<string, string>> peerData;
        vector<string> fileData;

        pair<string, string> onePeer;
        onePeer.first = peerIP;
        onePeer.second = peerPort;

        fileData.push_back(fileSize);
        fileData.push_back(fileSHA);

        if(fileToPeerMap.find(string(fileName)) == fileToPeerMap.end())
        {
            peerData.push_back(onePeer);
            fileToPeerMap.insert(make_pair(string(fileName), peerData));
        }
        else {
            peerData = fileToPeerMap[string(fileName)];
            /*
            cout<<"Old peer data = ";
            for(int k = 0; k < peerData.size(); ++k)
            {
                cout<<peerData[k].first<<" "<<peerData[k].second<<"\n";
            }
            */
            peerData.push_back(onePeer);
            /*
            cout<<"New peer data = ";
            for(int k = 0; k < peerData.size(); ++k)
            {
                cout<<peerData[k].first<<" "<<peerData[k].second<<"\n";
            }
            */
            fileToPeerMap[string(fileName)] = peerData;
        }
        fileToFileInfoMap.insert(make_pair(string(fileName), fileData));
        //cout<<"inserted into fileinfo map\n";


    }
    else if(command == "download_file")
    {

        // peer wants a list of other peers who have a particular file
        char fileName[100];

        memset(fileName, '\0', 100);
        memcpy(fileName, receiveData(100, sockFD), 100);

        //cout<<"dump for filename in tracker:\n";
        //dump(fileName);

        string fileNameStr = string(fileName);
        //cout<<"map size is "<<fileToFileInfoMap.size()<<"\n";
        map<string, vector<string>>::iterator it = fileToFileInfoMap.begin();
        //cout<<it->first<<"\n";

        /*
        cout<<fileToPeerMap.size()<<" peers have the requested file\n";
        cout<<"List of peers who have "<<fileNameStr<<" is : \n";

        for(int i = 0; i < fileToPeerMap.size(); ++i)
        {
            cout<<fileToPeerMap[fileNameStr][i].first<<" "<<fileToPeerMap[fileNameStr][i].second<<"\n";
        }
        */

        //cout<<"received file is = "<<fileNameStr<<"\n";

        if(fileToFileInfoMap.find(fileNameStr) != fileToFileInfoMap.end())
        {
            vector<string> fileData = fileToFileInfoMap[fileNameStr];
            vector<pair<string,string>> peerData = fileToPeerMap[fileNameStr];

            char listOfPeers[520];  // assuming a maximum of 20 peers
            int numPeers = peerData.size();

            char *insertPos = listOfPeers;
            int numBytes;
            for(int i = 0; i < numPeers; ++i)
            {
                numBytes = peerData[i].first.length();
                memcpy(insertPos, peerData[i].first.c_str(), numBytes);
                insertPos += numBytes;

                *insertPos = ':';
                ++insertPos;

                numBytes = peerData[i].second.length();
                memcpy(insertPos, peerData[i].second.c_str(), numBytes);
                insertPos += numBytes;

                *insertPos = ' ';
                ++insertPos;
            }

            char sendSHA[maxSHA], sendFileSize[30];

            memset(sendSHA, '\0', maxSHA);
            memset(sendFileSize, '\0', 30);

            memcpy(sendSHA, fileData[1].c_str(), fileData[1].length());
            memcpy(sendFileSize, fileData[0].c_str(), fileData[0].length());
            //cout<<"VC : "<<fileData[1]<<"\n";
            //cout<<"length = "<<fileData[1].length()<<"\n";
            if( sendData(sendSHA, maxSHA, sockFD) == -1)
            {
                perror("Couldn't send file SHA!");
                exit(1);
            }

            cout<<"\n\nSent fileSHA to peer\n";

            usleep(500);

            if( sendData(sendFileSize, 30, sockFD) == -1)
            {
                perror("Couldn't send file size!");
                exit(1);
            }

            cout<<"Sent filesize to peer\n";

            if( sendData(listOfPeers, 520, sockFD) == -1)
            {
                perror("Couldn't send list of peers!");
                exit(1);
            }

            cout<<"sent peer list to requesting peer\n\n";
        }
        else{
            cout<<"The requested file has not been shared by any peer\n";
        }

    }
    else if(command == "Show_downloads")
    {

    }
    else if(command == "stop_share")
    {

    }
    else {
       cout<<"Invalid command received by tracker!\n";
       cout<<"Tracker got command = "<<command<<"\n";
       exit(1);
    }

    cout<<"Tracker's request handling thread has finished\n";
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

    //char *fileDetails = (char*)malloc(sizeof(char)*bigValue);
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

        if ( pthread_create(&threads[0], NULL, serviceRequests, (void*)peerHandlingSocketPTR) < 0 )
        {
            perror("From tracker: Couldn't create a thread for handling request");
            exit(1);
        }
        //pthread_join(threads[0], NULL);

        //TODO: How to QUIT?

        //DONE: Make a separate thread here to receive and handle requests

        //The buffersize that you send to the receiveData function depends on what kind of request you're handling
    }

    //free(fileDetails);
    close(trackerSocket);
    return 0;
}
