#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdlib.h>

#include <unistd.h>

typedef struct Topic
{
    char key[20];
    void (* setter)(int, char *);
    void (* getter)(int, char *);
    struct Topic *next;
}
Topic;

void defaultSetter(int sockfd, char *argument);

void defaultGetter(int sockfd, char* argument);

Topic *createTopic(char* newKey,void (*getter), void (*setter)  );

void addTopic(Topic *topics, Topic *new_topic);

void* findGetter(Topic *topics, char* key );

void* findSetter(Topic *topics, char* key );

void processGetterMessage(int newsockfd, char * buffer, Topic* topics);

void processSetterMessage(int newsockfd, char * buffer,  Topic* topics );

void processListTopics(int sockfd, char * buffer, Topic* topics);

int processMessage(int newsockfd, char * buffer, Topic* topics);
