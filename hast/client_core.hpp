#ifndef hast_client_core_hpp
#define hast_client_core_hpp

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/un.h>
#include <vector>
#include <unistd.h>    //close
#include <sys/epoll.h>
#include <hast/tcp_config.hpp>
#include <hast/unix_config.hpp>
#include <cstring> //errno
#include <fcntl.h> //For TCP socket using non-block

//#include<arpa/inet.h> //inet_addr
//#include <netinet/in.h>

/******************** Error Flag *****************************
 * 1: Server doesn't exist, or socket has problem.
 * 2: Fail on sending message.
 * 3: Server's execution crash.
 * 4: No reply.
 * 5: Fail on epoll.
 * 6: Invalid message format.
 * 7: thread joinable is false (client_thread).
 * 8: epoll events is not 1.
 * 9: SSL_READ fail.
 *************************************************************/

namespace hast_client{
	const char SUCCESS {0};
	const char EXIST {1};
	const char SEND {2}; 
	const char CRASH {3};
	const char REPLY {4};
	const char EPOLL {5};
	const char FORMAT {6};
	const char JOIN {7};
	const char EPOLL_EV {8};
	const char SSL_r {9};
};
#define MAX_EVENTS 5

class client_core : public tcp_config{
protected:
	struct sockaddr_un addr; //unix only
	short int wait_maximum {2000};
	short int error_socket_index {-1};
	std::string node_name {"no name"};
	static const int transport_size {100};
	int epollfd;
	struct epoll_event ev, events[MAX_EVENTS];
	std::vector<std::string> *location {nullptr};
	short int amount {0};
	int *socketfd {nullptr};
	short int *location_list {nullptr};
	
	virtual inline void close_runner(short int runner_index);
	virtual inline short int build_runner(short int location_index);
	virtual inline short int get_runner(short int location_index);
	virtual char write(short int &runner_index, short int location_index, std::string &msg);
	virtual char read(short int runner_index, std::string &reply_str);
	inline bool build_on_i(short int i, short int location_index);
	inline char receive(short int runner_index,std::string &reply);
	std::string error_msg(const char flag, short int index, std::string msg);
	std::string reply_error_msg(short int index, std::string msg, std::string reply);
	inline void error_fire(std::string msg);
	inline short int up(short int runner_index);
	void echo_flag(const char flag);
	char fire_return(short int &location_index,std::string &msg, short int &runner_bk);
public:
	client_core();
	~client_core();
	char fire(short int &location_index,std::string &msg);
	char fireNclose(short int &location_index,std::string &msg);
	char fireNfreeze(short int &location_index,std::string &msg);
	char fireNcheck(short int &location_index,std::string &msg);
	char unfreeze(short int &location_index);
	char uncheck(short int &location_index);
	char shutdown_server(short int &location_index,std::string &shutdown_code);
	void import_location(std::vector<std::string> *location, short int unsigned amount = 0);
	void set_wait_maximum(short int wait);
	void set_error_node(short int socket_index,const char* file_name);
	std::vector<std::string> get_error_flag();
	
};
#include <hast/client_core.cpp>
#endif /* hast_client_core_hpp */

