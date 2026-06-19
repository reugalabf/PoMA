
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "poma_bleconnector.h"

static void error(char *msg){
    perror(msg);
    exit(1);
}

static int BLEwriter(const void *response, size_t rsp_size){

}

void processMessagesLoop(PoMA_BLE_SPEC *spec, Topic *topicsHead){

}

void BLEClientsLoopHandler(PoMA_BLE_SPEC *spec, Topic *topicsHead){

}

PoMA_BLE_SPEC *createPoMABLEConnectSpec(PoMA_BLE_SPEC *spec, uint8_t portno, int multiuser){

    int sockfd=-1;
//    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    socklen_t opt = sizeof(rem_addr);
    
    sockfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM); 
    

    if (sockfd < 0)
        error("ERROR opening socket");
    spec->server_sockfd = sockfd;
    bzero((char *)&(spec->serv_addr), sizeof(spec->serv_addr));
    spec->port = portno;
    spec->serv_addr.rc_family = AF_BLUETOOTH;
    spec->serv_addr.rc_addr = (bdaddr_t) {0}; // Any local adapter
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