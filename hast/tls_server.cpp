tls_server::~tls_server(){
	std::map<int,SSL*>::iterator it;
	std::map<int,SSL*>::iterator it_end;
	it = ssl_map.begin();
	it_end = ssl_map.end();
	for(;it!=it_end;++it){
		if(it->second!=nullptr){
			SSL_free(it->second);
			it->second = nullptr;
		}
	}
	ssl_map.clear();
	if(ctx!=nullptr){
		SSL_CTX_free(ctx);
		ctx = nullptr;
	}
	if(ssl!=nullptr){
		delete [] ssl;
		ssl = nullptr;
	}
	//CONF_modules_unload(1);
	//CONF_modules_free();
	//ENGINE_cleanup();
	sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_remove_state(0);
	ERR_free_strings();
}


void tls_server::reset_accept(int socket_index,SSL *ssl){
	shutdown(socket_index,SHUT_RDWR);
	close(socket_index);
	SSL_free(ssl);
}

void tls_server::close_socket(const short int thread_index){
	close_socket(socketfd[thread_index],__LINE__);
}

void tls_server::close_socket(const int socket_index, int line){
	socket_server::close_socket(socket_index,line);
	ssl_mx.lock();
	if(ssl_map[socket_index]!=nullptr){
		SSL_free(ssl_map[socket_index]);
		ssl_map[socket_index] = nullptr;
	}
	ssl_mx.unlock();
}

void tls_server::pending_first(){
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
		ssl[b] = ssl_map[socketfd[b]];
		pending_msg.pop_front();
		pending_fd.pop_front();
		--pending_amount;
		if(ssl[b]==nullptr){
			status[b] = hast::WAIT;
			socketfd[b] = -1;
			raw_msg[b].clear();
			continue;
		}
		got_it = false;
		status[b] = hast::READ_PREFIX;
		if(b!=recv_thread){
			while(got_it==false){}
		}
	}
}

inline bool tls_server::read(short int thread_index){
	int l;
	char new_char[transport_size];
	ssl_mx.lock();
	for(;;){
		l = SSL_read(ssl[thread_index], new_char, transport_size);
		if(l>0){
			raw_msg[thread_index].append(new_char,l);
		}
		else if(l==0){
			ssl_mx.unlock();
			close_socket(socketfd[thread_index],__LINE__);
			return false;
		}
		else{
			l = SSL_get_error(ssl[thread_index],l);
			if(l==SSL_ERROR_WANT_READ || l==SSL_ERROR_WANT_WRITE){
				ssl_mx.unlock();
				return true;
			}
			else{
				ssl_mx.unlock();
				close_socket(socketfd[thread_index],__LINE__);
				return false;
			}
		}
	}
}

inline bool tls_server::write(short int thread_index, std::string &msg){
	int len {msg.length()},flag;
	const char* cmsg {msg.c_str()};
	/*
	std::cout << "write get fd: " << SSL_get_fd(ssl[thread_index]) << std::endl;
	std::cout << "write fd: " << socketfd[thread_index] << std::endl;
	std::cout << "write msg: " << msg << std::endl;
	*/
	ssl_mx.lock();
	for(;;){
		flag = SSL_write(ssl[thread_index], cmsg, len);
		if(flag>0){
			if(flag==len){
				ssl_mx.unlock();
				return true;
			}
			else{
				msg = msg.substr(flag);
				cmsg = msg.c_str();
				len = msg.length();
			}
		}
		else if(flag==0){
			ssl_mx.unlock();
			close_socket(socketfd[thread_index],__LINE__);
			return false;
		}
		else{
			flag = SSL_get_error(ssl[thread_index],flag);
			if(flag==SSL_ERROR_WANT_READ || flag==SSL_ERROR_WANT_WRITE){
				continue;
			}
			else{
				ssl_mx.unlock();
				close_socket(socketfd[thread_index]);
				return false;
			}
		}
	}
}

inline bool tls_server::write(short int thread_index, const char* msg){
	std::string tmp_str(msg);
	return write(thread_index,tmp_str);
}

