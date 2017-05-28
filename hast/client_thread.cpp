client_thread::~client_thread(){
	reset_thread();
	recycle_thread();
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
		if(client_thread::build_runner(location_index)==-1){
			return false;
		}
		++j;
	}
	if(j!=amount){
		return false;
	}
	return true;
}

inline void client_thread::clear_waiting(){
	short int a;
	a = waiting.size();
	if(a>0){
		--a;
		for(;a>=0;--a){
			if(waiting[a]>=0){
				error_fire(error_msg(5,location_list[waiting[a]],"clear_waiting"));
				close_runner(waiting[a]);
				waiting[a] = -1;
				if(store_reply[a]!=nullptr){
					store_reply[a]->clear();
					store_reply[a] = nullptr;
				}
			}
		}
	}
}

inline short int client_thread::build_runner(short int location_index){
	short int i;
	int j;
	i = amount-1;
	for(;i>=0;--i){
		if(socketfd[i]==-1){
			if(build_on_i(i,location_index)==true){
				location_list[i] = location_index;
				return i;
			}
			else{
				return -1;
			}
		}
	}
	i = amount - 1;
	if(on_air==true){
		for(;i>=0;--i){
			j = waiting.size();
			if(j>0){
				--j;
				for(;j>=0;--j){
					if(waiting[j]==i){
						break;
					}
				}
				if(j==-1){
					close_runner(i);
					return client_thread::build_runner(location_index);
				}
			}
			else{
				break;
			}
		}
		std::cout << "build_runner: " << __LINE__ << std::endl;
		while(on_air==true){}
		std::cout << "build_runner: " << __LINE__ << std::endl;
		clear_waiting();
		return client_thread::build_runner(location_index);
	}
	else{
		close_runner(i);
		return client_thread::build_runner(location_index);
	}
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
			error_fire(error_msg(9,-1,"Thread joinable is false"));
			return false;
		}
	}
	return true;
}

short int client_thread::fire_thread(short int &location_index, std::string &msg, std::string *reply){
	short int runner_index,wait_bk;
	int j;
	if(on_air==false){
		std::cout << __LINE__ << std::endl;
		clear_waiting();
		if(recycle_thread()==false){
			msg.clear();
			return 9;
		}
	}
	runner_index = get_runner(location_index);
	std::cout << __LINE__ << " : " << runner_index << std::endl;
	if(runner_index==-1){
		std::cout << __LINE__ << " : " << runner_index << std::endl;
		runner_index = client_thread::build_runner(location_index);
		std::cout << __LINE__ << " : " << runner_index << std::endl;
	}
	if(runner_index==-1){
		std::cout << __LINE__ << " : " << runner_index << std::endl;
		error_fire(error_msg(1,location_index,"fire_thread"));
		msg.clear();
		return 1;
	}
	std::cout << __LINE__ << " : " << runner_index << std::endl;
	j = waiting.size()-1;
	for(;j>=0;--j){
		if(waiting[j]==-1){
			break;
		}
	}
	std::cout << __LINE__ << " : " << runner_index << std::endl;
	if(j==-1){
		waiting.push_back(-1);
		store_reply.push_back(nullptr);
		wait_bk = waiting.size()-1;
	}
	else{
		wait_bk = j;
	}
	std::cout << __LINE__ << " : " << runner_index << std::endl;
	j = write(runner_index,location_index,msg);
	std::cout << __LINE__ << " : " << runner_index << std::endl;
	if(j>0){
		return j;
	}
	mx.lock();
	if(on_air==false){
		mx.unlock();
		if(recycle_thread()==false){
			close_runner(runner_index);
			clear_waiting();
			msg.clear();
			return 9;
		}
		waiting[wait_bk] = runner_index;
		store_reply[wait_bk] = reply;
		on_air = true;
		epoll_0 = false;
		std::cout << __LINE__ << " : " << runner_index << std::endl;
		thread = new std::thread([&]{this->recv_epoll_thread();});
		std::cout << __LINE__ << " : " << runner_index << std::endl;
	}
	else{
		std::cout << __LINE__ << " : " << runner_index << std::endl;
		waiting[wait_bk] = runner_index;
		store_reply[wait_bk] = reply;
		mx.unlock();
		std::cout << __LINE__ << " : " << runner_index << std::endl;
	}
	return 0;
}

short int client_thread::fireNstore(short int &location_index,std::string &msg){
	return fire_thread(location_index,msg,&msg);
}

short int client_thread::fireNforget(short int &location_index,std::string &msg){
	return fire_thread(location_index,msg,nullptr);
}

inline short int client_thread::get_runner(short int location_index){
	short int i;
	int j;
	i = amount-1;
	if(on_air==true){
		for(;i>=0;--i){
			if(location_list[i]==location_index){
				j = waiting.size();
				if(j>0){
					--j;
					for(;j>=0;--j){
						if(waiting[j]==i){
							break;
						}
					}
					if(j==-1){
						return i;
					}
				}
				else{
					return i;
				}
			}
		}
		return -1;
	}
	else{
		for(;i>=0;--i){
			if(location_list[i]==location_index){
				return i;
			}
		}
		return -1;
	}
}

