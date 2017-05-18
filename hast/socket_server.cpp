namespace hast{
	socket_server::socket_server(){
		client_addr_size = sizeof(client_addr);
		epollfd = epoll_create1(0);
		ev.events = EPOLLIN;
		ev_tmp.events = EPOLLHUP;
	}

	socket_server::~socket_server(){
		close(epollfd);
	}

	void socket_server::done(const short int thread_index){
		/**
		 * This is here for threads break msg_recv loop accidentally.
		 * Threads are only allowed to break msg_recv loop by get 'false' from it.
		 **/
		status[thread_index] = hast::RECYCLE;
	}

	inline void socket_server::recv_epoll(){
		short int a,b,wait_amount {0};
		int c,loop_amount {0};
		while(got_it==false){}
		for(;;){
			/* TODO Find a way to clear `anti`.
			if(alive_thread==1){
				anti.clear();
			}
			*/
			b = socketfd.size()-1;
			wait_amount = 0;
			for(;b>=0;--b){
				if(status[b]==hast::READ){
					++b;
					continue;
				}
				else if(status[b]==hast::WAIT){
					++wait_amount;
				}
			}
			wait_mx.lock();
			if(wait_amount>1){
				++loop_amount;
				if(loop_amount>resize_while_loop){
					resize(wait_amount-1);
				}
				else{
					resize(0);
				}
			}
			else{
				resize(0);
				loop_amount = 0;
			}
			for(;;){
				a = epoll_wait(epollfd, events, MAX_EVENTS, 3500);
				if(a>0){
					wait_mx.unlock();
					break;
				}
			}
			--a;
			for(;a>=0;--a){
				c = events[a].data.fd;
				if(events[a].events!=1){
					epoll_ctl(epollfd, EPOLL_CTL_DEL, c,nullptr);
					shutdown(c,SHUT_RDWR);
					close(c);
					if(on_close!=nullptr){
						on_close(c);
					}
					continue;
				}
				b = get_thread();
				if(b==-1){
					break;
				}
				got_it = false;
				socketfd[b] = c;
				status[b] = hast::READ;
				ev_tmp.data.fd = c;
				epoll_ctl(epollfd, EPOLL_CTL_MOD, c,&ev_tmp);
				if(b!=recv_thread){
					while(got_it==false){}
				}
			}
			if(a>=0){
				for(;a>=0;--a){
					add_thread();
				}
				break;
			}
			else{
				if(b==recv_thread){
					add_thread();
					break;
				}
			}
		}
		resize(0);
		recv_thread = -1;
	}

	bool socket_server::msg_recv(const short int thread_index){
		if(anti_data_racing==true){
			anti[raw_msg_bk[thread_index]].unlock();
		}
		for(;;){
			if(anti_data_racing==true || freeze==true){
				raw_msg_bk[thread_index].clear();
			}
			if(check_data_racing==true){
				check_entry[thread_index] = false;
			}
			raw_msg[thread_index].clear();
			thread_mx.lock();
			if(socketfd[thread_index]>=0){
				ev.data.fd = socketfd[thread_index];
				epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd[thread_index],&ev);
				socketfd[thread_index] = -1;
			}
			status[thread_index] = hast::WAIT;
			if(recv_thread==-1){
				recv_thread = thread_index;
				thread_mx.unlock();
				recv_epoll();
			}
			else{
				thread_mx.unlock();
			}
			for(;;){
				if(status[thread_index]==hast::READ){
					break;
				}
				else if(status[thread_index]==hast::RECYCLE){
					return false;
				}
				else if(recv_thread==-1){
					if(status[thread_index]==hast::WAIT){
						break;
					}
				}
				else{
					wait_mx.lock();
					wait_mx.unlock();
				}
			}
			if(status[thread_index]==hast::WAIT){
				continue;
			}
			got_it = true;
			int l;
			char new_char[transport_size];
			for(;;){
				l = recv(socketfd[thread_index], new_char, transport_size, 0);
				if(l>0){
					raw_msg[thread_index].append(new_char,l);
				}
				else{
					break;
				}
			}
			if(raw_msg[thread_index]==""){
				//client close connection.
				close_socket(socketfd[thread_index]);
				continue;
			}
			if(check_data_racing==true){
				if(raw_msg[thread_index][0]=='<' && raw_msg[thread_index].back()=='>'){
					if(raw_msg[thread_index].length()==2){
						if(section_check==socketfd[thread_index]){
							check_str = "<>";
							section_check = -1;
							send(socketfd[thread_index], "1", 1,0);
						}
						else{
							send(socketfd[thread_index], "0", 1,0);
						}
						continue;
					}
					raw_msg[thread_index].pop_back();
					raw_msg[thread_index] = raw_msg[thread_index].substr(1);
					check_mx.lock();
					while(section_check>=0){}
					section_check = socketfd[thread_index];
					check_str.clear();
					l = socketfd.size()-1;
					for(;l>=0;--l){
						if(check_entry[l]==true){
							++l;
						}
					}
					check_str = raw_msg[thread_index];
					check_mx.unlock();
					send(socketfd[thread_index], "1", 1,0);
					continue;
				}
			}
			if(freeze==true){
				raw_msg_bk[thread_index] = raw_msg[thread_index];
				if(raw_msg[thread_index].back()=='!'){
					if(raw_msg[thread_index].length()==1){
						if(all_freeze==socketfd[thread_index]){
							all_freeze = -1;
							send(socketfd[thread_index], "1", 1,0);
						}
						else if(msg_freeze==socketfd[thread_index]){
							freeze_str = "!";
							msg_freeze = -1;
							send(socketfd[thread_index], "1", 1,0);
						}
						else{
							send(socketfd[thread_index], "0", 1,0);
						}
						continue;
					}
					if(raw_msg[thread_index]=="!!"){
						freeze_mx.lock();
						while(all_freeze>=0){}
						all_freeze = socketfd[thread_index];
						l = socketfd.size()-1;
						for(;l>=0;--l){
							if(status[l]==hast::BUSY && socketfd[l]>=0){
								++l;
							}
						}
						freeze_mx.unlock();
					}
					else{
						raw_msg[thread_index].pop_back();
						freeze_mx.lock();
						while(msg_freeze>=0){}
						msg_freeze = socketfd[thread_index];
						freeze_str = raw_msg[thread_index];
						l = socketfd.size()-1;
						for(;l>=0;--l){
							if(status[l]==hast::BUSY && raw_msg_bk[l]==freeze_str){
								++l;
							}
						}
						freeze_mx.unlock();
					}
					send(socketfd[thread_index], "1", 1,0);
					continue;
				}
				while(raw_msg[thread_index]==freeze_str){}
				while(all_freeze>=0){}
			}
			if(anti_data_racing==true){
				raw_msg_bk[thread_index] = raw_msg[thread_index];
				anti[raw_msg[thread_index]].lock();
			}
			status[thread_index] = hast::BUSY;
			return true;
		}
	}

	void socket_server::close_socket(const int socket_index){
		int a;
		shutdown(socket_index,SHUT_RDWR);
		close(socket_index);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, socket_index,nullptr);
		a = socketfd.size()-1;
		for(;a>=0;--a){
			if(socketfd[a]==socket_index){
				break;
			}
		}
		if(check_data_racing==true){
			if(section_check==socket_index){
				check_str = "<>";
				section_check = -1;
			}
			if(a>=0){
				check_entry[a] = false;
			}
		}
		if(freeze==true){
			if(all_freeze==socket_index){
				all_freeze = -1;
			}
			if(msg_freeze==socket_index){
				msg_freeze = -1;
				freeze_str = "!";
			}
			if(a>=0){
				raw_msg_bk[a].clear();
			}
		}
		if(anti_data_racing==true){
			if(a>=0){
				raw_msg_bk[a].clear();
			}
		}
		if(a>=0){
			raw_msg[a].clear();
			socketfd[a] = -1;
		}
	}

	void socket_server::close_socket(const short int thread_index){
		close_socket(socketfd[thread_index]);
	}
	
	int socket_server::get_socket(short int thread_index){
		return socketfd[thread_index];
	}

	void socket_server::start_accept(){
		int new_socket {1};
		while(new_socket>=0){
			new_socket = accept4(host_socket, (struct sockaddr *)&client_addr, &client_addr_size,SOCK_NONBLOCK);
			if(new_socket>0){
				ev.data.fd = new_socket;
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, new_socket,&ev)==-1){
					continue;
				}
				if(recv_thread==-1){
					add_thread();
				}
			}
		}
	}

	inline void socket_server::echo_back_msg(const short int thread_index, const char* msg){
		send(socketfd[thread_index], msg, strlen(msg),0);
	}

	inline void socket_server::echo_back_msg(const short int thread_index, std::string &msg){
		send(socketfd[thread_index], msg.c_str(), msg.length(),0);
	}

	inline void socket_server::echo_back_error(const short int thread_index, std::string msg){
		if(msg[0]=='[' || msg[0]=='{'){
			msg = "0{\"Error\":"+msg+"}";
		}
		else{
			msg = "0{\"Error\":\""+msg+"\"}";
		}
		send(socketfd[thread_index], msg.c_str(), msg.length(),0);
	}
	
	inline void socket_server::echo_back_sql_error(const short int thread_index){
		send(socketfd[thread_index], "0{\"Error\":\"SQL\"}", 16,0);
	}

	inline void socket_server::echo_back_result(const short int thread_index, bool error){
		if(error==true){
			send(socketfd[thread_index], "0", 1,0);
		}
		else{
			send(socketfd[thread_index], "1", 1,0);
		}
	}

	inline void socket_server::check_in(const short int thread_index, std::string &msg){
		while(section_check>=0){
			while(check_str==""){}
			while(msg==check_str){}
			break;
		}
		check_entry[thread_index] = true;
	}

	inline void socket_server::check_out(const short int thread_index){
		check_entry[thread_index] = false;
	}
};