inline void tls_server::recv_epoll(){
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
			if(ssl_map[c]==nullptr){
				close_socket(c,__LINE__);
				continue;
			}
			b = get_thread();
			if(b==-1){
				break;
			}
			got_it = false;
			socketfd[b] = c;
			ssl[b] = ssl_map[c];
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

inline bool tls_server::write(SSL* ssl_ptr, const char* cmsg){
	std::string msg(cmsg);
	int len {msg.length()},flag;
	cmsg = msg.c_str();
	ssl_mx.lock();
	for(;;){
		flag = SSL_write(ssl_ptr, cmsg, len);
		if(flag>0){
			if(flag==len){
				ssl_mx.unlock();
				return true;
			}
			else{
				msg = msg.substr(flag);
				cmsg = msg.c_str();
				len = msg.length();
			}
		}
		else if(flag==0){
			SSL_free(ssl_ptr);
			ssl_mx.unlock();
			return false;
		}
		else{
			flag = SSL_get_error(ssl_ptr,flag);
			if(flag==SSL_ERROR_WANT_READ || flag==SSL_ERROR_WANT_WRITE){
				continue;
			}
			else{
				SSL_free(ssl_ptr);
				ssl_mx.unlock();
				return false;
			}
		}
	}
}

void tls_server::start_accept(){
	if(execute==nullptr || ctx==nullptr){
		return;
	}
	SSL *ssl_ptr {SSL_new(ctx)};
	int l;
	int new_socket {1};
	while(new_socket>=0){
		new_socket = accept4(host_socket, (struct sockaddr *)&client_addr, &client_addr_size, SOCK_NONBLOCK);
		if(new_socket>0){
			//std::cout << "new socket: " << new_socket << std::endl;
			if(ssl_ptr==nullptr){
				ssl_ptr = SSL_new(ctx);
			}
			l = SSL_set_fd(ssl_ptr, new_socket);
			for(;;){
				l = SSL_accept(ssl_ptr);
				if (l <= 0) {
					l = SSL_get_error(ssl_ptr,l);
					if (l == SSL_ERROR_WANT_READ){
						continue;
						//std::cout << "Wait for data to be read" << l << std::endl;
					}
					else if (l == SSL_ERROR_WANT_WRITE){
						//std::cout << "Write data to continue" << l << std::endl;
					}
					else if (l == SSL_ERROR_SYSCALL){
						//std::cout << "SSL_ERROR_SYSCALL" << l << std::endl;
					}
					else if(l == SSL_ERROR_SSL){
						//std::cout << "SSL_ERROR_SSL" << l << std::endl;
					}
					else if (l == SSL_ERROR_ZERO_RETURN){
						//std::cout << "Same as error" << l << std::endl;
					}
					//ERR_print_errors_fp(stderr);
					reset_accept(new_socket,ssl_ptr);
					ssl_ptr = nullptr;
					l = -1;
					break;
				}
				else{
					break;
				}
			}
			if(l==-1){
				if(byebye==true){
					break;
				}
				else{
					continue;
				}
			}
			if(byebye==true){
				write(ssl_ptr,"1");
				break;
			}
			//std::cout << "SSL_accept OK" << std::endl;
			if(ssl_map[new_socket]!=nullptr){
				reset_accept(new_socket,ssl_ptr);
				ssl_ptr = nullptr;
				ssl_map[new_socket] = nullptr;
				continue;
			}
			else{
				ssl_map[new_socket] = ssl_ptr;
			}
			ev.data.fd = new_socket;
			if(epoll_ctl(epollfd, EPOLL_CTL_ADD, new_socket,&ev)==-1){
				reset_accept(new_socket,ssl_ptr);
				ssl_ptr = nullptr;
				ssl_map[new_socket] = nullptr;
				continue;
			}
			ssl_ptr = nullptr;
			if(recv_thread==-1){
				add_thread();
			}
			//std::cout << "accept OK" << std::endl;
		}
	}
	if(ssl_ptr!=nullptr){
		SSL_free(ssl_ptr);
		ssl_ptr = nullptr;
	}
}
