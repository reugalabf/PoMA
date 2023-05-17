/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "poma.hpp"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


void error(const char* msg)
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
int main(int argc, char **argv)
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    unsigned char status = 1;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    
    Poma *board = new Poma();
    PomaSocketListener *connector = new PomaSocketListener(board);

    board->registerTopic("GlobalVar", &getterGlobalVar, &setterGlobalVar) ;
    board->registerTopic("g_var", &getterGlobalVar, &setterGlobalVar) ;


    
    
    if (argc < 2)
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    portno = atoi(argv[1]);
    if (  portno ==0) 
        error("invalid port number");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    //bzero((char *) &serv_addr, sizeof(serv_addr));
    connector->socket_desc = sockfd;
    connector->start(portno);
    
    return 0;
}