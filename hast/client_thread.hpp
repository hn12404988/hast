#ifndef hast_client_thread_hpp
#define hast_client_thread_hpp

#include <thread>
#include <mutex>
#include <hast/client_core.hpp>

class client_thread : public client_core{
protected:
	std::string **store_reply {nullptr}; //store nullptr for `fireNforget` and pointer for `fireNstore`
	bool *waiting {nullptr};
	std::thread *thread {nullptr}; //recv thread
	std::mutex mx;
	bool on_air {false}; // recv_thread is working or not.
	virtual inline short int build_runner(short int location_index) override;
	inline short int get_runner(short int location_index) override;
	inline bool recycle_thread();
	void recv_epoll_thread();
	/**
	 * `fireNstore` and `fireNforget` use `fire_thread` to fire message.
	 **/
	char fire_thread(short int &location_index, std::string &msg, std::string *reply);
public:
	~client_thread();
	void import_location(std::vector<std::string> *location, short int unsigned amount = 0);
	char fireNstore(short int &location_index, std::string &msg);
	char fireNforget(short int &location_index, std::string &msg);
	void join(short int &location_index);
	bool multi_con(short int &location_index, short int unsigned amount);
};
#include <hast/client_thread.cpp>
#endif /* hast_client_thread_hpp */

