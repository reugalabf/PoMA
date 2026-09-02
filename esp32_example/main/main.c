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
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;
    PoMA_TCP_SPEC *tcp_spec = (PoMA_TCP_SPEC *)pvParameters;

    tcp_spec->processClientsLoop(tcp_spec, topicHead);
/*
    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }
    else if (addr_family == AF_INET6)
    {
        bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0)
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1)
    {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET)
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        }
        else if (source_addr.sin6_family == PF_INET6)
        {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        // do_retransmit(sock);
        do_processQuery(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);*/
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

    ESP_LOGE(TAG, "before createTopic");
    topicHead = createTopic("GlobalVar", getterGlobalVar, setterGlobalVar);
    ESP_LOGE(TAG, "head topic %s", topicHead->key );
    ESP_LOGE(TAG, "before addTopic");
    Topic* newTopic = createTopic("g_var", getterGlobalVar, setterGlobalVar);
    ESP_LOGE(TAG, "new topic %s", newTopic->key );
    addTopic(topicHead, newTopic);
    ESP_LOGE(TAG, "before malloc");
    PoMA_TCP_SPEC *tcpSpec = malloc(sizeof(PoMA_TCP_SPEC));
    ESP_LOGE(TAG, "createPoMATCPConnection");

    tcpSpec = createPoMATCPConnectSpec(tcpSpec, PORT, SINGLE_USER); // or MULTI_USER
//#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(tcp_server_task, "tcp_server", 4096*5, (void *)tcpSpec, 5, NULL);
//#endif
//#ifdef CONFIG_EXAMPLE_IPV6
    //xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)AF_INET6, 5, NULL);
//#endif
}
