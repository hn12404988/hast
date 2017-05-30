client_thread::~client_thread(){
	if(store_reply!=nullptr){
		delete [] store_reply;
		store_reply = nullptr;
	}
	if(waiting!=nullptr){
		delete [] waiting;
		waiting = nullptr;
	}
	recycle_thread();
}

void client_thread::import_location(std::vector<std::string> *location, short int unsigned amount){
	short int a;
	client_core::import_location(location,amount);
	if(waiting==nullptr){
		waiting = new bool [client_core::amount];
		for(a=0;a<client_core::amount;++a){
			waiting[a] = false;
		}
	}
	if(store_reply==nullptr){
		store_reply = new std::string* [client_core::amount];
		for(a=0;a<client_core::amount;++a){
			store_reply[a] = nullptr;
		}
	}
}

bool client_thread::multi_con(short int &location_index, short int unsigned amount){
	short int i,j;
	if(amount>client_core::amount){
		amount = client_core::amount;
	}
	j = 0;
	i = client_core::amount-1;
	for(;i>=0;--i){
		if(location_list[i]==location_index){
			++j;
		}
	}
	i = client_core::amount-1;
	for(;i>=0;--i){
		if(j==amount){
			break;
		}
		if(build_runner(location_index)==-1){
			return false;
		}
		++j;
	}
	if(j!=amount){
		return false;
	}
	return true;
}

inline bool client_thread::recycle_thread(){
	if(thread!=nullptr){
		if(thread->joinable()==true){
			thread->join();
			delete thread;
			thread = nullptr;
			return true;
		}
		else{
			error_fire(error_msg(hast_client::JOIN,-1,"Thread joinable is false"));
			return false;
		}
	}
	return true;
}

char client_thread::fire_thread(short int &location_index, std::string &msg, std::string *reply){
	short int runner_index;
	char a;
	if(on_air==false){
		if(recycle_thread()==false){
			msg.clear();
			return hast_client::JOIN;
		}
	}
	runner_index = get_runner(location_index);
	if(runner_index==-1){
		runner_index = build_runner(location_index);
	}
	if(runner_index==-1){
		error_fire(error_msg(hast_client::EXIST,location_index,"fire_thread"));
		msg.clear();
		return hast_client::EXIST;
	}
	a = write(runner_index,location_index,msg);
	if(a!=hast_client::SUCCESS){
		return a;
	}
	mx.lock();
	if(on_air==false){
		mx.unlock();
		if(recycle_thread()==false){
			close_runner(runner_index);
			msg.clear();
			return hast_client::JOIN;
		}
		waiting[runner_index] = true;
		store_reply[runner_index] = reply;
		on_air = true;
		thread = new std::thread([&]{this->recv_epoll_thread();});
	}
	else{
		waiting[runner_index] = true;
		store_reply[runner_index] = reply;
		mx.unlock();
	}
	return hast_client::SUCCESS;
}

char client_thread::fireNstore(short int &location_index,std::string &msg){
	return fire_thread(location_index,msg,&msg);
}

char client_thread::fireNforget(short int &location_index,std::string &msg){
	return fire_thread(location_index,msg,nullptr);
}

inline short int client_thread::get_runner(short int location_index){
	short int i;
	i = amount-1;
	if(on_air==true){
		for(;i>=0;--i){
			if(location_list[i]==location_index && waiting[i]==false){
				break;
			}
		}
		return i;
	}
	else{
		for(;i>=0;--i){
			if(location_list[i]==location_index){
				break;
			}
		}
		return i;
	}
}

