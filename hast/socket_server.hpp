#ifndef hast_socket_server_hpp
#define hast_socket_server_hpp
#include <hast/server_thread.hpp>
#include <cstring>
#include <map>

#define MAX_EVENTS 10
namespace hast{
	class socket_server : public hast::server_thread{
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

		void close_socket(const int socket_index);
		inline void recv_epoll();
	
	public:
		socket_server();
		~socket_server();
		bool msg_recv(const short int thread_index);
		void start_accept();
		void done(const short int thread_index);
		void close_socket(const short int thread_index);
		int get_socket(short int thread_index);
		inline void echo_back_msg(const short int thread_index, const char* msg);
		inline void echo_back_msg(const short int thread_index, std::string &msg);
		inline void echo_back_error(const short int thread_index, std::string msg);
		inline void echo_back_sql_error(const short int thread_index);
		inline void echo_back_result(const short int thread_index, bool error);
		inline void check_in(const short int thread_index, std::string &msg);
		inline void check_out(const short int thread_index);
	};
};
#include <hast/socket_server.cpp>
#endif /* hast_socket_server_hpp */
