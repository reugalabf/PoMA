#include <sys/socket.h>
#include <netinet/in.h>

#include "poma_core.h"

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int poma_sockfd;

int  tcpwriter(const void * response, size_t rsp_size)
{
    write(poma_sockfd,(char *)response, rsp_size);
    return 1;
}

//int processMessage(WRITERFUNC, char * buffer, Topic* topics);
/*
int processMessage(WRITERFUNC, char *buffer, Topic *topics)
{
    int n;
    //    printf("Here is the message of %zu length: %s\n",strlen(buffer), buffer);
    n = writer( "ACK: ", strlen("ACK: "));
    switch (buffer[0])
    {
    case '?':
        processGetterMessage(writer, &buffer[1], topics);
        break;
    case '=':
        processSetterMessage(writer, &buffer[1], topics);
        break;
    case '*':
        processListTopics( writer, &buffer[1], topics);
        break;
    default:
        writer( AVAILABLE_COMMANDS, strlen(AVAILABLE_COMMANDS));
    }
    return n;
}
*/

void processMessagesLoop(Topic *topicsHead){
unsigned char status=1;
ssize_t n;
char buffer[256];
while( status > 0 )
    {
        bzero(buffer,256);
        n = read(poma_sockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
                if (n < 0)
        {
            status = 0;
            printf("status: %d \n", status);
            error("ERROR writing to socket");
        }
        if (strlen(buffer) == 1 )
        {
            status = 0;
            //printf("--status: %d \n", status);
        }
        else
        {
            processMessage(&tcpwriter, buffer, topicsHead);

        }
    }
    close(poma_sockfd);


}