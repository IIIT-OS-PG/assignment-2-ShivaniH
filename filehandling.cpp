#include "filehandling.hpp"

char* putFileInBuffer(char *buffer, long long int *fileSizePtr, string filename)
{
    FILE *ptr = fopen((const char*)filename.c_str(),"r");

    if(ptr == NULL) 
    {
        fputs ("File error",stderr);
        exit(1);
    }
    
    //get file size
    fseek(ptr , 0 , SEEK_END);
    long long int fileSize = ftell (ptr);
    rewind(ptr);

    *fileSizePtr = fileSize;

    buffer = (char*) malloc (sizeof(char)*fileSize);

    // copy the file into the buffer
    int result = fread(buffer, 1, fileSize, ptr);

    //cout<<"contents of buffer : \n"<<buffer;
    //cout<<"\n";

    if (result != fileSize) 
    {
        fputs("Reading error",stderr); 
        exit(1);
    }

    fclose(ptr);
    return buffer;
}