#ifndef hast_client_core_hpp
#define hast_client_core_hpp

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
#include <hast/tcp_config.hpp>
#include <hast/unix_config.hpp>
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
	
	inline void close_runner(short int runner_index);
	inline bool build_on_i(short int i, short int location_index);
	virtual inline short int build_runner(short int location_index){
		short int runner_index;
		runner_index = amount-1;
		for(;runner_index>=0;--runner_index){
			if(socketfd[runner_index]==-1){
				if(build_on_i(runner_index,location_index)==true){
					location_list[runner_index] = location_index;
					return runner_index;
				}
				else{
					return -1;
				}
			}
		}
		if(runner_index==-1){
			runner_index = amount - 1;
			close_runner(runner_index);
			return build_runner(location_index);
		}
	}
	inline short int receive(short int runner_index,std::string &reply);
	std::string error_msg(short int flag, short int index, std::string msg);
	std::string reply_error_msg(short int index, std::string msg, std::string reply);
	inline void error_fire(std::string msg);
	inline short int up(short int runner_index);
	short int fire_return(short int &location_index,std::string &msg, short int &runner_bk);
	virtual inline short int get_runner(short int location_index){
		short int runner_index;
		for(runner_index=0;runner_index<amount;++runner_index){
			if(location_list[runner_index]==location_index){
				runner_index = up(runner_index);
				break;
			}
		}
		if(runner_index==amount){
			return -1;
		}
		else{
			return runner_index;
		}
	}
	virtual short int write(short int &runner_index, short int location_index, std::string &msg){
		if( send(socketfd[runner_index] , msg.c_str() , msg.length() , 0) < 0){
			close_runner(runner_index);
			runner_index = get_runner(location_index);
			if(runner_index==-1){
				runner_index = build_runner(location_index);
			}
			if(runner_index==-1){
				msg = error_msg(1,location_index,msg);
				error_fire(msg);
				msg.clear();
				return 1;
			}
			if( send(socketfd[runner_index] , msg.c_str() , msg.length() , 0) < 0){
				close_runner(runner_index);
				runner_index = -1;
				msg = error_msg(2,location_index,msg);
				error_fire(msg);
				msg.clear();
				return 2;
			}
		}
		return 0;
	}
	virtual bool read(short int runner_index, std::string &reply_str){
		reply_str.clear();
		int len;
		char reply[transport_size];
		for(;;){
			len = recv(socketfd[runner_index], reply, transport_size, MSG_DONTWAIT);
			if(len>0){
				reply_str.append(reply,len);
			}
			else if(len==-1){
				return true;
			}
			else if(len==0){
				reply_str.clear();
				return false;
			}
		}
	}
public:
	client_core();
	~client_core();
	short int fire(short int &location_index,std::string &msg);
	short int fireNclose(short int &location_index,std::string &msg);
	short int fireNfreeze(short int &location_index,std::string &msg);
	short int fireNcheck(short int &location_index,std::string &msg);
	short int unfreeze(short int &location_index);
	short int uncheck(short int &location_index);
	short int shutdown_server(short int &location_index,std::string &shutdown_code);
	void import_location(std::vector<std::string> *location, short int unsigned amount = 0);
	void set_wait_maximum(short int wait);
	void set_error_node(short int socket_index,const char* file_name);
	std::vector<std::string> get_error_flag();
	
};
#include <hast/client_core.cpp>
#endif /* hast_client_core_hpp */

