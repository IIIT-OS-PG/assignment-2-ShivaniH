/*-------------------------------------
| Standard header files              |
-------------------------------------*/

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <openssl/sha.h>
#include <cstdlib>
#include <cmath>

/*-------------------------------------
|   Vani-chan's header files          |
-------------------------------------*/

#include "../vaninet.hpp"      //POSIX headers are in here
#include "../filehandling.hpp"

#define _512KB 524288
#define maxSHA 655360   // max number of chars in hash (max chunks = 32768), considering max file size = 16 GB --> Or will it be 8GB, cause of the *2 done later?
#define MAX_NUM_THREADS 80
#define MAX_CHUNKS 16384    // 8 GB / 512 KB

using namespace std;

/*------------------------------------------------
|             Some Data Structures               |
-------------------------------------------------*/

string userInput, command;
vector<string> params(3);

pthread_mutex_t protectServerConn;

pthread_mutex_t protectChunk;

//pthread_t threads[MAX_NUM_THREADS];

/*----------------------------------------------------------------------------------------------------
|                              STRUCTS TO SEND DATA TO THREAD ROUTINES                               |
----------------------------------------------------------------------------------------------------*/
struct shareFileData {
    char file_path[256];
    char group_id[256];
    int socketFD;
    char *peerIP;
    char *peerPort;
};

struct peerServerData {
    char *peerIP;
    char *peerPort;
};

struct chunkWriterData {
    char *sourceFile;
    char *destFile;
    long long int sourceFileSize;
    char *peerServerIP;
    int peerServerPort;
    int *chunksNumsToGet = (int*) malloc(sizeof(int)*16384);
};

/**
 * Extract command and parameters
 * 
 */

