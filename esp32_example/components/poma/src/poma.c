#include "poma.h"
#include "esp_log.h"


static const char *TAG = "PoMA";

void defaultSetter(int sockfd, char *argument)
{
    char response[25];
    sprintf(response, "Setter Key not found\n");
    write(sockfd, response, strlen(response));
}

void defaultGetter(int sockfd, char *argument)
{
    char response[25];
    sprintf(response, "Getter Key not found\n");
    write(sockfd, response, strlen(response));
}

Topic *createTopic(char *newKey, void(*getter), void(*setter))
{
    Topic *newTopic;
    assert(strlen(newKey) < 21);
    newTopic = malloc(sizeof(Topic));
    if (newTopic != NULL)
    {
        strcpy(newTopic->key, newKey);
        newTopic->getter = getter;
        newTopic->setter = setter;
        newTopic ->next = NULL;
    }
    //ESP_LOGI(TAG, "createTopic: new key: %s", newTopic->key);
    return newTopic;
}
void addTopic(Topic *topics, Topic *new_topic)
{
    Topic *current = topics;
        while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = new_topic;
}

void *findGetter(Topic *topics, char *key)
{
    Topic *current = topics;
    // printf("topic: ->%s<- , key: ->%s<- ", current->key, key);
    //ESP_LOGI(TAG, "findGetter key: %s", key);
    if (key != NULL)
    {
        while (current != NULL)
        {   
            //ESP_LOGI(TAG, "findGetter current_key: %s", current->key);
            if (strcmp(key, current->key) == 0)
                return current->getter;
            current = current->next;
        }
    }
    return defaultGetter;
}

void *findSetter(Topic *topics, char *key)
{
    Topic *current = topics;
    // printf("topic: ->%s<- , key: ->%s<- ", current->key, key);

    if (key != NULL)
    {
        while (current != NULL)
        {
            if (strcmp(key, current->key) == 0)
                return current->setter;
            current = current->next;
        }
    }
    return defaultSetter;
}

void processGetterMessage(int newsockfd, char *buffer, Topic *topics)
{
    void (*getter)(int, char *);
    char *key;
    char delims[3] = {' ', '\n', '\0'};

    key = strtok(buffer, delims);
    getter = findGetter(topics, key);
    if (getter != NULL)
        getter(newsockfd, buffer);
}

void processSetterMessage(int newsockfd, char *buffer, Topic *topics)
{
    void (*setter)(int, char *);
    char *key, *argument;
    char delims[3] = {' ', '\n', '\0'};
    char lineDelim[2] = {'\n'};

    key = strtok(buffer, delims);
    // printf("buffer: ->%s<-", buffer);
    setter = findSetter(topics, key);
    argument = strtok(NULL, lineDelim);
    if (setter != NULL && key != NULL)
        setter(newsockfd, argument);
}

void processListTopics(int sockfd, char *buffer, Topic *topics)
{
    Topic *current = topics;

    while (current != NULL)
    {
        write(sockfd, current->key, strlen(current->key));
        write(sockfd, " | ", 4);
        current = current->next;
    }
    write(sockfd, "\n", 2);
}

int processMessage(int newsockfd, char *buffer, Topic *topics)
{
    int n;
    //    printf("Here is the message of %zu length: %s\n",strlen(buffer), buffer);
    n = write(newsockfd, "ACK: ", strlen("ACK: "));
    switch (buffer[0])
    {
    case '?':
        processGetterMessage(newsockfd, &buffer[1], topics);
        break;
    case '=':
        processSetterMessage(newsockfd, &buffer[1], topics);
        break;
    case '*':
        processListTopics(newsockfd, &buffer[1], topics);
        break;
    default:
        write(newsockfd, "Available commands: ? (get), = (set), * (list) \n", strlen("Available commands: ? (get), = (set), * (list) \n"));
    }
    return n;
}
