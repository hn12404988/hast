namespace hast{
	server_thread::server_thread(){}
	server_thread::~server_thread(){
		if(status!=nullptr){
			short int a;
			for(;;){
				for(a=0;a<max_thread;++a){
					if(thread_list[a]!=nullptr){
						if(thread_list[a]->joinable()==true){
							thread_list[a]->join();
							delete thread_list[a];
							thread_list[a] = nullptr;
						}
						else{
							//Something Wrong
						}
					}
				}
				for(a=0;a<max_thread;++a){
					if(thread_list[a]!=nullptr){
						break;
					}
				}
				if(a==max_thread){
					break;
				}
			}
			if(status!=nullptr){
				delete [] status;
				status = nullptr;
			}
			if(thread_list!=nullptr){
				delete [] thread_list;
				thread_list = nullptr;
			}
			if(raw_msg!=nullptr){
				delete [] raw_msg;
				raw_msg = nullptr;
			}
			if(socketfd!=nullptr){
				delete [] socketfd;
				socketfd = nullptr;
			}
			if(check_entry!=nullptr){
				delete [] check_entry;
				check_entry = nullptr;
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
		if(msg_freeze_id>=0){
			if(status[msg_freeze_id]==hast::WAIT){
				status[msg_freeze_id] = hast::GET;
				return msg_freeze_id;
			}
		}
		if(section_check_id>=0){
			if(status[section_check_id]==hast::WAIT){
				status[section_check_id] = hast::GET;
				return section_check_id;
			}
		}
		short int a {0};
		if(recv_thread==-1){
			for(;a<max_thread;++a){
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
		}
		else{
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
		}
		return a;
	}

	short int server_thread::get_thread_no_recv(){
		if(msg_freeze_id>=0){
			if(status[msg_freeze_id]==hast::WAIT){
				status[msg_freeze_id] = hast::GET;
				return msg_freeze_id;
			}
		}
		if(section_check_id>=0){
			if(status[section_check_id]==hast::WAIT){
				status[section_check_id] = hast::GET;
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
		return a;
	}
	
	inline void server_thread::add_thread(){
		short int a {0};
		for(;a<max_thread;++a){
			if(thread_list[a]==nullptr){
				thread_list[a] = new std::thread(execute,a);
				break;
			}
		}
	}
};
