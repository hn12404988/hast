client_thread::~client_thread(){
	reset_thread();
	recycle_thread();
}

void client_thread::multi_con(short int &location_index, short int unsigned amount){
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
	while(j<amount){
		for(;i>=0;--i){
			if(j==amount){
				break;
			}
			client_thread::build_runner(location_index);
			++j;
		}
	}
}

inline void client_thread::clear_waiting(){
	short int a;
	a = waiting.size();
	if(a>0){
		--a;
		for(;a>=0;--a){
			if(waiting[a]>=0){
				str = error_send(5,location_list[waiting[a]],"clear_waiting");
				client_thread::error_fire(str);
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

inline void client_thread::build_runner(short int &location_index){
	i = amount-1;
	for(;i>=0;--i){
		if(socketfd[i]==-1){
			if(build_on_i(location_index)==true){
				location_list[i] = location_index;
			}
			else{
				i = -1;
			}
			return;
		}
	}
	i = amount - 1;
	for(;i>=0;--i){
		j = waiting.size()-1;
		for(;j>=0;--j){
			if(waiting[j]==i){
				break;
			}
		}
		if(j==-1){
			break;
		}
	}
	if(i>=0){
		close_runner(i);
		client_thread::build_runner(location_index);
	}
	else{
		++amount;
		socketfd.push_back(-1);
		location_list.push_back(-1);
		client_thread::build_runner(location_index);
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
			str = "Thread jam";
			str = error_send(9,-1,str);
			client_thread::error_fire(str);
			return false;
		}
	}
	return true;
}

short int client_thread::fire_thread(short int &location_index, std::string &msg, std::string *reply){
	short int i_index,w_index;
	if(on_air==false){
		clear_waiting();
		if(recycle_thread()==false){
			msg.clear();
			return 9;
		}
	}
	search_runner(location_index);
	if(i==-1){
		client_thread::build_runner(location_index);
	}
	if(i==-1){
		str = "fire_thread";
		str = error_send(1,location_index,str);
		client_thread::error_fire(str);
		msg.clear();
		return 1;
	}
	i_index = i;
	j = waiting.size()-1;
	for(;j>=0;--j){
		if(waiting[j]==-1){
			break;
		}
	}
	if(j==-1){
		waiting.push_back(-1);
		store_reply.push_back(nullptr);
		j = waiting.size()-1;
	}
	w_index = j;
	if( send(socketfd[i] , msg.c_str() , msg.length() , 0) < 0){
		close_runner(i);
		search_runner(location_index);
		if(i==-1){
			client_thread::build_runner(location_index);
		}
		if(i==-1){
			str = "fail on sending in fire_thread";
			str = error_send(1,location_index,str);
			client_thread::error_fire(str);
			msg.clear();
			return 1;
		}
		i_index = i;
		if( send(socketfd[i] , msg.c_str() , msg.length() , 0) < 0){
			close_runner(i);
			str = error_send(2,location_index,msg);
			client_thread::error_fire(str);
			msg.clear();
			return 2;
		}
	}
	mx.lock();
	if(on_air==false){
		mx.unlock();
		if(recycle_thread()==false){
			close_runner(i_index);
			clear_waiting();
			msg.clear();
			return 9;
		}
		waiting[w_index] = i_index;
		store_reply[w_index] = reply;
		on_air = true;
		epoll_0_thread = false;
		thread = new std::thread([&]{this->recv_epoll();});
	}
	else{
		waiting[w_index] = i_index;
		store_reply[w_index] = reply;
		mx.unlock();
	}
	return 0;
}

short int client_thread::fireNstore(short int &location_index,std::string &msg){
	return fire_thread(location_index,msg,&msg);
}

short int client_thread::fireNforget(short int &location_index,std::string &msg){
	return fire_thread(location_index,msg,nullptr);
}

short int client_thread::fire(short int &location_index,std::string &msg){
	mx.lock();
	if(msg==""){
		str = error_send(7,location_index,"Empty");
		mx.unlock();
		client_thread::error_fire(str);
		return 7;
	}
	search_runner(location_index);
	if(i==-1){
		client_thread::build_runner(location_index);
	}
	if(i==-1){
		str = "client_thread::fire";
		msg = error_send(1,location_index,str);
		mx.unlock();
		client_thread::error_fire(msg);
		msg.clear();
		return 1;
	}
	else{
		if( send(socketfd[i] , msg.c_str() , msg.length() , 0) < 0){
			close_runner(i);
			search_runner(location_index);
			if(i==-1){
				client_thread::build_runner(location_index);
			}
			if(i==-1){
				str = "fail on sending in client_thread::fire";
				msg = error_send(1,location_index,str);
				mx.unlock();
				client_thread::error_fire(msg);
				msg.clear();
				return 1;
			}
			if( send(socketfd[i] , msg.c_str() , msg.length() , 0) < 0){
				close_runner(i);
				msg = error_send(2,location_index,msg);
				mx.unlock();
				client_thread::error_fire(msg);
				msg.clear();
				return 2;
			}
		}
	}
	short int tmp_j;
	tmp_j = receive(msg);
	if(tmp_j>0){
		str = error_send(j,location_index,msg);
		close_runner(i);
		client_thread::error_fire(str);
		msg.clear();
	}
	else{
		if(str==""){
			str = error_send(4,location_index,msg);
			close_runner(i);
			client_thread::error_fire(str);
			msg.clear();
			tmp_j = 4;
		}
		else{
			if(str[0]=='0'){
				str = reply_error_send(location_index, msg,str);
				client_thread::error_fire(str);
				msg = "0";
			}
			else{
				msg = str;
			}
			tmp_j = 0;
		}
	}
	mx.unlock();
	return tmp_j;
}

inline void client_thread::error_fire(std::string msg){
	if(msg!=""){
		if(error_socket_index!=-1){
			client_thread::fire(error_socket_index,msg);
		}
	}
}

short int client_thread::fireNclose(short int &location_index,std::string &msg){
	j = client_thread::fire(location_index,msg);
	if(j==0){
		close_runner(i);
		return 0;
	}
	else{
		i = j;
		return i;
	}
}

short int client_thread::fireNfreeze(short int &location_index,std::string &msg){
	if(msg==""){
		str = error_send(7,location_index,"Empty");
		client_thread::error_fire(str);
		return 7;
	}
	msg.append("!");
	return client_thread::fire(location_index,msg);
}

short int client_thread::unfreeze(short int &location_index){
	str = "!";
	return client_thread::fire(location_index,str);
}

short int client_thread::fireNcheck(short int &location_index,std::string &msg){
	if(msg==""){
		str = error_send(7,location_index,"Empty");
		client_thread::error_fire(str);
		return 7;
	}
	msg = "<"+msg+">";
	return client_thread::fire(location_index,msg);
}

short int client_thread::uncheck(short int &location_index){
	str = "<>";
	return client_thread::fire(location_index,str);
}

inline void client_thread::search_runner(short int &location_index){
	i = amount-1;
	for(;i>=0;--i){
		if(location_list[i]==location_index){
			j = waiting.size()-1;
			for(;j>=0;--j){
				if(waiting[j]==i){
					break;
				}
			}
			if(j==-1){
				break;
			}
		}
	}
}

void client_thread::recv_epoll(){
	int k,l,m;
	char reply_thread[transport_size];
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
			epoll_0_thread = true;
			mx.unlock();
			break;
		}
		mx.unlock();
		l = epoll_wait(epollfd, events, MAX_EVENTS, 1000*wait_maximum);
		if(l==0){
			epoll_0_thread = true;
			continue;
		}
		if(l>0){
			--l;
			for(;l>=0;--l){
				m = waiting.size()-1;
				for(;m>=0;--m){
					if(waiting[m]<0){
						continue;
					}
					if(events[l].data.fd==socketfd[waiting[m]]){
						if(events[l].events!=1){
							if(store_reply[m]!=nullptr){
								tmp_str = error_send(10,location_list[waiting[m]],*store_reply[m]);
								store_reply[m]->clear();
							}
							else{
								tmp_str = error_send(10,location_list[waiting[m]],"fireNforget");
							}
							close_runner(waiting[m]);
							client_thread::error_fire(tmp_str);
							waiting[m] = -1;
							m = -1;
							break;
						}
						tmp_str.clear();
						for(;;){
							k = recv(events[l].data.fd, reply_thread, transport_size, MSG_DONTWAIT);
							if(k>0){
								k += tmp_str.length();
								tmp_str.append(reply_thread);
								tmp_str.resize(k);
								k = 0;
								continue;
							}
							else if(k==-1){
								k = 0;
								break;
							}
							else if(k==0){
								k = 3;
								break;
							}
						}
						break;
					}
				}
				if(m==-1){
					continue;
				}
				if(k==3){
					if(store_reply[m]!=nullptr){
						tmp_str = error_send(3,location_list[waiting[m]],*store_reply[m]);
						store_reply[m]->clear();
					}
					else{
						tmp_str = error_send(3,location_list[waiting[m]],"Server crash (client_thread)");
					}
					close_runner(waiting[m]);
					client_thread::error_fire(tmp_str);
				}
				else{
					if(tmp_str==""){
						if(store_reply[m]!=nullptr){
							tmp_str = error_send(4,location_list[waiting[m]],*store_reply[m]);
							store_reply[m]->clear();
						}
						else{
							tmp_str = error_send(4,location_list[waiting[m]],"No reply (client_thread)");
						}
						close_runner(waiting[m]);
						client_thread::error_fire(tmp_str);
					}
					else{
						if(tmp_str[0]=='0'){
							if(store_reply[m]==nullptr){
								tmp_str = reply_error_send(location_list[waiting[m]],"fireNforget",tmp_str);
							}
							else{
								tmp_str = reply_error_send(location_list[waiting[m]],*store_reply[m],tmp_str);
								*store_reply[m] = "0";
							}
							client_thread::error_fire(tmp_str);
						}
						else{
							if(store_reply[m]!=nullptr){
								*store_reply[m] = tmp_str;
							}
						}
					}
				}
				waiting[m] = -1;
				continue;
			}
		}
		else{
			tmp_str = strerror(errno);
			tmp_str = error_send(6,-1,tmp_str);
			client_thread::error_fire(tmp_str);
			mx.lock();
			on_air = false;
			epoll_0_thread = true;
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
		epoll_0_thread = false;
	}
	else{
		epoll_0_thread = true;
	}
	mx.unlock();
	while(epoll_0_thread==false){}
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
				str = error_send(4,location_index,"client_thread::join");
				client_thread::error_fire(str);
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
		epoll_0_thread = false;
	}
	else{
		epoll_0_thread = true;
	}
	while(epoll_0_thread==false){}
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
				str = error_send(4,location_index,"client_thread::join_store");
				client_thread::error_fire(str);
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
		epoll_0_thread = false;
	}
	else{
		epoll_0_thread = true;
	}
	while(epoll_0_thread==false){}
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
				str = error_send(4,location_index,"client_thread::join_forget");
				client_thread::error_fire(str);
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
		epoll_0_thread = false;
	}
	else{
		epoll_0_thread = true;
	}
	mx.unlock();
	while(epoll_0_thread==false){}
	b = waiting.size()-1;
	for(;b>=0;--b){
		a = waiting[b];
		if(a>=0){
			str = error_send(4,location_list[a],"client_thread::reset_thread");
			close_runner(a);
			waiting[b] = -1;
			client_thread::error_fire(str);
		}
	}
}
