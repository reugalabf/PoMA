
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "poma_bleconnector.h"

static int ref_sockfd=-1;

static void error(char *msg){
    perror(msg);
    exit(1);
}

static int BLEwriter(const void *response, size_t rsp_size){

    assert(ref_sockfd >=0); 
    write(ref_sockfd, (char *)response, rsp_size);
    return 1;


}

void processBLEMessagesLoop(PoMA_BLE_SPEC *spec, Topic *topicsHead){
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
            processMessage(&BLEwriter, buffer, topicsHead);
        }
    }
    printf("\n");
    close(spec->session_sockfd);
}

static void BLEClientsLoopHandler(PoMA_BLE_SPEC *spec, Topic *topicsHead){
 socklen_t clilen;
    
    do
    {
        //printf("Start processMessagesLoop \n");
        clilen =  sizeof(&spec->cli_addr);
        spec->session_sockfd = accept(spec->server_sockfd, (struct sockaddr *)&spec->cli_addr, &clilen);
        if (spec->session_sockfd < 0)
            error("ERROR on accept");
    
        ref_sockfd = spec->session_sockfd;
        processBLEMessagesLoop(spec, topicsHead);
        //printf("After processMessagesLoop\n");
    } while (spec->running != 0);
    printf("Bye and Good night! \n");
}

PoMA_BLE_SPEC *createPoMABLEConnectSpec(PoMA_BLE_SPEC *spec, uint8_t portno, int multiuser){

    int sockfd=-1;
//    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
//    socklen_t opt = sizeof(rem_addr);
    
    sockfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM); 
    

    if (sockfd < 0)
        error("ERROR opening socket");
    spec->server_sockfd = sockfd;
    bzero((char *)&(spec->serv_addr), sizeof(spec->serv_addr));
    spec->port = portno;
    spec->serv_addr.rc_family = AF_BLUETOOTH;
    spec->serv_addr.rc_bdaddr = (bdaddr_t) {0}; // Any local adapter
    spec->serv_addr.rc_channel = portno;
    
    if (bind(spec->server_sockfd, (struct sockaddr *)&(spec->serv_addr),
             sizeof(spec->serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5); //5 is for backlog size. Read documentation. 

    printf("Socket bound. Waiting on port %d... \n", spec->port);
    spec->multi_user = multiuser;
    spec->running = 1;
    spec->processClientsLoop = BLEClientsLoopHandler;
    return spec;

}