void client_thread::recv_epoll_thread(){
	int k,l,m;
	std::string tmp_str;
	for(;;){
		mx.lock();
		l = waiting.size()-1;
		for(;l>=0;--l){
			if(waiting[l]>=0){
				break;
			}
		}
		if(l==-1){
			on_air = false;
			epoll_0 = true;
			mx.unlock();
			break;
		}
		mx.unlock();
		l = epoll_wait(epollfd, events, MAX_EVENTS, wait_maximum);
		if(l==0){
			epoll_0 = true;
			std::cout << "epoll 0" << std::endl;
			continue;
		}
		if(l>0){
			std::cout << "recv_epoll l: " << l << std::endl;
			--l;
			for(;l>=0;--l){
				m = waiting.size()-1;
				for(;m>=0;--m){
					if(waiting[m]<0){
						continue;
					}
					if(events[l].data.fd==socketfd[waiting[m]]){
						std::cout << "recv_epoll: " << __LINE__ << std::endl;
						if(events[l].events!=1){
							if(store_reply[m]!=nullptr){
								tmp_str = error_msg(10,location_list[waiting[m]],*store_reply[m]);
								store_reply[m]->clear();
								std::cout << "recv_epoll: " << __LINE__ << std::endl;
							}
							else{
								tmp_str = error_msg(10,location_list[waiting[m]],"fireNforget");
								std::cout << "recv_epoll: " << __LINE__ << std::endl;
							}
							close_runner(waiting[m]);
							error_fire(tmp_str);
							waiting[m] = -1;
							store_reply[m] = nullptr;
							m = -1;
							break;
						}
						if(read(waiting[m],tmp_str)==false){
							std::cout << "recv_epoll: " << __LINE__ << std::endl;
							k = 3;
						}
						else{
							std::cout << "recv_epoll: " << __LINE__ << std::endl;
							k = 0;
						}
						break;
					}
				}
				if(m==-1){
					std::cout << "recv_epoll: " << __LINE__ << std::endl;
					continue;
				}
				if(k==3){
					if(store_reply[m]!=nullptr){
						tmp_str = error_msg(3,location_list[waiting[m]],*store_reply[m]);
						store_reply[m]->clear();
					}
					else{
						tmp_str = error_msg(3,location_list[waiting[m]],"Server crash (client_thread)");
					}
					close_runner(waiting[m]);
					error_fire(tmp_str);
				}
				else{
					if(tmp_str==""){
						if(store_reply[m]!=nullptr){
							tmp_str = error_msg(4,location_list[waiting[m]],*store_reply[m]);
							store_reply[m]->clear();
						}
						else{
							tmp_str = error_msg(4,location_list[waiting[m]],"No reply (client_thread)");
						}
						close_runner(waiting[m]);
						error_fire(tmp_str);
					}
					else{
						if(tmp_str[0]=='0'){
							if(store_reply[m]==nullptr){
								tmp_str = reply_error_msg(location_list[waiting[m]],"fireNforget",tmp_str);
							}
							else{
								tmp_str = reply_error_msg(location_list[waiting[m]],*store_reply[m],tmp_str);
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
				waiting[m] = -1;
				store_reply[m] = nullptr;
				continue;
			}
		}
		else{
			tmp_str = strerror(errno);
			tmp_str = error_msg(6,-1,tmp_str);
			error_fire(tmp_str);
			mx.lock();
			on_air = false;
			epoll_0 = true;
			mx.unlock();
			break;
		}
	}
}

bool client_thread::join(short int &location_index){
	short int a,b,w_size;
	bool boo {true};
	mx.lock();
	if(on_air==true){
		epoll_0 = false;
	}
	else{
		epoll_0 = true;
	}
	mx.unlock();
	while(epoll_0==false){}
	a = amount-1;
	w_size = waiting.size();
	for(;a>=0;--a){
		if(location_list[a]==location_index){
			for(b=0;b<w_size;++b){
				if(waiting[b]==a){
					break;
				}
			}
			if(b<w_size){
				close_runner(a);
				waiting[b] = -1;
				store_reply[b] = nullptr;
				error_fire(error_msg(4,location_index,"client_thread::join"));
				boo = false;
			}
		}
	}
	return boo;
}

bool client_thread::join_store(short int &location_index){
	short int a,b,w_size;
	bool boo {true};
	if(on_air==true){
		epoll_0 = false;
	}
	else{
		epoll_0 = true;
	}
	while(epoll_0==false){}
	w_size = waiting.size();
	a = amount-1;
	for(;a>=0;--a){
		if(location_list[a]==location_index){
			for(b=0;b<w_size;++b){
				if(waiting[b]==a && store_reply[b]!=nullptr){
					break;
				}
			}
			if(b<w_size){
				close_runner(a);
				waiting[b] = -1;
				store_reply[b] = nullptr;
				error_fire(error_msg(4,location_index,"client_thread::join_store"));
				boo = false;
			}
		}
	}
	return boo;
}

bool client_thread::join_forget(short int &location_index){
	short int a,b,w_size;
	bool boo {true};
	if(on_air==true){
		epoll_0 = false;
	}
	else{
		epoll_0 = true;
	}
	while(epoll_0==false){}
	w_size = waiting.size();
	a = amount-1;
	for(;a>=0;--a){
		if(location_list[a]==location_index){
			for(b=0;b<w_size;++b){
				if(waiting[b]==a && store_reply[b]==nullptr){
					break;
				}
			}
			if(b<w_size){
				close_runner(a);
				waiting[b] = -1;
				error_fire(error_msg(4,location_index,"client_thread::join_forget"));
				boo = false;
			}
		}
	}
	return boo;
}

void client_thread::reset_thread(){
	short int a,b;
	mx.lock();
	if(on_air==true){
		epoll_0 = false;
	}
	else{
		epoll_0 = true;
	}
	mx.unlock();
	while(epoll_0==false){}
	b = waiting.size()-1;
	for(;b>=0;--b){
		a = waiting[b];
		if(a>=0){
			close_runner(a);
			waiting[b] = -1;
			store_reply[b] = nullptr;
			error_fire(error_msg(4,location_list[a],"client_thread::reset_thread"));
		}
	}
}
