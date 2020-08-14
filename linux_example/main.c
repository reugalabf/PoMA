/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "poma.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

//#include "poma.h"

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int GlobalVar=0;

void setterGlobalVar(int sockfd, char *argument)
{
    GlobalVar = atoi(argument);
    write(sockfd,"done\n",6);
}

void getterGlobalVar(int sockfd, char* argument)
{
    char response[10];
    sprintf(response,"%d\n", GlobalVar);
    write(sockfd,response,strlen(response));

}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    unsigned char status = 1;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    Topic *topicHead;
    topicHead = createTopic("GlobalVar", getterGlobalVar, setterGlobalVar);
    addTopic(topicHead, createTopic("g_var", getterGlobalVar, setterGlobalVar));

    if (argc < 2)
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    printf("Socket bound. Waiting on port %d... \n", portno);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    printf("Client Connected.\n");

    while( status > 0 )
    {
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
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
            processMessage(newsockfd, buffer, topicHead);

        }
    }
    close(newsockfd);
    return 0;
}
