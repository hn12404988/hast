# Neural Socket

TCP and Unix Domain socket libraries written in C++11, including server and client. Server can hold multiple connections (by opening new thread). Client can also hold multiple connections to servers.

## Main Feature of Server Class

* Each connection running independently on its own thread. Once the client shut down the connection or the connection fail in some reasons, this thread will close and recycle automatically.

## Main Feature of Client Class

* One client can hold multiple connections to different servers or a server (A server can create multiple threads for a client). You can set a maximum amount that it can hold. If the contact list is full, client will shut down one connection which is used most unfrequently, and add the new connection in.

## Getting Started

* So far, this project is only tested on Linux. 
* You will need an C++ compiler which can support C++11.
* Copy "neural_socket" directory into your OS include directory.
* Usage of this project and other details please refer to example files.
* Try to compile the example files, and do some test run. Compile options can be (both for server and client):

```
g++ --std=c++11 -pthread -o unix_client.o unix_client_example.cpp
```

## Bugs and Issues

This project is still in developed and maintained. Open an issue if you discover some problems.