void client_thread::recv_epoll_thread(){
	short int l,m;
	char a;
	std::string tmp_str;
	bool wait_history [amount];
	bool init {false};
	for(m=0;m<amount;++m){
		wait_history[m] = false;
	}
	for(;;){
		mx.lock();
		for(m=0;m<amount;++m){
			if(waiting[m]==true){
				break;
			}
		}
		if(m==amount){
			on_air = false;
			mx.unlock();
			break;
		}
		mx.unlock();
		l = epoll_wait(epollfd, events, MAX_EVENTS, wait_maximum);
		if(l==0){
			if(init==false){
				for(m=0;m<amount;++m){
					wait_history[m] = waiting[m];
				}
				init = true;
			}
			else{
				for(m=0;m<amount;++m){
					if(waiting[m]==true){
						if(wait_history[m]==true){
							if(store_reply[m]!=nullptr){
								tmp_str = error_msg(hast_client::REPLY,location_list[m],*store_reply[m]);
								store_reply[m]->clear();
							}
							else{
								tmp_str = error_msg(hast_client::REPLY,location_list[m],"fireNforget");
							}
							close_runner(m);
							error_fire(tmp_str);
							waiting[m] = false;
							wait_history[m] = false;
							store_reply[m] = nullptr;
						}
						else{
							wait_history[m] = true;
						}
					}
				}
			}
			continue;
		}
		if(l>0){
			--l;
			for(;l>=0;--l){
				for(m=0;m<amount;++m){
					if(waiting[m]==false){
						continue;
					}
					if(events[l].data.fd==socketfd[m]){
						if(events[l].events!=1){
							if(store_reply[m]!=nullptr){
								tmp_str = error_msg(hast_client::EPOLL_EV,location_list[m],*store_reply[m]);
								store_reply[m]->clear();
							}
							else{
								tmp_str = error_msg(hast_client::EPOLL_EV,location_list[m],"fireNforget");
							}
							close_runner(m);
							error_fire(tmp_str);
							waiting[m] = false;
							wait_history[m] = false;
							store_reply[m] = nullptr;
							m = -1;
							break;
						}
						a = read(m,tmp_str);
						break;
					}
				}
				if(m==-1){
					continue;
				}
				if(a!=hast_client::SUCCESS){
					if(store_reply[m]!=nullptr){
						tmp_str = error_msg(a,location_list[m],*store_reply[m]);
						store_reply[m]->clear();
					}
					else{
						tmp_str = error_msg(a,location_list[m],"Server crash (client_thread)");
					}
					close_runner(m);
					error_fire(tmp_str);
				}
				else{
					if(tmp_str==""){
						if(store_reply[m]!=nullptr){
							tmp_str = error_msg(hast_client::REPLY,location_list[m],*store_reply[m]);
							store_reply[m]->clear();
						}
						else{
							tmp_str = error_msg(hast_client::REPLY,location_list[m],"No reply (client_thread)");
						}
						close_runner(m);
						error_fire(tmp_str);
					}
					else{
						if(tmp_str[0]=='0'){
							if(store_reply[m]==nullptr){
								tmp_str = reply_error_msg(location_list[m],"fireNforget",tmp_str);
							}
							else{
								tmp_str = reply_error_msg(location_list[m],*store_reply[m],tmp_str);
								*store_reply[m] = "0";
							}
							error_fire(tmp_str);
						}
						else{
							if(store_reply[m]!=nullptr){
								*store_reply[m] = tmp_str;
							}
						}
					}
				}
				waiting[m] = false;
				wait_history[m] = false;
				store_reply[m] = nullptr;
				continue;
			}
		}
		else{
			tmp_str = strerror(errno);
			tmp_str = error_msg(hast_client::EPOLL,-1,tmp_str);
			error_fire(tmp_str);
			mx.lock();
			for(m=0;m<amount;++m){
				if(waiting[m]==true){
					close_runner(m);
					waiting[m] = false;
					if(store_reply[m]!=nullptr){
						store_reply[m]->clear();
						store_reply[m] = nullptr;
					}
				}
			}
			on_air = false;
			mx.unlock();
			break;
		}
	}
}

void client_thread::join(short int &location_index){
	short int a;
	a = amount-1;
	for(;a>=0;--a){
		if(location_list[a]==location_index && waiting[a]==true){
			++a;
		}
	}
}
