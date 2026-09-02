#include <stdio.h>
#include <string.h>

#include <assert.h>
#include <stdlib.h>

#include <unistd.h>



#ifndef POMACORE_H
#define POMACORE_H

typedef struct Topic
{
    char key[20];
    void (*setter)(void *, char *);
    void (*getter)(void *, char *);
    struct Topic *next;
} Topic;

// extern Topic *topicHead;

#define WRITERFUNC int (*writer)(const void *__buf, size_t __n)

#define AVAILABLE_COMMANDS "Available commands: ? (get), = (set), * (list) \n"

void defaultSetter(WRITERFUNC, char *argument);

void defaultGetter(WRITERFUNC, char *argument);

Topic *createTopic(char *newKey, void(*getter), void(*setter));

void addTopic(Topic *topics, Topic *new_topic);

void *findGetter(Topic *topics, char *key);

void *findSetter(Topic *topics, char *key);

void processGetterMessage(WRITERFUNC, char *buffer, Topic *topics);

void processSetterMessage(WRITERFUNC, char *buffer, Topic *topics);

void processListTopics(WRITERFUNC, char *buffer, Topic *topics);

int processMessage(WRITERFUNC, char *buffer, Topic *topics);

#endif