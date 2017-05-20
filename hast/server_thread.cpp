namespace hast{
	server_thread::server_thread(){}
	server_thread::~server_thread(){
		short int a;
		a = thread_list.size()-1;
		for(;a>=0;--a){
			if(thread_list[a]!=nullptr){
				if(thread_list[a]->joinable()==true){
					thread_list[a]->join();
					delete thread_list[a];
				}
			}
		}
	}

	inline void server_thread::resize(short int amount){
		short int a;
		a = socketfd.size()-1;
		for(;a>=0;--a){
			if(recv_thread==a){
				continue;
			}
			if(status[a]==hast::WAIT && thread_list[a]!=nullptr){
				if(amount>0){
					status[a] = hast::RECYCLE;
					--amount;
					continue;
				}
			}
			else if(status[a]==hast::RECYCLE){
				if(thread_list[a]->joinable()==true){
					thread_list[a]->join();
					delete thread_list[a];
					thread_list[a] = nullptr;
					status[a] = hast::BUSY;
				}
				else{
					//TODO Something Wrong
				}
			}
		}
	}

	short int server_thread::get_thread(){
		thread_mx.lock();
		short int a;
		a = socketfd.size()-1;
		for(;a>=0;--a){
			if(recv_thread==a){
				continue;
			}
			if(status[a]==hast::WAIT){
				break;
			}
		}
		if(a==-1){
			if(status[recv_thread]==hast::WAIT){
				a = recv_thread;
				status[a] = hast::GET;
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
		short int a;
		a = socketfd.size()-1;
		for(;a>=0;--a){
			if(recv_thread==a){
				continue;
			}
			if(status[a]==hast::WAIT){
				break;
			}
		}
		if(a>=0){
			status[a] = hast::GET;
		}
		thread_mx.unlock();
		return a;
	}
	
	inline void server_thread::add_thread(){
		short int a;
		thread_mx.lock();
		a = socketfd.size();
		if(a>0){
			--a;
			for(;a>=0;--a){
				if(thread_list[a]==nullptr){
					thread_list[a] = new std::thread(execute,a);
					break;
				}
			}
			if(a>=0){
				thread_mx.unlock();
				return;
			}
			else{
				a = socketfd.size();
				if(max_amount>0){
					if(anti_data_racing==true || check_data_racing==true || freeze==true){}
					else{
						if(a>=max_amount){
							thread_mx.unlock();
							return;
						}
					}
				}
			}
		}
		if(anti_data_racing==true || freeze==true){
			raw_msg_bk.push_back("");
		}
		if(check_data_racing==true){
			check_entry.push_back(false);
		}
		socketfd.push_back(-1);
		status.push_back(hast::BUSY);
		raw_msg.push_back("");
		thread_list.push_back(nullptr);
		thread_list[a] = new std::thread(execute,a);
		thread_mx.unlock();
	}
};
