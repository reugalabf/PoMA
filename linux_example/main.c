/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "poma_tcpconnector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int GlobalVar = 0;

void setterGlobalVar(WRITERFUNC, char *argument)
{
    if (argument != NULL)
        GlobalVar = atoi(argument);
    writer("done\n", 6);
}

void getterGlobalVar(WRITERFUNC, char *argument)
{
    char response[10];
    sprintf(response, "%d\n", GlobalVar);
    writer(response, strlen(response));
}

int main(int argc, char *argv[])
{
    // int sockfd, portno;
    // socklen_t clilen;

    // struct sockaddr_in serv_addr, cli_addr;
    PoMA_TCP_SPEC *tcpSpec = malloc(sizeof(PoMA_TCP_SPEC));

    Topic *topicHead;
    topicHead = createTopic("GlobalVar", getterGlobalVar, setterGlobalVar);
    addTopic(topicHead, createTopic("g_var", getterGlobalVar, setterGlobalVar));

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    tcpSpec = createPoMATCPConnectSpec(tcpSpec, atoi(argv[1]), SINGLE_USER); //or MULTI_USER
    
    tcpSpec->processClientsLoop(tcpSpec, topicHead);
    
    return 0;
}