void getCommandParams()
{
    int space;
    space = userInput.find(' ');
    //cout<<"space pos = "<<space<<"\n";
    command = userInput.substr(0, space);
    userInput.erase(0, space+1);

    //cout<<"userinput modified to "<<userInput<<"\n";

    int i = 0;
    while(userInput.size() != 0 && i < 3)
    {
        space = userInput.find(' ');
        params[i] = userInput.substr(0, space);
        //cout<<"dump for params, index =  "<<i<<"\n";
        // dump(params[i]);
        params[i] += '\0';
        userInput.erase(0, space+1);
        //cout<<"userinput modified to "<<userInput<<"\n";
        ++i;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
                                            |       FUNCTIONS FOR CLIENT-LIKE BEHAVIOUR OF THE PEER       |
----------------------------------------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------
|                              SHARE FILE                              |
----------------------------------------------------------------------*/

void *shareFile(void* shareData)
{   
    struct shareFileData *reqData = (struct shareFileData*) shareData;

    char *filePath = reqData->file_path;
    //cout<<"size of filepath ptr = "<<strlen(filePath)<<"\n";
    char *groupID = reqData->group_id;
    int sockFD = reqData->socketFD;
    char *peerIP = reqData->peerIP;
    char *peerPort = reqData->peerPort;

    //Tell the tracker that you want to share a file

    char buffy[12] = "upload_file";
    if( sendData(buffy, 12, sockFD) == -1)
    {
        perror("Couldn't send command name!"); 
        exit(1);
    }
    
    if( sendData(filePath, strlen(filePath), sockFD) == -1)  //send its path/name
    {
        perror("Couldn't send any file name!"); 
        exit(1);
    }

    /**
     * Calculate SHA1
     */

    long long int fileSize = 0;
    FILE *ptr = fopen(filePath,"r");

    cout<<"\nNow trying to open "<<filePath<<"\n";

    if(ptr == NULL) 
    {
        fputs ("File error",stderr);
        exit(1);
    }

    fseek(ptr , 0 , SEEK_END);
    fileSize = ftell(ptr);
    rewind(ptr);

    string fileSizeString;
    fileSizeString = to_string(fileSize);

    if( sendData((char*)fileSizeString.c_str(), 30, sockFD) == -1 )     //send file size
    {
        perror("Couldn't send any file size!"); 
        exit(1);
    }

    if( sendData(peerIP, 16, sockFD) == -1 )   // send peer IP
    {
        perror("Couldn't send peer IP to tracker!"); 
        exit(1);
    }

    if( sendData(peerPort, 10, sockFD)  == -1)     // send peer port
    {
        perror("Couldn't send peer port to tracker"); 
        exit(1);
    }

    //cout<<"unsigned char size  = "<<sizeof(unsigned char)<<"\n";
 
    //char *fiveTwelveBuffer = (char*)malloc(sizeof(char)*_512KB);
    //unsigned char *hashOfChunk = (unsigned char*)malloc(sizeof(unsigned char)*20);
    char fiveTwelveBuffer[_512KB];
    unsigned char hashOfChunk[SHA_DIGEST_LENGTH*2];
    long long int numRemaining = fileSize;
    //unsigned char* sha1MD = (unsigned char*)malloc(sizeof(unsigned char)*maxSHA);
    unsigned char sha1MD[maxSHA];

    //unsigned char* ptrToSHA1MD = sha1MD;
    long int bytesRead;

    /*
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    */

    memset(fiveTwelveBuffer, 0, _512KB);
    memset(hashOfChunk, 0, 40);
    memset(sha1MD, 0, maxSHA);

    long long int shaIndex = 0;

    while(numRemaining > 0)
    {
        memset(fiveTwelveBuffer, 0, _512KB);
        bytesRead = fread(fiveTwelveBuffer, 1, _512KB, ptr);
        cout << "VC: " << bytesRead << "\n\n";
        if(bytesRead != _512KB)
        {
            SHA1((unsigned char *)fiveTwelveBuffer, bytesRead, hashOfChunk);
        }
        else SHA1((unsigned char *)fiveTwelveBuffer, _512KB, hashOfChunk);
        printf("Hash of chunk is : ");
        for(int i = 0; i < SHA_DIGEST_LENGTH; ++i, shaIndex+=2)
        {
            printf("%02x",hashOfChunk[i]);
            //sha1MD[shaIndex] = hashOfChunk[i];
            sprintf( (char*)&sha1MD[shaIndex], "%02x",hashOfChunk[i]);      // 02x --> To write 2 nibbles in one byte
        }
        printf("\n");
        //printf("hash collected till now %02x \n", ptrToSHA1MD);
        //memcpy(sha1MD, hashOfChunk, 20);
        //ptrToSHA1MD += 20;
        numRemaining -= _512KB;
    }

    sha1MD[shaIndex]= '\0';
    ++shaIndex;

    fclose(ptr);

    printf("SHA1 for the file is ");
    for(int i = 0; i < shaIndex; ++i)
    {
        //printf("%02x",sha1MD[i]);
        cout<<sha1MD[i];
    }
    printf("\n\n");

    cout<<"\n\nGoing to send the SHA1 hash to the tracker now...\n";

    //cout<<"shaIndex = "<<shaIndex<<"\n";
    
    if(sendData((char*)sha1MD, maxSHA, sockFD) == -1)
    {
        perror("Couldn't send SHA1 to tracker!"); 
        exit(1);
    }

    close(sockFD);

    void *exitStatus = 0;

    pthread_exit(exitStatus);
}

/*-------------------------------------------------------------------------
|                              CHUNK WRITER                              |
-------------------------------------------------------------------------*/


void *getChunkAndStore(void* chunkWritingData)
{
    struct chunkWriterData *CWD = (struct chunkWriterData*) chunkWritingData;

    char *sourceFile = CWD->sourceFile;
    long long int sourceFileSize = CWD->sourceFileSize;
    char *destFile = CWD->destFile;

    // No one else should mess with the address of the peer whom I've to contact!!

    pthread_mutex_lock(&protectServerConn);

    char *peerServerIP = CWD->peerServerIP;
    //int peerServerPort = CWD->peerServerPort;
    int chunkNumsToGet[MAX_CHUNKS];
    cout<<"\n\nI'm the thread connecting to "<<CWD->peerServerPort<<" : I've to get chunk numbers: \n";
    
    //cout<<"CWD->chunksNumsToGet[0] = "<<CWD->chunksNumsToGet[0]<<"\n";
    memcpy(chunkNumsToGet, CWD->chunksNumsToGet, MAX_CHUNKS);

    for(int i = 0; chunkNumsToGet[i]!= '\0'; ++i)
    {
        cout<<"thread connected to server "<<CWD->peerServerPort<<" : "<<chunkNumsToGet[i]<<"\n";
    }
    
    int chunkWriterSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(chunkWriterSocket == -1)
    {
        perror("Could not create socket!");
        exit(1);
    }

    struct sockaddr_in peerServerAddress;

    peerServerAddress.sin_family = AF_INET;
    if(inet_pton(AF_INET, peerServerIP, &(peerServerAddress.sin_addr)) )
    peerServerAddress.sin_port = htons(CWD->peerServerPort);

    if(connect(chunkWriterSocket, (const sockaddr *)&peerServerAddress, sizeof(peerServerAddress)) < 0)   //try to send connection request
    {
        perror("Could not connect to peer server");
        exit(1);
    }

    cout<<"Connected to peer server, whose IP is "<<peerServerIP<<" and whose port is "<<CWD->peerServerPort<<"\n";

    pthread_mutex_unlock(&protectServerConn);

    // Receive those chunks whose number is passed to you
    // Right now, work under the assumption that there's only one thread, and that it gets all chunks from a peer who has the complete file

    char requestType[50];

    memcpy(requestType, "fileChunk", 10);
    sendData(requestType, 50, chunkWriterSocket);

    sendData(sourceFile, 100, chunkWriterSocket);

    string chunkString;
    int i = 0;
    //cout<<"chunkNumsToGet[0] = "<<chunkNumsToGet[i]<<"\n";
    while (chunkNumsToGet[i] != '\0')
    {
        chunkString += to_string(chunkNumsToGet[i]);
        chunkString += " ";
        ++i;
    }

    sendData((char*)chunkString.c_str(), MAX_CHUNKS*3, chunkWriterSocket);

    FILE *fptr = fopen(destFile, "r+");

    //Use a loop to write chunks, use seek to find correct place to put chunk

    i = 0;
    while (chunkNumsToGet[i] != '\0')
    {
        //send chunk number to peer-server

        // Don't mess with which chunk I've to receive!! 
        pthread_mutex_lock(&protectChunk);

        char chunkNumber[6];
        string cNString = to_string(chunkNumsToGet[i]);
        memcpy(chunkNumber, cNString.c_str(), cNString.length());
        sendData(chunkNumber, 6, chunkWriterSocket);

        //receive chunk

        char theChunk[_512KB];
        memset(theChunk, '\0', _512KB);
        memcpy(theChunk, receiveData(_512KB, chunkWriterSocket), _512KB);

        cout<<"Received chunk "<<chunkNumber<<"\n";

        pthread_mutex_unlock(&protectChunk);

        //cout<<"Contents of this chunk: \n";
        //cout<<theChunk<<"\n";

        // Write it to file
        fseek(fptr, (atoi(chunkNumber)-1)*_512KB, SEEK_SET);

        //cout<<"PKB: ftell tells you : "<<ftell(fptr)<<"\n";

        long long int numBytesToWrite;
        if(((atoi(chunkNumber)-1)*_512KB) + _512KB > sourceFileSize)
        {
            numBytesToWrite = sourceFileSize - ((atoi(chunkNumber)-1)*_512KB);
        }
        else {
            numBytesToWrite = _512KB;
        }

        int numBytesWritten = fwrite(theChunk, 1, numBytesToWrite, fptr);
        if(ferror(fptr))
        {
            cout<<"Here's the problem\n";
        }
        else
        {
            cout<<"No problem writing the chunk\n";
        }
        
        cout<<"\nWrote "<<numBytesWritten<<" bytes to the file\n";

        ++i;
    }
    
    fclose(fptr);

    void *exitStatus = 0;

    pthread_exit(exitStatus);
}


/*----------------------------------------------------------------------------------------------------------------------------------------
                                            |       FUNCTIONS FOR SERVER-LIKE BEHAVIOUR OF THE PEER       |
----------------------------------------------------------------------------------------------------------------------------------------*/

void *servicePeerRequests(void* socket)
{
    cout<<"\n\nPeer service provider now began running. . . \n";
    int *socketFD = (int*)socket;
    int sockFD = *socketFD;

    char requestType[50];

    memcpy(requestType, receiveData(50, sockFD), 50);

    cout<<"Request type is "<<requestType<<"\n"; 

    if(strncmp(requestType, "fileChunk", 9) == 0)
    {
        /*-----------------------------------------------------------------------------
        |                              SEND FILE CONTENTS                             |
        ------------------------------------------------------------------------------*/
        //cout<<"Reached inside if, just about to send chunks of file\n";
        char fileName[100];
        memcpy(fileName, receiveData(100, sockFD), 100);

        cout<<"Peer server received file name: "<<fileName<<"\n";

        char chunkNumber[6];
        char chunkString[MAX_CHUNKS*3];

        memcpy(chunkString, receiveData(MAX_CHUNKS*3, sockFD), MAX_CHUNKS*3);

        string chunkCPPString = string(chunkString);

        cout<<"CPPchunk string is "<<chunkCPPString<<"\n\n";
        int numChunks = count(chunkCPPString.begin(), chunkCPPString.end(), ' ');

        FILE *fp = fopen(fileName, "r");
        cout<<"Opened file "<<fileName<<"\n";

        for(int i = 0; i < numChunks; ++i)
        {
            memcpy(chunkNumber, receiveData(6, sockFD), 6);     //Can extract it from chunkCPPString too. . . 
            long int startByte = (atoi(chunkNumber)-1)*_512KB;
            cout<<"startByte is "<<startByte<<"\n";

            if( fseek(fp, startByte, 0) != 0 )
            {
                perror("Could not seek a particular chunk in the file");
            }

            char theChunk[_512KB];
            memset(theChunk, '\0', _512KB);

            int numBytesRead = fread(theChunk, 1, _512KB, fp);
            cout<<"Read "<<numBytesRead<<" bytes\n";

            sendData(theChunk, _512KB, sockFD);

            cout<<"Sent chunk "<<chunkNumber<<"\n\n";
        }    

        fclose(fp);
    }

    void *exitStatus = 0;

    pthread_exit(exitStatus);
}

/*---------------------------------------------------------------------
|                              PEER SERVER                              |
----------------------------------------------------------------------*/

void *peerServer(void *serverData)
{
    int serverSocket;
    struct peerServerData *reqData = (struct peerServerData*) serverData;
    const char *serverIP = reqData->peerIP;
    int serverPort = atoi(reqData->peerPort)+1;

    serverSocket = setUpSocket(serverIP, serverPort).first;

    cout<<"\n\nFinished setting up peer server socket...\n";

    if(listen(serverSocket, 10) == -1)
    {
        perror("Couldn't listen on peer server port");
    }

    struct sockaddr_storage incomingAddress;
    socklen_t incomingAddrSize = sizeof(incomingAddress);

    int peerHandlingSocket;

    while(true)
    {
        cout<<"\n\nPeer server waiting for connections . . . \n";
        cout<<"Peer server IP = "<<serverIP<<" peer server port = "<<serverPort<<"\n";
        peerHandlingSocket = accept(serverSocket, (struct sockaddr*)&incomingAddress, (socklen_t *)&incomingAddrSize);
        if(peerHandlingSocket == -1)
        {
            perror("From peer: Couldn't get a new socket descriptor for handling new request");
            exit(1);
        }

        int *peerHandlingSocketPTR = &peerHandlingSocket;

        //pthread_t peerServerRequestHandlers[MAX_NUM_THREADS];

        pthread_t serverMultiThreadID;

        if ( pthread_create(&serverMultiThreadID, NULL, servicePeerRequests, (void*)peerHandlingSocketPTR) < 0 )
        {
            perror("From Peer: Couldn't create a thread for handling request");
            exit(1);
        }

        pthread_detach(serverMultiThreadID);

        //DETACH THIS THREAD!
    }
}

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
    int peerPort = stoi(peerIPPort.substr(posOfColon+1));

    /** 
     * Setting up stuff to connect to a tracker
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

    fin.close();

    cout<<"Read tracker info. . . \n";

    trackersIP = trackerInfo[0].first.c_str();
    trackersPort = stoi(trackerInfo[0].second);

    struct sockaddr_in trackerAddress;

    trackerAddress.sin_family = AF_INET;
    if(inet_pton(AF_INET, trackersIP, &(trackerAddress.sin_addr)) )
    trackerAddress.sin_port = htons(trackersPort);

    /**
     * Setting up a peer-server thread to handle requests from other peers
     */ 

    struct peerServerData *PSD;
    PSD = (struct peerServerData*)malloc(sizeof(struct peerServerData));
    PSD->peerIP = (char*)peerIP;
    PSD->peerPort = (char*)to_string(peerPort).c_str();
    pthread_t serverThreadID;

    //pthread_mutex_lock(&threadPoolLock);

    //DETACH THIS THREAD!!
    if ( pthread_create(&serverThreadID, NULL, peerServer, (void*)PSD) < 0 )
    {
        perror("Couldn't create a thread to run peer server");
        exit(1);
    }

    pthread_detach(serverThreadID);

    //pthread_mutex_unlock(&threadPoolLock);


    /**
     * ///////////////////////////////--------------------------------------- MAIN USER COMMAND INPUT LOOP ------------------------------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    */

    userInput = "";

    struct shareFileData *SFD;




    //cin>>userInput;
    //cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, userInput);
    //cout<<"user input is "<<userInput<<"\n";
    while(userInput != "logout")
    {
        // Connect to either one of the trackers, before every request

        pair<int, struct sockaddr_in> socketPair = setUpSocket(peerIP, peerPort);       // A NEW SOCKET DESCRIPTOR FOR EVERY THREAD
        int peerSocket = socketPair.first;
        int *peerSocketPTR = &peerSocket;

        struct sockaddr_in peerAddress = socketPair.second;

        cout<<"Finished setting up a new peer socket...\n";

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

        getCommandParams();

        //cout<<"command is "<<command<<"\n";

        /*
        cout<<"dump for command : \n";
        dump(command);
        cout<<"dump for download file : \n";
        dump("download_file​");
        */

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
            /*----------------------------------------------------------------------------------------------------
            |                                               SHARE FILE                                        |
            ----------------------------------------------------------------------------------------------------*/
            string filePath = params[0];
            string groupID = params[1];

            //cout<<"param0 = "<<filePath<<"\n";
            //cout<<"param1 = "<<params[1]<<"\n";
            SFD = (struct shareFileData*)malloc(sizeof(struct shareFileData));
            strcpy(SFD->file_path, filePath.c_str());
            strcpy(SFD->group_id, groupID.c_str());
            SFD->peerIP = (char*)peerIP;
            SFD->peerPort = (char*)to_string(peerPort).c_str();
            SFD->socketFD = peerSocket;

            //cout<<"OK TILL HERE\n";

            pthread_t fileSharingThreadID;

            if ( pthread_create(&fileSharingThreadID, NULL, shareFile, (void*)SFD) < 0 )
            {
                perror("Couldn't create a thread to share file");
                exit(1);
            }

        }
        else if(command == "download_file")
        {

            /*----------------------------------------------------------------------------------------------------
            |                                               DOWNLOAD FILE                                        |
            ----------------------------------------------------------------------------------------------------*/

            string groupID = params[0];
            string fileName = params[1];
            string destPath = params[2];

            // Ask tracker for a list of peers who have the file -- send fileName to tracker

            char bufferCommand[14] = "download_file";
            if( sendData(bufferCommand, 14, peerSocket) == -1 )
            {
                perror("Couldn't send command name!"); 
                exit(1);
            }
          
            //cout<<"dump for filename in peer:\n";
            //dump(fileName);

            if( sendData((char*)fileName.c_str(), 100, peerSocket) == -1)
            {
                perror("Couldn't send file name!"); 
                exit(1);
            }

            char fileSHA[maxSHA], fileSize[30], peerInfo[520];
            memset(fileSHA, '\0', maxSHA);
            memset(fileSize, '\0', 30);
            memset(peerInfo, '\0', 520);

            //cout<<"VC : "<<"fileSHA : "<<fileSHA<<"\n";
            memcpy(fileSHA, receiveData(maxSHA, peerSocket), maxSHA);
            cout<<"fileSHA = "<<fileSHA<<"\n"; 
            //cout<<"Waiting for file size now\n";
            memcpy(fileSize, receiveData(30, peerSocket), 30);
            cout<<"fileSize = "<<fileSize<<"\n";
            memcpy(peerInfo, receiveData(520, peerSocket), 520);
            cout<<"peerInfo = "<<peerInfo<<"\n";

            vector<pair<string,int>> peersWhoOwnFile;

            string peerInfoStr = string(peerInfo);

            int numPeers = count(peerInfoStr.begin(), peerInfoStr.end(), ':');

            int posSpace, posColon, port;
            string singlePeer, IP;
            //cout<<"Peer info in vector: \n";
            for(int i = 0; i < numPeers; ++i)
            {
                posSpace = peerInfoStr.find(' ');
                singlePeer = peerInfoStr.substr(0, posSpace);
                posColon = singlePeer.find(':');
                IP = singlePeer.substr(0,posColon);
                port = stoi(singlePeer.substr(posColon+1));
                peerInfoStr.erase(0, posSpace+1);
                peersWhoOwnFile.push_back(make_pair(IP, port));
                //cout<<peersWhoOwnFile[i].first<<" "<<peersWhoOwnFile[i].second<<"\n";
            }

            long long int fileSizeValue = stoll(string(fileSize));
            //cout<<"File size = "<<fileSizeValue<<" and numChunks without ceil = "<<(float)fileSizeValue/(float)_512KB<<"\n";
            int numChunks = int(ceil((float)fileSizeValue/(float)_512KB));
            cout<<"number of chunks to be downloaded = "<<numChunks<<"\n";

            // CREATING A FILE FILLED WITH NULLS, TO STORE THE FILE THAT WILL BE RECEIVED ----- NOPE!!!!!

            //string stringOfNulls = string(fileSizeValue, '\0');

            //putBufferInAFile((char*)stringOfNulls.c_str(), fileSizeValue, destPath);

            // CREATING THREADS TO DOWNLOAD CHUNKS

            pthread_t chunkWritingThreads[MAX_NUM_THREADS];

            float workPerThread = numChunks/numPeers;
            int intWorkPerThread = (int)(floor(workPerThread));
            int extraWork = 0;
            if(workPerThread - intWorkPerThread != 0)
            {
                extraWork = 1;
            }
            else {
                extraWork = 0;
            }

            int startChunk = 1;

            cout<<"number of peers = "<<numPeers<<"\n";
            // For now, assume that numPeers <= MAX_NUM_THREADS

            pthread_mutex_init(&protectServerConn, NULL);
            pthread_mutex_init(&protectChunk, NULL);

            FILE *destinationFile = fopen(destPath.c_str(), "w");

            fclose(destinationFile);

            for(int i = 0; i < numPeers; ++i)
            {
                int * chunkNumbers = (int *) calloc(MAX_CHUNKS, sizeof(int));

                struct chunkWriterData *CWD = (struct chunkWriterData *)malloc(sizeof(struct chunkWriterData));

                CWD->sourceFile = (char*)fileName.c_str();
                CWD->sourceFileSize = fileSizeValue;
                CWD->destFile = (char*)destPath.c_str();
                CWD->peerServerIP = (char*)peersWhoOwnFile[i].first.c_str();
                cout<<"Value of peer server port read = "<<peersWhoOwnFile[i].second<<"\n";
                CWD->peerServerPort = peersWhoOwnFile[i].second+1;        //The peer-server's port = peer's port + 1
                
                memset(chunkNumbers, '\0', MAX_CHUNKS);
                //For now, since you know there's just one peer, put all chunk numbers for just this one thread

                int j;
                for(j = 0; j < workPerThread; ++j, ++startChunk)
                {
                    chunkNumbers[j] = startChunk;
                    //cout<<"chunkNumbers "<<j<<" : "<<chunkNumbers[j]<<"\n";
                    //cout<<"PKB : Verifying chunk numbers: "<<chunkNumbers[j]<<"\n";
                }

                if(i == numPeers - 1 && extraWork)
                {
                    cout<<"Extra work for thread "<<i+1<<"\n";
                    chunkNumbers[j] = startChunk;
                }

                CWD->chunksNumsToGet = chunkNumbers;    //working??
                /*
                cout<<"Thread "<<i+1<<"\n";
                for(j = 0; j < workPerThread; ++j)
                {
                    cout<<CWD->chunksNumsToGet[j]<<"\n";
                    //cout<<"chunkNumbers "<<j<<" : "<<chunkNumbers[j]<<"\n";
                }
                */

                pthread_create(&chunkWritingThreads[i], NULL, getChunkAndStore, (void*)CWD);
                //sleep(1);

                cout<<"\n\nCreated thread "<<i+1<<"from main\n";
            }

            for(int i = 0; i < numPeers; ++i)
            {
                pthread_join(chunkWritingThreads[i], NULL);
            }

            cout<<"\n\n"<<destPath<<" has been downloaded\n";

            //VERIFY ITS INTEGRITY!!
        }
        else if(command == "Show_downloads")
        {

        }
        else if(command == "stop_share")
        {

        }
        else {
            cout<<"Invalid command entered!\n";
            cout<<"Command entered acc to me is "<<command<<"\n";
        }


        cin>>userInput;

    }   //end of user input loop

        
    /***
     
    cout<<"Going to send data now...\n";
    cout<<"Peer ip = "<<peerIP<<" peer port = "<<ntohs(peerAddress.sin_port)<<"\n";
    
    if(sendData((char*), , peerSocket) == -1)
    {
        perror("Couldn't send any data!"); 
        exit(1);
    }

    ***/
    pthread_mutex_destroy(&protectServerConn);
    free(SFD);
    return 0;
}