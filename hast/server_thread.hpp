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
		short int max_amount {0}, recv_thread {-1};
		std::mutex thread_mx;

		std::vector<char> status;
		std::vector<std::thread*> thread_list;
		std::vector<std::string> raw_msg_bk;
		std::vector<bool> check_entry;

		int all_freeze {-1}; //for socket_index
		int msg_freeze {-1}; //for socket_index
		int section_check {-1}; //for socket_index
		std::string check_str {"<>"};
		std::string freeze_str {"!"};

		short int get_thread();
		short int get_thread_no_recv();
		inline void resize(short int amount);
		inline void add_thread();
	public:
		std::function<void(const short int)> execute {nullptr};
		std::vector<int> socketfd;
		std::vector<std::string> raw_msg;
		bool anti_data_racing {false};
		bool check_data_racing {false};
		bool freeze {false};
	};
};
#include <hast/server_thread.cpp>
#endif /* hast_server_thread_hpp */
