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
		if(recv_thread==thread_index){
			recv_thread = -1;
		}
		in_execution[thread_index] = true;
		socketfd[thread_index] = -2;
	}

	inline void socket_server::recv_epoll(){
		while(got_it==false){}
		for(;;){
			resize();
			if(alive_thread==1){
				anti.clear();
			}
			waiting_mx.lock();
			for(;;){
				i = epoll_wait(epollfd, events, MAX_EVENTS, 3500);
				if(i>0){
					waiting_mx.unlock();
					break;
				}
			}
			if(all_freeze>=0 || msg_freeze>=0 || section_check>=0){
				j = i;
			}
			else{
				j = i - alive_thread + 1;
			}
			for(;j>0;--j){
				add_thread();
			}
			--i;
			for(;i>=0;--i){
				if(events[i].events!=1){
					close_socket(events[i].data.fd);
					continue;
				}
				get_thread();
				if(j==-1){
					break;
				}
				got_it = false;
				socketfd[j] = events[i].data.fd;
				ev_tmp.data.fd = events[i].data.fd;
				epoll_ctl(epollfd, EPOLL_CTL_MOD, events[i].data.fd,&ev_tmp);
				if(j!=recv_thread){
					while(got_it==false){}
				}
			}
			if(i>=0){
				break;
			}
			else{
				if(socketfd[recv_thread]>=0){
					break;
				}
			}
		}
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
			if(socketfd[thread_index]>=0){
				ev.data.fd = socketfd[thread_index];
				epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd[thread_index],&ev);
				socketfd[thread_index] = -1;
			}
			in_execution[thread_index] = false;
			recv_mx.lock();
			if(recv_thread==-1){
				recv_thread = thread_index;
				recv_mx.unlock();
				recv_epoll();
			}
			else{
				recv_mx.unlock();
			}
			for(;;){
				if(socketfd[thread_index]>=0){
					break;
				}
				else if(socketfd[thread_index]==-2){
					return false;
				}
				else if(recv_thread==-1){
					break;
				}
				else{
					waiting_mx.lock();
					waiting_mx.unlock();
				}
			}
			if(socketfd[thread_index]==-1){
				continue;
			}
			got_it = true;
			int l;
			char new_char[transport_size];
			for(;;){
				l = recv(socketfd[thread_index], new_char, transport_size, 0);
				if(l>0){
					l += raw_msg[thread_index].length();
					raw_msg[thread_index].append(new_char);
					raw_msg[thread_index].resize(l);
					l = 0;
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
							if(in_execution[l]==true && socketfd[l]>=0){
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
							if(in_execution[l]==true && raw_msg_bk[l]==freeze_str){
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
			in_execution[thread_index] = true;
			return true;
		}
	}

	inline void socket_server::close_socket(const int socket_index){
		--alive_socket;
		int a;
		epoll_ctl(epollfd, EPOLL_CTL_DEL, socket_index,nullptr);
		shutdown(socket_index,SHUT_RDWR);
		close(socket_index);
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
				++alive_socket;
			}
		}
	}

	inline void socket_server::echo_back_msg(const int socket_index, const char* msg){
		send(socket_index, msg, strlen(msg),0);
	}

	inline void socket_server::echo_back_msg(const int socket_index, std::string &msg){
		send(socket_index, msg.c_str(), msg.length(),0);
	}

	inline void socket_server::echo_back_error(const int socket_index, std::string msg){
		if(msg[0]=='[' || msg[0]=='{'){
			msg = "0{\"Error\":"+msg+"}";
		}
		else{
			msg = "0{\"Error\":\""+msg+"\"}";
		}
		send(socket_index, msg.c_str(), msg.length(),0);
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
