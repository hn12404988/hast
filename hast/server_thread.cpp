namespace hast{
	server_thread::server_thread(){}
	server_thread::~server_thread(){
		if(status!=nullptr){
			short int a {0};
			for(;a<max_thread;++a){
				if(thread_list[a]!=nullptr){
					if(thread_list[a]->joinable()==true){
						thread_list[a]->join();
						delete thread_list[a];
					}
				}
			}
			delete [] status;
			delete [] thread_list;
			delete [] raw_msg;
			delete [] socketfd;
			if(raw_msg_bk!=nullptr){
				delete [] raw_msg_bk;
			}
			if(check_entry!=nullptr){
				delete [] check_entry;
			}
		}
	}
	void server_thread::init(){
		short int a;
		status = new char [max_thread];
		thread_list = new std::thread* [max_thread];
		raw_msg = new std::string [max_thread];
		socketfd = new int [max_thread];
		for(a=0;a<max_thread;++a){
			status[a] = hast::BUSY;
			thread_list[a] = nullptr;
			raw_msg[a] = "";
			socketfd[a] = -1;
		}
		if(anti_data_racing==true){
			raw_msg_bk = new std::string [max_thread];
			for(a=0;a<max_thread;++a){
				raw_msg_bk[a] = "";
			}
		}
		if(section_check==true){
			check_entry = new bool [max_thread];
			for(a=0;a<max_thread;++a){
				check_entry[a] = false;
			}
		}
	}
	
	inline void server_thread::resize(short int amount){
		short int a {0};
		for(;a<max_thread;++a){
			if(recv_thread==a){
				continue;
			}
			if(status[a]==hast::WAIT && thread_list[a]!=nullptr){
				if(msg_freeze_id!=a && section_check_id!=a){
					if(amount>0){
						status[a] = hast::RECYCLE;
						--amount;
						continue;
					}
				}
			}
			else if(status[a]==hast::RECYCLE){
				if(thread_list[a]->joinable()==true){
					thread_list[a]->join();
					delete thread_list[a];
					thread_list[a] = nullptr;
					status[a] = hast::BUSY;
					socketfd[a] = -1;
				}
				else{
					//TODO Something Wrong
				}
			}
		}
	}

	short int server_thread::get_thread(){
		thread_mx.lock();
		if(msg_freeze_id>=0){
			if(status[msg_freeze_id]==hast::WAIT){
				status[msg_freeze_id] = hast::GET;
				thread_mx.unlock();
				return msg_freeze_id;
			}
		}
		if(section_check_id>=0){
			if(status[section_check_id]==hast::WAIT){
				status[section_check_id] = hast::GET;
				thread_mx.unlock();
				return section_check_id;
			}
		}
		short int a {0};
		for(;a<max_thread;++a){
			if(recv_thread==a){
				continue;
			}
			if(status[a]==hast::WAIT){
				break;
			}
		}
		if(a==max_thread){
			if(status[recv_thread]==hast::WAIT){
				a = recv_thread;
				status[a] = hast::GET;
			}
			else{
				a = -1;
			}
		}
		else{
			status[a] = hast::GET;
		}
		thread_mx.unlock();
		return a;
	}

	short int server_thread::get_thread_no_recv(){
		thread_mx.lock();
		if(msg_freeze_id>=0){
			if(status[msg_freeze_id]==hast::WAIT){
				status[msg_freeze_id] = hast::GET;
				thread_mx.unlock();
				return msg_freeze_id;
			}
		}
		if(section_check_id>=0){
			if(status[section_check_id]==hast::WAIT){
				status[section_check_id] = hast::GET;
				thread_mx.unlock();
				return section_check_id;
			}
		}
		short int a {0};
		for(;a<max_thread;++a){
			if(recv_thread==a){
				continue;
			}
			if(status[a]==hast::WAIT){
				break;
			}
		}
		if(a<max_thread){
			status[a] = hast::GET;
		}
		else{
			a = -1;
		}
		thread_mx.unlock();
		return a;
	}
	
	inline void server_thread::add_thread(){
		short int a {0};
		thread_mx.lock();
		for(;a<max_thread;++a){
			if(thread_list[a]==nullptr){
				thread_list[a] = new std::thread(execute,a);
				break;
			}
		}
		thread_mx.unlock();
	}
};
