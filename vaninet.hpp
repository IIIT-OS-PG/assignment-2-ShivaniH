#include<iostream>
#include <cstring>

/*-------------------------------------
| POSIX header files              |
-------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  //has getaddrinfo()
#include <arpa/inet.h>  //for inet_pton

/*-------------------------------------
|   Vani-chan's header files          |
-------------------------------------*/

std::pair<int, struct sockaddr_in> setUpSocket(const char *trackersIP, int trackerNum);

int sendData(char *buffer, long long int bufferSize, int sendersSocket);

char* receiveData(long long int bufferSize, int receiversSocket);