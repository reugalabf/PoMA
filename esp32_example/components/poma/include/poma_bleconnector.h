
#include <sys/socket.h>
//#include <bluetooth/bluetooth.h>
//#include <bluetooth/rfcomm.h>

#include "poma_core.h"

//extern int ref_sockfd;
#define MULTI_USER 1
#define SINGLE_USER 0

typedef struct PoMA_BLE_SPEC
{
    int running;
    uint8_t own_addr_type;;
    int multi_user;
    
    void (*processClientsLoop)(struct PoMA_BLE_SPEC *spec, Topic *topicsHead);
} PoMA_BLE_SPEC;

void processBLEMessagesLoop(PoMA_BLE_SPEC *spec, Topic *topicsHead);

PoMA_BLE_SPEC *createPoMABLEConnectSpec(PoMA_BLE_SPEC *spec, uint8_t portno, int multiuser);