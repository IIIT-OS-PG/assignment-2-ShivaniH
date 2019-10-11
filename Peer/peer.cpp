/*-------------------------------------
| Standard header files              |
-------------------------------------*/

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <openssl/sha.h>
#include <cstdlib>

/*-------------------------------------
|   Vani-chan's header files          |
-------------------------------------*/

#include "../vaninet.hpp"      //POSIX headers are in here
#include "../filehandling.hpp"

#define _512KB 524288
#define maxSHA 655360   // max number of chars in hash (max chunks = 32768), considering max file size = 16 GB --> Or will it be 8GB, cause of the *2 done later?
#define MAX_NUM_THREADS 10

using namespace std;

/*------------------------------------------------
|             Some Data Structures               |
-------------------------------------------------*/

string userInput, command;
vector<string> params(3);

pthread_t threads[MAX_NUM_THREADS];

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

/**
 * Extract command and parameters
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
        userInput.erase(0, space+1);
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

    cout<<"Now trying to open "<<filePath<<"\n";

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

    if( sendData((char*)fileSizeString.c_str(), fileSizeString.length(), sockFD) == -1 )     //send file size
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
        cout << "VC: " << bytesRead << "\n";
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
            sprintf( (char*)&sha1MD[shaIndex], "%02x",hashOfChunk[i]);
        }
        printf("\n");
        //printf("hash collected till now %02x \n", ptrToSHA1MD);
        //memcpy(sha1MD, hashOfChunk, 20);
        //ptrToSHA1MD += 20;
        numRemaining -= _512KB;
    }

    sha1MD[shaIndex]= '\0';
    ++shaIndex;

    //fclose(ptr);

    printf("SHA1 for the file is ");
    for(int i = 0; i < shaIndex; ++i)
    {
        //printf("%02x",sha1MD[i]);
        cout<<sha1MD[i];
    }
    printf("\n");

    cout<<"Going to send the SHA1 hash to the tracker now...\n";

    cout<<"shaIndex = "<<shaIndex<<"\n";
    
    if(sendData((char*)sha1MD, maxSHA, sockFD) == -1)
    {
        perror("Couldn't send SHA1 to tracker!"); 
        exit(1);
    }

    close(sockFD);

    void *exitStatus = 0;

    pthread_exit(exitStatus);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
                                            |       FUNCTIONS FOR SERVER-LIKE BEHAVIOUR OF THE PEER       |
----------------------------------------------------------------------------------------------------------------------------------------*/

void *sendFile()
{
    char *buffer;
    long long int fileSize = 0;
    long long int *ptrToFileSize = &fileSize;

    buffer = putFileInBuffer(buffer, ptrToFileSize, "countOfMonteCristo.txt");    //You'll need to put the file in a buffer only when sending it, not to calculate SHA1

    //cout<<"file size is "<<fileSize<<"\n";
    //cout<<"contents of file : \n"<<buffer;
    //cout<<"\n";

    void *exitStatus = 0;
    pthread_exit(exitStatus);
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

    fin.close();

    cout<<"Read tracker info. . . \n";

    trackersIP = trackerInfo[0].first.c_str();
    trackersPort = stoi(trackerInfo[0].second);

    struct sockaddr_in trackerAddress;

    trackerAddress.sin_family = AF_INET;
    if(inet_pton(AF_INET, trackersIP, &(trackerAddress.sin_addr)) )
    trackerAddress.sin_port = htons(trackersPort);

    /**
     * ///////////////////////////////--------------------------------------- MAIN USER COMMAND INPUT LOOP ------------------------------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    */

    userInput = "";

    struct shareFileData *SFD;


    //cin>>userInput;
    //cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, userInput);
    cout<<"user input is "<<userInput<<"\n";
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

        cout<<"command is "<<command<<"\n";

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
            string filePath = params[0];
            string groupID = params[1];

            cout<<"param0 = "<<filePath<<"\n";
            cout<<"param1 = "<<params[1]<<"\n";
            SFD = (struct shareFileData*)malloc(sizeof(struct shareFileData));
            strcpy(SFD->file_path, filePath.c_str());
            strcpy(SFD->group_id, groupID.c_str());
            SFD->peerIP = (char*)peerIP;
            SFD->peerPort = (char*)to_string(peerPort).c_str();
            SFD->socketFD = peerSocket;

            cout<<"OK TILL HERE\n";

            if ( pthread_create(&threads[i], NULL, shareFile, (void*)SFD) < 0 )
            {
                perror("Couldn't create a thread to share file");
                exit(1);
            }
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
            cout<<"Invalid command entered!\n";
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

    free(SFD);
    return 0;
}