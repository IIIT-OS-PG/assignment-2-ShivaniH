/*-------------------------------------
| Standard header files              |
-------------------------------------*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <openssl/sha.h>
#include <cstring>

/*-------------------------------------
|   Vani-chan's header files          |
-------------------------------------*/

#include "../vaninet.hpp"      //POSIX headers are in here
#include "../filehandling.hpp"

#define _512KB 524288
#define maxSHA 655360   // max number of chars in hash (max chunks = 32768), considering max file size = 16 GB --> Or will it be 8GB, cause of the *2 done later?
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

    fin.close();

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
     * Main user command input loop
    /*

    /*
    string userInput = "";
    cin>>userInput;
    while(userInput != "logout")
    {
        //use switch to check which command matches the start of the userInput -- how to match?
        cin>>userInput;
    }

    */

    /**
     * Talking to Tracker
     */ 

    char *buffer;
    long long int fileSize = 0;
    long long int *ptrToFileSize = &fileSize;

    //buffer = putFileInBuffer(buffer, ptrToFileSize, "countOfMonteCristo.txt");    //You'll need to put the file in a buffer only when sending it, not to calculate SHA1

    //cout<<"file size is "<<fileSize<<"\n";
    //cout<<"contents of file : \n"<<buffer;
    //cout<<"\n";

    /**
     * Calculate SHA1
     */

    FILE *ptr = fopen("CountOfMonteCristo.txt","r");

    if(ptr == NULL) 
    {
        fputs ("File error",stderr);
        exit(1);
    }

    fseek(ptr , 0 , SEEK_END);
    fileSize = ftell(ptr);
    rewind(ptr);

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

    int shaIndex = 0;

    while(numRemaining > 0)
    {
        memset(fiveTwelveBuffer, 0, _512KB);
        bytesRead = fread(fiveTwelveBuffer, 1, _512KB, ptr);
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
        sha1MD[shaIndex]= '\0';
        //printf("hash collected till now %02x \n", ptrToSHA1MD);
        //memcpy(sha1MD, hashOfChunk, 20);
        //ptrToSHA1MD += 20;
        numRemaining -= _512KB;
    }

    fclose(ptr);

    printf("SHA1 for the file is ");
    for(int i = 0; i < shaIndex; ++i)
    {
        //printf("%02x",sha1MD[i]);
        cout<<sha1MD[i];
    }
    printf("\n");
    //cout<<"SHA1 for the file is "<<(sha1MD & 0xFF)<<"\n";
    
    /*
    UNCOMMENT THIS WHEN NEEDED*/
    cout<<"Going to send data now...\n";
    cout<<"Peer ip = "<<peerIP<<" peer port = "<<ntohs(peerAddress.sin_port)<<"\n";
    
    if(sendData((char*)sha1MD, shaIndex, peerSocket) == -1)
    {
        perror("Couldn't send any data!"); 
        exit(1);
    }
   /* */
            
    close(peerSocket);
    return 0;
}