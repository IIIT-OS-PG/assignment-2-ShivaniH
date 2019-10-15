#include <iostream>
#include <fstream>
#include <string>

using namespace std;

char* putFileInBuffer(char *buffer, long long int *fileSizePtr ,string filename);

void putBufferInAFile(char *buffer, long long int fileSize, string fileName);