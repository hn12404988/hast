#ifndef hast_client_thread_hpp
#define hast_client_thread_hpp

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
protected:
	std::vector<std::string*> store_reply; //store nullptr for `fireNforget` and pointer for `fireNstore`
	std::vector<short int> waiting; //store runner_index
	std::thread *thread {nullptr}; //recv thread
	std::mutex mx;
	bool on_air {false}; // recv_thread is working or not.
	bool epoll_0 {false}; // epoll_wait return 0 in recv_thread is happened.
	inline short int build_runner(short int location_index) override;
	inline void clear_waiting();
	inline short int get_runner(short int location_index) override;
	inline bool recycle_thread();
	void recv_epoll_thread();
	/**
	 * `fireNstore` and `fireNforget` use `fire_thread` to fire message.
	 **/
	short int fire_thread(short int &location_index, std::string &msg, std::string *reply);
public:
	~client_thread();
	short int fireNstore(short int &location_index, std::string &msg);
	short int fireNforget(short int &location_index, std::string &msg);
	bool join(short int &location_index);
	bool join_store(short int &location_index);
	bool join_forget(short int &location_index);
	void reset_thread();
	bool multi_con(short int &location_index, short int unsigned amount);
};
#include <hast/client_thread.cpp>
#endif /* hast_client_thread_hpp */

