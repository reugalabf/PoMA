//#include <sys/socket.h>
//#include <netinet/in.h>

//#include "poma_core.h"

#include "poma_tcpconnector.h"



static int ref_sockfd=-1;

static void error(char *msg)
{
    perror(msg);
    exit(1);
}

static int tcpwriter(const void *response, size_t rsp_size)
{
    assert(ref_sockfd >=0); 
    write(ref_sockfd, (char *)response, rsp_size);
    return 1;
}

void processMessagesLoop(PoMA_TCP_SPEC *spec, Topic *topicsHead)
{
    unsigned char status = 1;
    ssize_t n;
    char buffer[256];
    while (status > 0)
    {
        bzero(buffer, 256);
        n = read(spec->session_sockfd, buffer, 255);
            
        if (n < 0)
        {
            error("ERROR reading from socket");
            status = 0;
        }

        if (n == 0)
        {
            status = 0;
            //printf("--status: %d \n", status);
        }
        else
        {
            //printf("processMessage: buffer %s \n", buffer);
            printf(".");
            fflush(stdout);
            processMessage(&tcpwriter, buffer, topicsHead);
        }
    }
    printf("\n");
    close(spec->session_sockfd);
}

static void tcpClientsLoopHandler(PoMA_TCP_SPEC *spec, Topic *topicsHead)
{
    socklen_t clilen;
    
    do
    {
        //printf("Start processMessagesLoop \n");
        clilen =  sizeof(&spec->cli_addr);
        spec->session_sockfd = accept(spec->server_sockfd, (struct sockaddr *)&spec->cli_addr, &clilen);
        if (spec->session_sockfd < 0)
            error("ERROR on accept");
    
        ref_sockfd = spec->session_sockfd;
        processMessagesLoop(spec, topicsHead);
        //printf("After processMessagesLoop\n");
    } while (spec->running != 0);
    printf("Bye and Good night! \n");
}

PoMA_TCP_SPEC *createPoMATCPConnectSpec(PoMA_TCP_SPEC *spec, int portno, int multiuser)
{

    int sockfd;

    
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP); // IPPROTO_TCP
    

    if (sockfd < 0)
        error("ERROR opening socket");
    spec->server_sockfd = sockfd;
    bzero((char *)&(spec->serv_addr), sizeof(spec->serv_addr));
    spec->port = portno;
    spec->serv_addr.sin_family = AF_INET;
    spec->serv_addr.sin_addr.s_addr = INADDR_ANY;
    spec->serv_addr.sin_port = htons(portno);
    
    if (bind(spec->server_sockfd, (struct sockaddr *)&(spec->serv_addr),
             sizeof(spec->serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);

    printf("Socket bound. Waiting on port %d... \n", spec->port);
    spec->multi_user = multiuser;
    spec->running = 1;
    spec->processClientsLoop = tcpClientsLoopHandler;
    return spec;
}