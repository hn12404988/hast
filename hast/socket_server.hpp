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
		int unsigned pending_amount {0};
		std::mutex wait_mx,thread_mx;
		std::timed_mutex check_mx,freeze_mx;

		std::string check_str {"<>"};
		std::string freeze_str {"!"};
		std::string shutdown_code {""};
		bool byebye {false};

		virtual void close_socket(const int socket_index, int line = 0);
		void pending_first();
		inline void recv_epoll();
	
	public:
		std::function<void(const int)> on_close {nullptr};
		socket_server();
		~socket_server();
		bool call_shutdown {false};
		virtual bool msg_recv(const short int thread_index);
		virtual void start_accept();
		void set_shutdown_code(std::string code);
		void set_topology_wait(short int unsigned time);
		void done(const short int thread_index);
		void close_socket(const short int thread_index);
		inline void check_in(const short int thread_index, std::string &msg);
		inline void check_out(const short int thread_index);
		virtual inline void echo_back_msg(const short int thread_index, const char* msg);
		virtual inline void echo_back_msg(const short int thread_index, std::string &msg);
		virtual inline void echo_back_error(const short int thread_index, std::string msg);
		virtual inline void echo_back_sql_error(const short int thread_index);
		virtual inline void echo_back_result(const short int thread_index, bool error);
	};
};
#include <hast/socket_server.cpp>
#endif /* hast_socket_server_hpp */
