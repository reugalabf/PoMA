# PoMa Example

The example has two Topics: GlobalVar and g_var. These two topics share the same setter and getter callbacks which in turn access the global varialbe *GlobalVar*

The example can open a TCP interface as well as a Bluetooth (Serial) interface

# How to Run and Test

## Configure Bluetooth device and services on linux

Make the bluetooth interface "discoverable" and "pairable". 
Register Serial Port to the Bluetooth service

You can run/browse setup-bluetooth.sh 

## Execute the Example 

Execute the *poma_example* with a TCP port number as a parameter

```
./poma_example --tcp_port 3333
```
or execute it with a Bluetooth channel parameter

```
./poma_example --blue_channel 1
```
or execute it to open both a TCP port and a Bluetooth channel
```
./poma_example --tcp_port 3333 --blue_channel 1
```

You can connect to poma_example using *netcat* or similar from another device or your own computer
```
netcat 127.0.0.1 3333
```
You can connect to the bluetooth interface using bluetooth terminal such as "Serial Bluetooth Terminal" on a mobile device. 

Now you can send commands (follow by an enter) to *poma_example* for example:

The list command (*) will return the list of available Topics

```
*
ACK: GlobalVar | g_var | 
```

The set command (=) followed by a Topic and a value will assign that value to a variable associated to a Topic 

```
= g_var 8
ACK: done 
```

The get command (?) followed by a Topic will return the assigned value to a variable 

```
? g_var
ACK: 8
```
