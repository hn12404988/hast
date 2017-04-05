#ifndef hast_client_thread_h
#define hast_client_thread_h

#include <thread>
#include <mutex>
#include <hast/client_core.hpp>

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

class client_thread : public client_core{
private:
	std::vector<std::string*> store_reply;
	std::vector<short int> waiting; //for i index
	std::thread *thread {nullptr};
	std::mutex mx;
	bool on_air {false};
	bool epoll_0_thread;
	inline void build_runner(short int &location_index);
	inline void clear_waiting();
	inline void search_runner(short int &location_index);
	inline bool recycle_thread();
	void recv_epoll();
	short int fire_thread(short int &location_index, std::string &msg, std::string *reply);
	inline void error_fire(std::string msg);
public:
	~client_thread();
	short int fire(short int &location_index,std::string &msg);
	short int fireNstore(short int &location_index, std::string &msg);
	short int fireNforget(short int &location_index, std::string &msg);
	short int fireNclose(short int &location_index,std::string &msg);
	short int fireNfreeze(short int &location_index,std::string &msg);
	short int fireNcheck(short int &location_index,std::string &msg);
	short int uncheck(short int &location_index);
	short int unfreeze(short int &location_index);
	bool join(short int &location_index);
	bool join_store(short int &location_index);
	bool join_forget(short int &location_index);
	void reset_thread();
	void multi_con(short int &location_index, short int unsigned amount);
};
#include <hast/client_thread.cpp>
#endif

