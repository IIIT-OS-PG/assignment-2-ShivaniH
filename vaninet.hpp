#include<iostream>

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


int setUpSocket(const char *trackersIP, int trackerNum);