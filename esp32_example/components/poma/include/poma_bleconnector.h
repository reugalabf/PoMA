
#include <sys/socket.h>
//#include <bluetooth/bluetooth.h>
//#include <bluetooth/rfcomm.h>

#include "poma_core.h"

//extern int ref_sockfd;
#define MULTI_USER 1
#define SINGLE_USER 0

typedef struct PoMA_BLE_SPEC
{
    //int running;

    //uint8_t own_addr_type;;
    //int multi_user;
    char device_name[249]; //ble max device name but practical adverticement is 30
    void (*processClientsLoop)(struct PoMA_BLE_SPEC *spec, Topic *topicsHead);
} PoMA_BLE_SPEC;


/* Provided by the NimBLE "store" component; persists bonding info in NVS. */
void ble_store_config_init(void);


void processBLEMessagesLoop(PoMA_BLE_SPEC *spec, Topic *topicsHead);

PoMA_BLE_SPEC *createPoMABLEConnectSpec(PoMA_BLE_SPEC *spec, uint8_t portno, int multiuser);
