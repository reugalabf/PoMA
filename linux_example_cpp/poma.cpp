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

Poma::Poma(){
    fallback = new Topic ("");    
    if (fallback != NULL)
    {
        fallback->getter = &defaultGetter;
        fallback->setter = &defaultSetter;
    }
}
Poma::~Poma() {
/* Topic *topics;
  Topic *fallback;*/
delete fallback;
fallback = NULL;
delete topic;
topic = NULL;



}

int Poma::processMessage(int newsockfd, char *buffer)
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


void Poma::registerTopic(char *newKey, ptr getter, ptr setter)
{
    
    addTopic(createTopic(newKey,  getter, setter) );
}

/***********************************************
 * poma
 * private
 * *********************************************/


Topic *Poma::createTopic(char *newKey, ptr getter, ptr setter)
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



void Poma::addTopic(Topic *new_topic)
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




Topic *Poma::findTopic(char *key)
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


void Poma::processGetterMessage(int newsockfd, char *buffer)
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

void Poma::processSetterMessage(int newsockfd, char *buffer)
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

void Poma::processListTopics(int sockfd, char *buffer)
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

Topic::Topic( char *aKey)
{

    strncpy ((char*)key,aKey, sizeof(key) );
    next = NULL;
}
Topic::~Topic() {
/*
    char key[20];
    ptr setter;
    ptr getter ;
    Topic *next;
  */
    key = NULL
    delete setter;
    setter = NULL;
    delete getter;
    getter = NULL;
    delete next;
    next = NULLs;

}



/* PomaSocketListener

*/
PomaSocketListener::PomaSocketListener( Poma *poma){

board = poma;

}


PomaSocketListener::~PomaSocketListener(){}

void PomaSocketListener::error(const char* msg)
{
    perror(msg);
    exit(1);
}


void PomaSocketListener::start(int portno){
int sockfd, newsockfd;
    socklen_t clilen;
    unsigned char status = 1;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    printf("Socket bound. Waiting on port %d... \n", portno);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    printf("Client Connected.\n");

    while( status > 0 )
    {
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
                if (n < 0)
        {
            status = 0;
            printf("status: %d \n", status);
            error("ERROR writing to socket");
        }
        if (strlen(buffer) == 1 )
        {
            status = 0;
            //printf("--status: %d \n", status);
        }
        else
        {
            //processMessage(newsockfd, buffer, topicHead);
            board->processMessage(newsockfd, buffer);

        }
    }
    close(newsockfd);

}

void PomaSocketListener::stop(){}