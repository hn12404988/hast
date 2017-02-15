#ifndef server_thread_h
#define server_thread_h
#include <mutex>
#include <thread>

class server_thread{
protected:
	short int i,j, alive_socket {0}, alive_thread{0};
	short int max_amount {0};
	short int recv_thread {-1};
	std::mutex recv_mx;

	std::vector<bool> in_execution;
	std::vector<std::thread*> thread_list;
	std::vector<std::string> raw_msg_bk;
	std::vector<bool> check_entry;

	short int all_freeze {-1}; //for socket_index
	short int msg_freeze {-1}; //for socket_index
	short int section_check {-1}; //for socket_index
	std::string check_str {"<>"};
	std::string freeze_str {"!"};

	inline void get_thread();
	inline void resize();
	inline void add_thread();
public:
	std::function<void(const short int)> execute;
	std::vector<int> socketfd;
	std::vector<std::string> raw_msg;
	bool anti_data_racing {false};
	bool check_data_racing {false};
	bool freeze {false};
	~server_thread();
};

#include <hast/server_thread.cpp>
#endif
