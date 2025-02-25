#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdlib.h>

#include <unistd.h>

#include <sys/types.h>

#include <netinet/in.h>




#define HELP_MESSAGE "Available commands: ? (get), = (set), * (list) \n"

typedef void (*ptr)(int, char*);



class Topic
{
public:
  Topic( char* k);
  ~Topic();
  char key[20];
  ptr setter;
  ptr getter ;
  Topic *next;

  bool isMatching(char *candidate);
  void linkTopic(Topic *aTopic);
  void invokeSetter(int sockfd, char *message);
  void invokeGetter(int sockfd, char *message);  
};

class Poma
{
public:
  Topic *topics;
  Topic *fallback;
  int processMessage(int newsockfd, char *buffer);
  void registerTopic(char *newKey, ptr getter,  ptr setter);

  Poma();
  ~Poma();

private:
  void addTopic(Topic *new_topic);
  Topic* createTopic(char *newKey, ptr getter,  ptr setter);
  Topic* findTopic(char *key);
  void processGetterMessage(int newsockfd, char *buffer);
  void processSetterMessage(int newsockfd, char *buffer);
  void processListTopics(int sockfd, char *buffer);
};

class PomaSocketListener
{
public:
PomaSocketListener(Poma *poma);
~PomaSocketListener();
void start(int port);
void error(const char* msg);
void stop();

Poma *board;
int socket_desc;

};
