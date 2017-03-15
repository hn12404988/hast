#ifndef socket_server_h
#define socket_server_h
#include <hast/server_thread.h>
#include <cstring>
#include <map>

#define MAX_EVENTS 10

class socket_server : public server_thread{
protected:
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;
	int epollfd;
	bool got_it {true};
	const int listen_pending{50};
	const int transport_size{100};
	struct epoll_event ev,ev_tmp, events[MAX_EVENTS];
	int host_socket {0};
	
	std::mutex waiting_mx,freeze_mx,check_mx;
	std::map<std::string,std::mutex> anti;

	inline void close_socket(const int socket_index);
	inline void recv_epoll();
	
public:
	socket_server();
	~socket_server();
	bool msg_recv(const short int thread_index);
	void start_accept();
	void done(const short int thread_index);
	int get_socket(short int thread_index);
	inline void echo_back_msg(const int socket_index, const char* msg);
	inline void echo_back_msg(const int socket_index, std::string &msg);
	inline void echo_back_error(const int socket_index, std::string msg);
	inline void check_in(const short int thread_index, std::string &msg);
	inline void check_out(const short int thread_index);
};

#include <hast/socket_server.cpp>
#endif
