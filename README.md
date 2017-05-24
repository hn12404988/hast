# hast

Server and client libraries for socket communication with topology features in linux, supporting TCP/IP and Unix Domain socket. Features are handling requests in parallel and tiny enough to be embeded in program. 

* [中文簡介](https://github.com/hn12404988/hast/blob/master/README_Chinese.md)

## Introduction Video

* Abstraction Layer
  - [English](https://www.youtube.com/watch?v=EpoL8mSOA6E)(version < 1.0.0, OUTDATED)
  - [中文](https://www.youtube.com/watch?v=G41F7xHC2bs)(version < 1.0.0, OUTDATED)
* API Layer (WIP)
* Code Layer (WIP)

## Main Features of Server Class

* The way server deal with request is like the concept of `goroutine` in GO. You set the maximum amount of threads (or by default, maximum is 2 threads) to deal with requests. It's not thread-pool with fixed amount of threads, the amount of threads is dynamic, so it can be 1 to maximum amount in any time. 
* Server can be frozen with all threads or certain threads by the call from client. More details are in the `example` folder. 

## Main Features of Client Class

* `client_core` is a client library for single-thread used. It send request and wait for the reply.
* `client_thread` is a client library for multi-thread used. It can send request and open a new thread to receive the reply.

## Getting Started

* Only for Linux (kernel > 2.5.44 because using `epoll.h`). 
* `gcc` > 4.9, due to this [bug](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54562)
* Header-only library, so copy `hast` folder to your include folder.
* This project use `std::thread`, so compile file with `std::thread` library (Can be `-pthread`).

## Wiki Page

* More information about how to use this library in this repository's [wiki page](https://github.com/hn12404988/hast/wiki).

## Framework

* There is another my project called [dalahast](https://github.com/hn12404988/dalahast), which is a example for this project. It contain a complete system from web front-end to hast back-end.
* [hast_web](https://github.com/hn12404988/hast_web) is a library base on this project. The difference between them is that hast_web open a websocket port and remove all topology features.

## Bugs and Issues

This project is still in developed and maintained. Open an issue if you discover some problems.
