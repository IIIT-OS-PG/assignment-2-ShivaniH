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
#include <pthread.h>

/*-------------------------------------
|   Vani-chan's header files          |
-------------------------------------*/

using namespace std;

std::pair<int, struct sockaddr_in> setUpSocket(const char *IP, int port);

int sendData(char *buffer, long long int bufferSize, int sendersSocket);

char* receiveData(long long int bufferSize, int receiversSocket);

void dump(string s);