#ifndef hast_socket_server_hpp
#define hast_socket_server_hpp
#include <hast/server_thread.hpp>
#include <cstring>
#include <map>
#include <list>

#define MAX_EVENTS 10
namespace hast{
	class socket_server : public hast::server_thread{
	protected:
		struct sockaddr_storage client_addr;
		socklen_t client_addr_size;
		int epollfd;
		short int unsigned topology_wait {2000};
		bool got_it {true};
		const int listen_pending{50};
		const int transport_size{100};
		const int resize_while_loop{20};
		struct epoll_event ev,ev_tmp, events[MAX_EVENTS];
		int host_socket {0};

		std::list<int> pending_fd;
		std::list<std::string> pending_msg;
		int pending_amount {0};
		std::mutex wait_mx;
		std::timed_mutex check_mx,freeze_mx;

		std::string check_str {"<>"};
		std::string freeze_str {"!"};

		/**
		 * RETURN true:  At least one pending is sending out.
		 * RETURN false: No pending was sent out.
		 **/
		void pending_first();
		void close_socket(const int socket_index);
		inline void recv_epoll();
	
	public:
		std::function<void(const int)> on_close {nullptr};
		socket_server();
		~socket_server();
		bool msg_recv(const short int thread_index);
		void start_accept();
		void set_topology_wait(short int unsigned time);
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
