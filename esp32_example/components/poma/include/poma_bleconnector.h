
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
    //uint8_t own_addr_type;;
    int multi_user;
    
    void (*processClientsLoop)(struct PoMA_BLE_SPEC *spec, Topic *topicsHead);
} PoMA_BLE_SPEC;

static uint8_t own_addr_type;
static int gap_event_handler(struct ble_gap_event *event, void *arg);

/* Provided by the NimBLE "store" component; persists bonding info in NVS. */
void ble_store_config_init(void);


static int blewriter(const void *response, size_t rsp_size);
static int buffered_blewriter(const void *response, size_t rsp_size);
void processBLEMessagesLoop(PoMA_BLE_SPEC *spec, Topic *topicsHead);

PoMA_BLE_SPEC *createPoMABLEConnectSpec(PoMA_BLE_SPEC *spec, uint8_t portno, int multiuser);