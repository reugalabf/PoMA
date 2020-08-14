# PoMa Example

The example has two Topics: GlobalVar and g_var. These two topics share the same setter and getter callbacks which in turn access the global varialbe *GlobalVar*



# How to Run and Test

Execute the *poma_example* with the port number as a parameter

```
./poma_example 3333
```
which will make poma_example to wait for a connecting client on port 3333. 

Now you can connect to poma_example using *netcat* or similar from another device or your own computer
```
netcat 127.0.0.1 3333
```

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
