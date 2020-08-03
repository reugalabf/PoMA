# PoMA
Poor Man's API

**PoMA** is a simple API that makes it possible to **set** and **get** variables from a running system. PoMA is implemented on top TCP sockets.

It is motivated by the need to manage the configuration of dedicated devices (such as IoT devices) without using a local web server, i.e. a web servers running on the device. 

PoMA is based on the idea of *Topics*. For every Topic the system can handle two different kind of operations: **get** and **set**. There is also another operation to **list** available Topics.

##Operations

There are three operations.

1. **Get**: this operation takes one parameter (a topic ) and returns the value or status for a given topic.  

    ```
    ? TOPICNAME
    ```
1. **Set**: this operation takes two parameters (a topic and a value) and sets the value to the given topic.

    ```
    = TOPICNAME VALUE
    ```
1. **List**: this operation takes no parameter and return a list of the available topics  
    ```
    *
    ```

## Network Protocol 
**PoMA** is implemented on top of TCP sockets. The system that will receive and process PoMA operations will be responsible of managing the PoMA server.
 Since PoMA is implemented on top of TCP sockets, it is possible to use existing low levels tools such as *netcat* on the user side to use PoMA. It also opens the possibility to develop high level tools and libraries to interface with a PoMA server. 
 
 A typical session has the following steps:

1. Start the PoMA server (on the system being accessed) and wait for messages on a given port

1. On the client side, open a TCP connection to the server to start the session

    1. The client sends messages with available operations: ? (get), = (set) and * (list)

    1. The client gets a reply for each message

1. The client sends an empty message to close the session.


