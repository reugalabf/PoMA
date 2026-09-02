#include <sys/socket.h>
#include <netinet/in.h>

#include "poma_core.h"

//extern int ref_sockfd;
#define MULTI_USER 1
#define SINGLE_USER 0

typedef struct PoMA_TCP_SPEC
{
    int running;
    int session_sockfd;
    int server_sockfd;
    int port;
    int multi_user;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    void (*processClientsLoop)(struct PoMA_TCP_SPEC *spec, Topic *topicsHead);
} PoMA_TCP_SPEC;

//static void error(char *msg);

//static int tcpwriter(const void *response, size_t rsp_size);

void processMessagesLoop(PoMA_TCP_SPEC *spec, Topic *topicsHead);

//void tcpClientsLoopHandler(PoMA_TCP_SPEC *spec, Topic *topicsHead);

PoMA_TCP_SPEC *createPoMATCPConnectSpec(PoMA_TCP_SPEC *spec, int portno, int multiuser);