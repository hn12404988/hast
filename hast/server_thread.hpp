#ifndef hast_server_thread_hpp
#define hast_server_thread_hpp
#include <mutex>
#include <thread>

namespace hast{
	const char WAIT {0};
	const char BUSY {1};
	const char RECYCLE {2};
	const char READ {3};
	const char READ_PREFIX {4};
	const char GET {5};
	
	class server_thread{
	protected:
		server_thread();
		~server_thread();
		short int max_thread {0}, recv_thread {-1};
		std::mutex thread_mx;

		char *status {nullptr};
		std::thread **thread_list {nullptr};
		bool *check_entry {nullptr};

		int msg_freeze_fd {-1};
		short int msg_freeze_id {-1};
		int section_check_fd {-1};
		short int section_check_id {-1};

		short int get_thread();
		short int get_thread_no_recv();
		void init();
		inline void resize(short int amount);
		inline void add_thread();
	public:
		std::function<void(const short int)> execute {nullptr};
		int *socketfd {nullptr};
		std::string *raw_msg {nullptr};
		bool section_check {false};
		bool msg_freeze {false};
	};
};
#include <hast/server_thread.cpp>
#endif /* hast_server_thread_hpp */
