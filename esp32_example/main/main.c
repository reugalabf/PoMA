/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

// #include <poma.h>
#include "poma_tcpconnector.h"
#include "poma_bleconnector.h"

#define PORT CONFIG_EXAMPLE_PORT

static const char *TAG = "ESP_POMA";

int GlobalVar = 0;
Topic *topicHead;

void setterGlobalVar(WRITERFUNC, char *argument)
{
    if (argument != NULL)
        GlobalVar = atoi(argument);
    writer("done", strlen("done"));
}

void getterGlobalVar(WRITERFUNC, char *argument)
{
    char response[10];
    sprintf(response, "%d", GlobalVar);
    writer(response, strlen(response));
}

static void tcp_server_task(void *pvParameters)
{
    // char addr_str[128];
    // int addr_family = (int)pvParameters;
    // int ip_protocol = 0;
    // struct sockaddr_in6 dest_addr;
    PoMA_TCP_SPEC *tcp_spec = (PoMA_TCP_SPEC *)pvParameters;

    tcp_spec->processClientsLoop(tcp_spec, topicHead);
}

static void ble_server_task(void *pvParameters)
{
    PoMA_BLE_SPEC *ble_spec = (PoMA_BLE_SPEC *)pvParameters;

    ble_spec->processClientsLoop(ble_spec, topicHead);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    topicHead = createTopic("GlobalVar", getterGlobalVar, setterGlobalVar);

    Topic *newTopic = createTopic("g_var", getterGlobalVar, setterGlobalVar);

    addTopic(topicHead, newTopic);

    PoMA_TCP_SPEC *tcpSpec = malloc(sizeof(PoMA_TCP_SPEC));

    tcpSpec = createPoMATCPConnectSpec(tcpSpec, PORT, SINGLE_USER); // or MULTI_USER
    xTaskCreate(tcp_server_task, "tcp_server", 4096 * 5, (void *)tcpSpec, 5, NULL);

    PoMA_BLE_SPEC *bleSpec = malloc(sizeof(PoMA_BLE_SPEC));

    bleSpec = createPoMABLEConnectSpec(bleSpec, 1, SINGLE_USER);
    // a new task is created... but nimble runds the stack on its own task. WIP    
    xTaskCreate(ble_server_task, "ble_server", 4096 * 2, (void *) bleSpec, 5, NULL);
}