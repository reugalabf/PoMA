#include <sys/socket.h>
#include <netinet/in.h>

#include "poma_core.h"

void error(char *msg)
{
    perror(msg);
    exit(1);
}

typedef struct PoMA_TCP_SPEC
{
    int running;
    int session_sockfd;
    int server_sockfd;
    int port;
    int multi_user;
    struct sockaddr_in serv_addr; 
    struct sockaddr_in cli_addr;
    void (*processClientsLoop) (struct PoMA_TCP_SPEC *spec,Topic *topicsHead);
} PoMA_TCP_SPEC;

int ref_sockfd;

int  tcpwriter(void * response, size_t rsp_size)
{
    write(ref_sockfd,(char *)response, rsp_size);
    return 1;
}

PoMA_TCP_SPEC* createPoMATCPConnectSpec(PoMA_TCP_SPEC* spec, int portno, int multiuser){

    int sockfd;
  
    printf("port: %d\n", portno);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("#39\n");

    if (sockfd < 0)
        error("ERROR opening socket");
    spec->server_sockfd = sockfd;    
    bzero((char *) &spec->serv_addr, sizeof(spec->serv_addr));
    spec->port = portno;
    spec->serv_addr.sin_family = AF_INET;
    spec->serv_addr.sin_addr.s_addr = INADDR_ANY;
    spec->serv_addr.sin_port = htons(portno);
    printf("#49\n");
    printf("port %d\n", spec->serv_addr.sin_port);
    
    if (bind(sockfd, (struct sockaddr *) &spec->serv_addr,
             sizeof(&spec->serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
   
    printf("Socket bound. Waiting on port %d... \n", spec->port);
    spec->multi_user = multiuser;
    spec->running = 1;
    return spec;
}


void processMessagesLoop(PoMA_TCP_SPEC *spec  ,Topic *topicsHead){
unsigned char status=1;
ssize_t n;
char buffer[256];
while( status > 0 )
    {
        bzero(buffer,256);
        n = read(spec->session_sockfd,buffer,255);
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
    close(spec->session_sockfd);


}

void processClientsLoop(PoMA_TCP_SPEC *spec  ,Topic *topicsHead){
    socklen_t clilen;
    do
    {
        clilen = sizeof(&spec->cli_addr);
        spec->session_sockfd = accept(spec->server_sockfd, (struct sockaddr *) &spec->cli_addr, clilen);
        if (&spec->session_sockfd < 0)
            error("ERROR on accept");
        printf("Client Connected.\n");
        ref_sockfd = spec->session_sockfd;
        processMessagesLoop(spec  ,topicsHead);

    } while (spec->running !=0);


}

