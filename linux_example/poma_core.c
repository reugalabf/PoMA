#include "poma_core.h"

#define LIST_SEPARATOR " ! "
#define NEW_LINE "\n"
#define NEW_LINE_COUNT sizeof(NEW_LINE) -1

void defaultSetter(WRITERFUNC, char *argument)
{
    char response[25];
    sprintf(response, "Setter Key not found\n");
    writer( response, strlen(response));
}

void defaultGetter(WRITERFUNC, char *argument)
{
    char response[25];
    sprintf(response, "Getter Key not found\n");
    writer( response, strlen(response));
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
    }
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
    //printf("key %s size: %d\n", key, sizeof(key));
    if (key != NULL)
    {
        while (current != NULL)
        {
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

void processGetterMessage(WRITERFUNC, char *buffer, Topic *topics)
{
    void (*getter)(void*, char *);
    char *key;
    char delims[4] = {' ', '\n', '\r','\0' };

    key = strtok(buffer, delims);

    getter = findGetter(topics, key);
    if (getter != NULL)
        getter(writer, buffer);
    writer( NEW_LINE, NEW_LINE_COUNT);
}

void processSetterMessage(WRITERFUNC, char *buffer, Topic *topics)
{
    void (*setter)(void*, char *);
    char *key, *argument;
    char delims[4] = {' ', '\n', '\r', '\0'};
    char lineDelim[3] = {'\n','\r', '\0'};

    key = strtok(buffer, delims);
    // printf("buffer: ->%s<-", buffer);
    setter = findSetter(topics, key);
    argument = strtok(NULL, lineDelim);
    if (setter != NULL && key != NULL)
        setter(writer, argument);
    writer( NEW_LINE, NEW_LINE_COUNT);
}

void processListTopics(WRITERFUNC, char *buffer, Topic *topics)
{
    Topic *current = topics;

    while (current != NULL)
    {
        writer(current->key, strlen(current->key));
        writer( LIST_SEPARATOR, sizeof(LIST_SEPARATOR)-1);
        current = current->next;
    }
    writer( NEW_LINE, NEW_LINE_COUNT);
}

int processMessage(WRITERFUNC, char *buffer, Topic *topics)
{
    int n;
    //    printf("Here is the message of %zu length: %s\n",strlen(buffer), buffer);
    n = writer( "ACK: ", strlen("ACK: "));
    switch (buffer[0])
    {
    case '?':
        processGetterMessage(writer, &buffer[1], topics);
        break;
    case '=':
        processSetterMessage(writer, &buffer[1], topics);
        break;
    case '*':
        processListTopics( writer, &buffer[1], topics);
        break;
    default:
        writer( AVAILABLE_COMMANDS, strlen(AVAILABLE_COMMANDS));
    }
    return n;
}