#ifndef client_core_h
#define client_core_h

#include <iostream>
#include<sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/un.h>
#include <vector>
#include<unistd.h>    //close
#include <sys/epoll.h>
#include <hast/tcp_config.h>
#include <hast/unix_config.h>
#include <cstring> //errno
//#include<arpa/inet.h> //inet_addr
//#include <netinet/in.h>

/******************** Error Flag *****************************
 * 1: Server doesn't exist, or socket has problem.
 * 2: Fail on sending message.
 * 3: Server's execution crash.
 * 4: No reply.
 * 5: waiting list of cient_thread jam.
 * 6: Fail on epoll.
 * 7: Invalid message format.
 * 8: runner is not enough (client_thread).
 * 9: thread joinable is false (client_thread).
 * 10: epoll events is not 1.
 *************************************************************/

#define MAX_EVENTS 5

class client_core : public tcp_config{
protected:
	struct sockaddr_un addr; //unix only
	short int wait_maximum {2};
	short int error_socket_index {-1};
	std::string node_name {"no name"};
	std::string server_index {"-1"};
	static const int transport_size {100};
	char reply[transport_size];
	std::string str;
	short int i;
	int j;
	int epollfd;
	struct epoll_event ev, events[MAX_EVENTS];
	std::vector<std::string> *location {nullptr};
	short int amount {0};
	std::vector<int> socketfd;
	std::vector<short int> location_list;
	
	inline void close_runner(short int index);
	inline void close_socket(int socket_index);
	inline bool build_on_i(short int &location_index);
	inline void build_runner(short int &location_index);
	inline short int receive(std::string &msg);
	std::string error_send(short int flag, short int index, std::string msg);
	std::string reply_error_send(short int index, std::string msg, std::string reply);
	inline void error_fire(std::string msg);
	inline void up();
public:
	client_core();
	~client_core();
	short int fire(short int &location_index,std::string &msg);
	short int fireNclose(short int &location_index,std::string &msg);
	short int fireNfreeze(short int &location_index,std::string &msg);
	short int fireNcheck(short int &location_index,std::string &msg);
	short int unfreeze(short int &location_index);
	short int uncheck(short int &location_index);
	void import_location(std::vector<std::string> *location, short int amount = 0);
	void set_wait_maximum(short int wait);
	void set_error_node(short int socket_index,const char* file_name,std::string server);
	
};
#include <hast/client_core.cpp>
#endif

