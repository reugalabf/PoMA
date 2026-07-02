/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "poma_tcpconnector.h"
#include "poma_bleconnector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

int GlobalVar = 0;

void setterGlobalVar(WRITERFUNC, char *argument)
{
    if (argument != NULL)
        GlobalVar = atoi(argument);
    writer("done", strlen("done") );
}

void getterGlobalVar(WRITERFUNC, char *argument)
{
    char response[10];
    sprintf(response, "%d", GlobalVar);
    writer(response, strlen(response));
}

typedef struct Server_Thread_Spec
{

    PoMA_BLE_SPEC *blue_spec;
    PoMA_TCP_SPEC *tcp_spec;
    Topic *head;

} Server_Thread_Spec;

void *tcp_thread(void *param)
{

    Server_Thread_Spec *spec = (Server_Thread_Spec *)param;
    spec->tcp_spec->processClientsLoop(spec->tcp_spec, spec->head);
    return NULL;
}

void *blue_thread(void *param)
{

    Server_Thread_Spec *spec = (Server_Thread_Spec *)param;
    spec->blue_spec->processClientsLoop(spec->blue_spec, spec->head);
    return NULL;
}

int main(int argc, char *argv[])
{
    int tcp_port_idx = -1;
    int blue_channel_idx = -1;

    pthread_t t1, t2; // Thread handles
    int spawn1 = -1;
    int spawn2 = -1;

    PoMA_TCP_SPEC *tcpSpec = malloc(sizeof(PoMA_TCP_SPEC));
    PoMA_BLE_SPEC *btSpec = malloc(sizeof(PoMA_BLE_SPEC));

    Server_Thread_Spec thread_spec;
    Topic *topicHead;

    topicHead = createTopic("GlobalVar", getterGlobalVar, setterGlobalVar);
    addTopic(topicHead, createTopic("g_var", getterGlobalVar, setterGlobalVar));

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no TCP port or Bluetooth channel provided\n");
        fprintf(stderr, "Usage: %s --tcp_port PORT | --blue_channel CHANNEL  \n", argv[0]);
        exit(1);
    }

    thread_spec.head = topicHead;

    for (int i = 1; i < argc; i += 2)
    {
        const char *key = argv[i];

        if ((strcmp(key, "--tcp_port") == 0) && argc > i + 1)
        {
            tcp_port_idx = i + 1;
        }
        else if ((strcmp(key, "--blue_channel") == 0) && argc > i + 1)
        {
            blue_channel_idx = i + 1;
        }
        else if (strcmp(key, "--help") == 0)
        {
            fprintf(stderr, "Usage: %s --tcp_port PORT | --blue_channel  \n", argv[0]);
        }
        else
        {
            fprintf(stderr, "Usage: %s --tcp_port PORT | --blue_channel CHANNEL  \n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    //printf("here ...argc %d tcp_port_idx %d\n", argc, tcp_port_idx);
    /**************/
    
    if (tcp_port_idx != -1)
    {
        tcpSpec = createPoMATCPConnectSpec(tcpSpec, atoi(argv[tcp_port_idx]), SINGLE_USER); // or MULTI_USER
        thread_spec.tcp_spec = tcpSpec;
        if (spawn1 = pthread_create(&t1, NULL, tcp_thread, &thread_spec) != 0)
        {
            perror("Failed to create PoMA TCP thread ");
            return -1;
        }
    }

    if (blue_channel_idx != -1)
    {
        btSpec = createPoMABLEConnectSpec(btSpec, atoi(argv[blue_channel_idx]), SINGLE_USER); // or MULTI_USER
        thread_spec.blue_spec = btSpec;
        if (spawn2 = pthread_create(&t2, NULL, blue_thread, &thread_spec) != 0)
        {
            perror("Failed to create PoMA Bluetooth thread ");
            return -1;
        }
    }

    spawn1 == 0 && pthread_join(t1, NULL);
    spawn2 == 0 && pthread_join(t2, NULL);
    return 0;
}
