#include "poma.hpp"

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



/***********************************************
 * poma
 * public
 * *********************************************/

poma::poma(){
    fallback = new Topic ("");    
    if (fallback != NULL)
    {
        fallback->getter = &defaultGetter;
        fallback->setter = &defaultSetter;
    }
}
poma::~poma() {}

int poma::processMessage(int newsockfd, char *buffer)
{
    int n;
    //    printf("Here is the message of %zu length: %s\n",strlen(buffer), buffer);
    n = write(newsockfd, "ACK: ", strlen("ACK: "));
    switch (buffer[0])
    {
    case '?':
        processGetterMessage(newsockfd, &buffer[1]);
        break;
    case '=':
        processSetterMessage(newsockfd, &buffer[1]);
        break;
    case '*':
        processListTopics(newsockfd, &buffer[1]);
        break;
    default:
        write(newsockfd, HELP_MESSAGE, strlen(HELP_MESSAGE));
    }
    return n;
}


void poma::registerTopic(char *newKey, ptr getter, ptr setter)
{
    
    addTopic(createTopic(newKey,  getter, setter) );
}

/***********************************************
 * poma
 * private
 * *********************************************/


Topic *poma::createTopic(char *newKey, ptr getter, ptr setter)
{
    Topic *newTopic;
    assert(strlen(newKey) < 21);
    newTopic = new Topic (newKey);
    
    if (newTopic != NULL)
    {
        newTopic->getter = getter;
        newTopic->setter = setter;
    
    }
    return newTopic;
}



void poma::addTopic(Topic *new_topic)
{
    Topic *current = topics;
    if(current != NULL){
        /*while (current->next != NULL)
        {
            current = current->next;
            
        }
        current->next = new_topic;*/
        current->linkTopic(new_topic);
    }
    else{topics = new_topic;}
}




Topic *poma::findTopic(char *key)
{
    Topic *current = topics;
    // printf("topic: ->%s<- , key: ->%s<- ", current->key, key);

    while (current != NULL)
    {
        if (current->isMatching(key) ==0)
            return current;
        current = current->next;
    }
    return fallback;
}


void poma::processGetterMessage(int newsockfd, char *buffer)
{
    //void (*getter)(int, char *);
    Topic *topic;
    char *key;
    char delims[3] = {' ', '\n', '\0'};

    key = strtok(buffer, delims);
    topic = findTopic( key);
    if (topic != NULL)
        topic->invokeGetter(newsockfd, buffer);
}

void poma::processSetterMessage(int newsockfd, char *buffer)
{
    //void (*setter)(int, char *);
    Topic *topic;
    char *key, *argument;
    char delims[4] = {' ', '\n', '\0', '\t'};
    char lineDelim[2] = {'\n'};

    key = strtok(buffer, delims);
    // printf("buffer: ->%s<-", buffer);
    topic = findTopic( key);
    argument = strtok(NULL, lineDelim);
    if (topic != NULL && key != NULL)
        topic->invokeSetter(newsockfd, argument);
}

void poma::processListTopics(int sockfd, char *buffer)
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

/***********************************************
 * Topic
 * public
 * *********************************************/

bool Topic::isMatching(char *candidate){
    return strcmp(candidate, key);
};

void Topic::linkTopic(Topic *aTopic){
        //avoiding recurive calls
        Topic *current = this;
        while (current->next != NULL)
        {
            current = current->next;
            
        }
        current->next = aTopic;
};
void Topic::invokeSetter( int sockfd, char *arguments)
{   
    if (setter != NULL /*&& arguments != NULL*/)
        setter(sockfd, arguments);
};

void Topic::invokeGetter(int newsockfd, char *buffer)
{
    
    if (getter != NULL)
        getter(newsockfd, buffer);
};

Topic::Topic(char *aKey)
{

    strncpy (key,aKey, sizeof(key) );
    next = NULL;
}
Topic::~Topic() {}
