# PoMA Esp32 Example 
The example is based on the TCP Server example that is part of esp-idf v 4.1

The device connects to a given Access Point and  runs a TCP Server, i.e. a simple server that reads a socket. 

The server has a simple PoMA's Topic chain.  Each received message is processed by calling the PoMA's message processing function.
The server will keep calling PoMA functionality with every message that is longer that 1 character.

## How to use example

1.  Configure the Access Point to which the device will connect

2.  Build and flash the example using the tools provided by esp-idf 4.1

3.  Create a TCP connection to the device (you have to know the IP and port number)
There are many host-side tools which can be used to interact with the TCP server. 
One command line tool is [netcat](http://netcat.sourceforge.net) which can send and receive many kinds of packets. 

There are three operations.

1. **Get**: this operation takes one parameter (a topic ) and returns the value or status for a given topic. The get operator is the character **?** 

    ```
    ? TOPICNAME
    ```
    Response

    ```
    ACK: SOMEVALUE
    ```

1. **Set**: this operation takes two parameters (a topic and a value) and sets the value to the given topic. The set operator is the character **=**

    ```
    = TOPICNAME VALUE
    ```
    
    Response

    ```
    ACK: done
    ```

1. **List**: this operation takes no parameter and return a list of the available topics. The list  operator is the character **\***  
    
    Response

    ```
    ACK: TOPICNAME | TOPICNAME2 | TOPICNAME3 |
    ```

## Hardware Required

This example can be run on any commonly available ESP32 development board.

