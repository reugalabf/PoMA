
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "poma_core.h"

//extern int ref_sockfd;
#define MULTI_USER 1
#define SINGLE_USER 0

typedef struct PoMA_BLE_SPEC
{
    int running;
    int session_sockfd;
    int server_sockfd;
    int port;
    int multi_user;
    struct sockaddr_rc serv_addr;
    struct sockaddr_rc cli_addr;
    void (*processClientsLoop)(struct PoMA_BLE_SPEC *spec, Topic *topicsHead);
} PoMA_BLE_SPEC;

//static void error(char *msg);

//static int writer(const void *response, size_t rsp_size);

void processBLEMessagesLoop(PoMA_BLE_SPEC *spec, Topic *topicsHead);

//void BLEClientsLoopHandler(PoMA_BLE_SPEC *spec, Topic *topicsHead);

PoMA_BLE_SPEC *createPoMABLEConnectSpec(PoMA_BLE_SPEC *spec, uint8_t portno, int multiuser);