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

	void socket_server::set_topology_wait(short int unsigned time){
		topology_wait = time;
	}

	void socket_server::set_shutdown_code(std::string code){
		shutdown_code = code;
	}
	
	void socket_server::done(const short int thread_index){
		/**
		 * This is here for threads break msg_recv loop accidentally.
		 * Threads are only allowed to break msg_recv loop by getting 'false' from it.
		 **/
		status[thread_index] = hast::RECYCLE;
	}

	void socket_server::pending_first(){
		if(pending_amount==0){
			return;
		}
		short int b;
		while(pending_amount>0){
			b = get_thread();
			if(b==-1){
				break;
			}
			raw_msg[b] = pending_msg.front();
			socketfd[b] = pending_fd.front();
			pending_msg.pop_front();
			pending_fd.pop_front();
			--pending_amount;
			got_it = false;
			status[b] = hast::READ_PREFIX;
			if(b!=recv_thread){
				while(got_it==false){}
			}
		}
	}
	
	inline void socket_server::recv_epoll(){
		short int a,b,wait_amount {0};
		int c,loop_amount {0};
		while(got_it==false){}
		for(;;){
			if(pending_amount>0){
				if(msg_freeze_fd==-1 && section_check_fd==-1){
					pending_first();
					if(status[recv_thread]!=hast::WAIT){
						break;
					}
				}
			}
			wait_amount = 0;
			for(b=0;b<max_thread;++b){
				if(status[b]==hast::WAIT){
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
				if(pending_amount==0){
					a = epoll_wait(epollfd, events, MAX_EVENTS, 3500);
				}
				else{
					a = epoll_wait(epollfd, events, MAX_EVENTS, 1);
				}
				if(byebye==true){
					wait_mx.unlock();
					break;
				}
				if(pending_amount>0){
					if(msg_freeze_fd==-1 && section_check_fd==-1){
						wait_mx.unlock();
						pending_first();
						if(status[recv_thread]==hast::WAIT){
							if(a>0){
								break;
							}
							else{
								wait_mx.lock();
								continue;
							}
						}
						else{
							break;
						}
					}
				}
				if(a>0){
					wait_mx.unlock();
					break;
				}
			}
			if(byebye==true){
				break;
			}
			if(status[recv_thread]!=hast::WAIT){
				break;
			}
			--a;
			for(;a>=0;--a){
				c = events[a].data.fd;
				if(events[a].events!=1){
					close_socket(c,__LINE__);
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
		for(;;){
			if(section_check==true){
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
			if(byebye==false && recv_thread==-1){
				recv_thread = thread_index;
				thread_mx.unlock();
				recv_epoll();
			}
			else{
				thread_mx.unlock();
			}
			for(;;){
				if(status[thread_index]==hast::READ || status[thread_index]==hast::READ_PREFIX){
					break;
				}
				else if(status[thread_index]==hast::RECYCLE || byebye==true){
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
			if(msg_freeze_fd==-1 && msg_freeze_id==thread_index){
				freeze_mx.unlock();
				msg_freeze_id = -1;
			}
			if(section_check_fd==-1 && section_check_id==thread_index){
				check_mx.unlock();
				section_check_id = -1;
			}
			if(status[thread_index]==hast::WAIT){
				continue;
			}
			got_it = true;
			int l;
			if(status[thread_index]==hast::READ){
				char new_char[transport_size];
				for(;;){
					l = recv(socketfd[thread_index], new_char, transport_size, 0);
					if(l>0){
						raw_msg[thread_index].append(new_char,l);
					}
					else if(l==0){
						break;
					}
					else{
						l = 1;
						break;
					}
				}
				if(l==0){
					//client close connection.
					close_socket(socketfd[thread_index],__LINE__);
					continue;
				}
				if(call_shutdown==true){
					if(raw_msg[thread_index]==shutdown_code){
						byebye = true;
						send(socketfd[thread_index], "1", 1,0);
						continue;
					}
				}
			}
			else{
			}
			if(section_check==true){
				if(raw_msg[thread_index][0]=='<' && raw_msg[thread_index].back()=='>'){
					/**
					 * signal
					 **/
					if(raw_msg[thread_index].length()==2){
						/**
						 * `uncheck` signal
						 **/
						//if(section_check_fd==socketfd[thread_index]){
						if(section_check_fd>=0){
							section_check_fd = -1;
							check_str = "<>";
							send(socketfd[thread_index], "1", 1,0);
						}
						else{
							//Something wrong, mainly because the time of check period exceed `topology_wait`.
							send(socketfd[thread_index], "0{\"Error\":\"uncheck fail\"}", 25,0);
						}
						if(section_check_id==thread_index){
							check_mx.unlock();
							section_check_id = -1;
						}
					}
					else{
						/**
						 * `check` signal
						 **/
						if(check_mx.try_lock()==true){
							//No another signal is `section checking`, so this signal can start to initiate.
							raw_msg[thread_index].pop_back();
							raw_msg[thread_index] = raw_msg[thread_index].substr(1);
						}
						else{
							if(section_check_fd==-1){
								//Last signal is over but mutex is locked.
								if(section_check_id==thread_index){
									//This thread is here again, so keep locking anyway.
									raw_msg[thread_index].pop_back();
									raw_msg[thread_index] = raw_msg[thread_index].substr(1);
								}
								else{
									//Need lock thread to deal with this msg, so pending this signal
									pending_fd.push_back(socketfd[thread_index]);
									pending_msg.push_back(raw_msg[thread_index]);
									//socketfd[thread_index] = -1; //Don't open epoll
									++pending_amount;
									//TODO echo back how long should that client wait.
									continue;
								}
							}
							else{
								//There is a signal is `section-checking`, so pending this signal.
								pending_fd.push_back(socketfd[thread_index]);
								pending_msg.push_back(raw_msg[thread_index]);
								//socketfd[thread_index] = -1; //Don't open epoll
								++pending_amount;
								//TODO echo back how long should that client wait.
								continue;
							}
						}
						section_check_fd = socketfd[thread_index];
						section_check_id = thread_index;
						check_str.clear();
						for(l=0;l<max_thread;++l){
							if(check_entry[l]==true){
								--l;
							}
						}
						check_str = raw_msg[thread_index];
						//check_mx.unlock();
						send(socketfd[thread_index], "1", 1,0);
					}
					continue;
				}
			}
			if(msg_freeze==true){
				if(raw_msg[thread_index].back()=='!'){
					/**
					 * This msg is just a signal.
					 **/
					if(raw_msg[thread_index].length()==1){
						/**
						 * This is a `unfreeze` signal.
						 **/
						//if(msg_freeze_fd==socketfd[thread_index]){
						if(msg_freeze_fd>=0){
							msg_freeze_fd = -1;
							freeze_str = "!";
							send(socketfd[thread_index], "1", 1,0);
						}
						else{
							//Something wrong, mainly because the time of check period exceed `topology_wait`.
							send(socketfd[thread_index], "0{\"Error\":\"unfreeze fail\"}", 26,0);
						}
						if(msg_freeze_id==thread_index){
							freeze_mx.unlock();
							msg_freeze_id = -1;
						}
					}
					else{
						/**
						 * This is a `freeze` signal.
						 **/
						if(freeze_mx.try_lock()==true){
							//No another signal is in `freeze`, so this signal can start to initiate.
							raw_msg[thread_index].pop_back();
						}
						else{
							if(msg_freeze_fd==-1){
								//Last signal is over but mutex is locked.
								if(msg_freeze_id==thread_index){
									//This thread is here again, so keep locking anyway.
									raw_msg[thread_index].pop_back();
								}
								else{
									//Need lock thread to deal with this msg, so pending this signal.
									pending_fd.push_back(socketfd[thread_index]);
									pending_msg.push_back(raw_msg[thread_index]);
									//socketfd[thread_index] = -1; //Don't open epoll
									++pending_amount;
									//TODO echo back how long should that client wait.
									//pending first
									continue;
								}
							}
							else{
								//There is a signal is in `freeze`, so pending this signal.
								pending_fd.push_back(socketfd[thread_index]);
								pending_msg.push_back(raw_msg[thread_index]);
								//socketfd[thread_index] = -1; //Don't open epoll
								++pending_amount;
								//TODO echo back how long should that client wait.
								continue;
							}
						}
						if(raw_msg[thread_index]=="!"){
							freeze_str.clear();
							msg_freeze_id = thread_index;
						}
						else{
							freeze_str = raw_msg[thread_index];
							msg_freeze_id = -1;
						}
						msg_freeze_fd = socketfd[thread_index];
						for(l=0;l<max_thread;++l){
							if(status[l]==hast::BUSY && socketfd[l]>=0){
								--l;
							}
						}
						if(msg_freeze_fd==socketfd[thread_index]){
							send(socketfd[thread_index], "1", 1,0);
						}
						else{
							send(socketfd[thread_index], "0{\"Error\":\"init too long\"}", 26,0);
							msg_freeze_fd = -1;
							freeze_str = "!";
							msg_freeze_id = -1;
						}
						if(freeze_str.length()>0){
							// `msg_freeze` doesn't use mutex.
							freeze_mx.unlock();
						}
					}
					continue;
				}
				/**
				 * Here is the place where msg pass through or back to pending list.
				 **/
				if(msg_freeze_fd>=0){
					if(freeze_str.length()==0){ //freeze_str==""
						//all-freeze
						if(msg_freeze_id==thread_index){
							pending_fd.push_back(socketfd[thread_index]);
							pending_msg.push_back(raw_msg[thread_index]);
							//socketfd[thread_index] = -1; //Don't open epoll
							++pending_amount;
							continue;
						}
						else{
							if(freeze_mx.try_lock_for(std::chrono::milliseconds(topology_wait))==false){
								//This `all-freeze` is too long, so abandon this `freeze`.
								msg_freeze_fd = -1;
								freeze_str = "!";
								if(msg_freeze_id==thread_index){
									freeze_mx.unlock();
									msg_freeze_id = -1;
								}
							}
							else{
								freeze_mx.unlock();
							}
						}
					}
					else{
						//msg-freeze
						freeze_mx.lock(); //Wait for signal's initiation
						freeze_mx.unlock(); //Wait for signal's initiation
						if(raw_msg[thread_index]==freeze_str){
							pending_fd.push_back(socketfd[thread_index]);
							pending_msg.push_back(raw_msg[thread_index]);
							//socketfd[thread_index] = -1; //Don't open epoll
							++pending_amount;
							continue;
						}
					}
				}
				else{
					if(msg_freeze_id==thread_index){
						//Lock thread is here, so unlock and clean this signal asap.
						freeze_mx.unlock();
						msg_freeze_id = -1;
					}
				}
			}
			status[thread_index] = hast::BUSY;
			return true;
		}
	}

	void socket_server::close_socket(const int socket_index, int line){
		if(socket_index<0){
			return;
		}
		std::cout << "close fd: " << socket_index << " by line: " << line << std::endl;
		short int a;
		shutdown(socket_index,SHUT_RDWR);
		close(socket_index);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, socket_index,nullptr);
		for(a=0;a<max_thread;++a){
			if(socketfd[a]==socket_index){
				break;
			}
		}
		if(a==max_thread){
			a = -1;
		}
		if(section_check==true){
			if(section_check_fd==socket_index){
				section_check_fd = -1;
				check_str = "<>";
			}
			if(a>=0){
				if(section_check_id==a){
					check_mx.unlock();
					section_check_id = -1;
				}
				check_entry[a] = false;
			}
		}
		if(msg_freeze==true){
			if(msg_freeze_fd==socket_index){
				msg_freeze_fd = -1;
				freeze_str = "!";
			}
			if(a>=0){
				if(msg_freeze_id==a){
					freeze_mx.unlock();
					msg_freeze_id = -1;
				}
			}
		}
		if(a>=0){
			raw_msg[a].clear();
			socketfd[a] = -1;
		}
		if(on_close!=nullptr){
			on_close(socket_index);
		}
	}

	void socket_server::close_socket(const short int thread_index){
		close_socket(socketfd[thread_index],__LINE__);
	}
	
	void socket_server::start_accept(){
		int new_socket {1};
		while(new_socket>=0){
			new_socket = accept4(host_socket, (struct sockaddr *)&client_addr, &client_addr_size,SOCK_NONBLOCK);
			if(new_socket>0){
				if(byebye==true){
					send(new_socket,"1",1,0);
					break;
				}
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
		if(section_check_fd>=0){
			while(check_str==""){
				//signal is initiating
			}
			/**
			 * if(section_check_id==thread_index){
			 *     This thread is the thread received this `section-check` signal,
			 *     and lock `check_mx` mutex. It means that if this thread try_lock_for()
			 *     `check_mx` mutex will cause `undefined behavior`.
			 *     But gcc 5.4 will let this situation act like any other thread, so we
			 *     let all thread do the same thing here.
			 *     Please keep in mind that each compiler may have different behavior here.
			 * }
			 * else{
			 *     Other threads.
			 * }
			 **/
			if(check_str==raw_msg[thread_index]){
				if(check_mx.try_lock_for(std::chrono::milliseconds(topology_wait))==false){
					//This `section-check` is too long, so abandon this `section-check`.
					section_check_fd = -1;
					check_str = "<>";
					if(section_check_id==thread_index){
						//Lock thread is here, so unlock and clean this signal asap.
						check_mx.unlock();
						section_check_id = -1;
					}
				}
				else{
					//`section-check` is over.
					check_mx.unlock();
				}
			}
		}
		else{
			if(section_check_id==thread_index){
				//Lock thread is here, so unlock and clean this signal asap.
				check_mx.unlock();
				section_check_id = -1;
			}
		}
		check_entry[thread_index] = true;
	}

	inline void socket_server::check_out(const short int thread_index){
		check_entry[thread_index] = false;
	}
};
