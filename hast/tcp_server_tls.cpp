tcp_server_tls::~tcp_server_tls(){
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

bool tcp_server_tls::init(const char* crt, const char* key, hast::tcp_socket::port port, short int unsigned max){
	short int a;
	if(tcp_server::init(port, max)==false){
		return false;
	}
	const SSL_METHOD *method;
	SSL_load_error_strings();	
	OpenSSL_add_ssl_algorithms();
	SSL_library_init();
	method = SSLv23_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		return false;
	}
	//SSL_CTX_set_ecdh_auto(ctx, 1); This is removed in newer openssl.
	/* Set the key and cert */
	if (SSL_CTX_use_certificate_file(ctx, crt, SSL_FILETYPE_PEM) < 0) {
		ERR_print_errors_fp(stderr);
		return false;
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) < 0 ) {
		ERR_print_errors_fp(stderr);
		return false;
	}
	ssl = new SSL* [max_thread];
	for(a=0;a<max_thread;++a){
		ssl[a] = nullptr;
	}
	return true;
}

void tcp_server_tls::reset_accept(int socket_index,SSL *ssl){
	shutdown(socket_index,SHUT_RDWR);
	close(socket_index);
	SSL_free(ssl);
}

void tcp_server_tls::close_socket(const int socket_index){
	socket_server::close_socket(socket_index);
	SSL *tmp_ptr {nullptr};
	tmp_ptr = ssl_map[socket_index];
	if(tmp_ptr!=nullptr){
		ssl_mx.lock();
		SSL_free(tmp_ptr);
		ssl_map[socket_index] = nullptr;
		ssl_mx.unlock();
	}
}

void tcp_server_tls::start_accept(){
	if(execute==nullptr || ctx==nullptr){
		return;
	}
	SSL *ssl_ptr {SSL_new(ctx)};
	int l,loop;
	int new_socket {1};
	while(new_socket>=0){
		new_socket = accept4(host_socket, (struct sockaddr *)&client_addr, &client_addr_size, SOCK_NONBLOCK);
		if(new_socket>0){
			//std::cout << "new socket: " << new_socket << std::endl;
			if(ssl_ptr==nullptr){
				ssl_ptr = SSL_new(ctx);
			}
			l = SSL_set_fd(ssl_ptr, new_socket);
			loop = 0;
			for(;;){
				l = SSL_accept(ssl_ptr);
				if (l <= 0) {
					l = SSL_get_error(ssl_ptr,l);
					if (l == SSL_ERROR_WANT_READ){
						//std::cout << "Wait for data to be read" << l << std::endl;
						++loop;
						if(loop<10000){
							continue;
						}
						std::cout << "error read loop" << std::endl;
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
				ssl_mx.lock();
				SSL_write(ssl_ptr,"1",1);
				ssl_mx.unlock();
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

bool tcp_server_tls::msg_recv(const short int thread_index){
	//std::cout << "msg_recv start: " << thread_index << std::endl;
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
			//std::cout << "recv: " << thread_index << std::endl;
			recv_epoll();
			//std::cout << "recv over: " << thread_index << std::endl;
		}
		else{
			//std::cout << "not recv: " << thread_index << std::endl;
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
		ssl[thread_index] = ssl_map[socketfd[thread_index]];
		if(ssl[thread_index]==nullptr){
			close_socket(socketfd[thread_index]);
			continue;
		}
		int l;
		if(status[thread_index]==hast::READ){
			char new_char[transport_size];
			ssl_mx.lock();
			for(;;){
				l = SSL_read(ssl[thread_index], new_char, transport_size);
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
			ssl_mx.unlock();
			if(l==0){
				//client close connection.
				close_socket(socketfd[thread_index]);
				continue;
			}
			if(call_shutdown==true){
				if(raw_msg[thread_index]==shutdown_code){
					byebye = true;
					ssl_mx.lock();
					SSL_write(ssl[thread_index],"1",1);
					ssl_mx.unlock();
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
					if(section_check_fd==socketfd[thread_index]){
						section_check_fd = -1;
						check_str = "<>";
						ssl_mx.lock();
						SSL_write(ssl[thread_index],"1",1);
						ssl_mx.unlock();
					}
					else{
						//Something wrong, mainly because the time of check period exceed `topology_wait`.
						ssl_mx.lock();
						SSL_write(ssl[thread_index],"0{\"Error\":\"uncheck fail\"}",25);
						ssl_mx.unlock();
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
								socketfd[thread_index] = -1; //Don't open epoll
								++pending_amount;
								//TODO echo back how long should that client wait.
								continue;
							}
						}
						else{
							//There is a signal is `section-checking`, so pending this signal.
							pending_fd.push_back(socketfd[thread_index]);
							pending_msg.push_back(raw_msg[thread_index]);
							socketfd[thread_index] = -1; //Don't open epoll
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
					ssl_mx.lock();
					SSL_write(ssl[thread_index],"1",1);
					ssl_mx.unlock();
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
					if(msg_freeze_fd==socketfd[thread_index]){
						msg_freeze_fd = -1;
						freeze_str = "!";
						ssl_mx.lock();
						SSL_write(ssl[thread_index],"1",1);
						ssl_mx.unlock();
					}
					else{
						//Something wrong, mainly because the time of check period exceed `topology_wait`.
						ssl_mx.lock();
						SSL_write(ssl[thread_index],"0{\"Error\":\"unfreeze fail\"}",26);
						ssl_mx.unlock();
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
								socketfd[thread_index] = -1; //Don't open epoll
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
							socketfd[thread_index] = -1; //Don't open epoll
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
						ssl_mx.lock();
						SSL_write(ssl[thread_index],"1",1);
						ssl_mx.unlock();
					}
					else{
						ssl_mx.lock();
						SSL_write(ssl[thread_index],"0{\"Error\":\"init too long\"}",26);
						ssl_mx.unlock();
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
						socketfd[thread_index] = -1; //Don't open epoll
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
						socketfd[thread_index] = -1; //Don't open epoll
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

inline void tcp_server_tls::echo_back_msg(const short int thread_index, std::string &msg){
	ssl_mx.lock();
	SSL_write(ssl[thread_index], msg.c_str(), msg.length());
	ssl_mx.unlock();
}

inline void tcp_server_tls::echo_back_msg(const short int thread_index, const char* msg){
	ssl_mx.lock();
	SSL_write(ssl[thread_index], msg, strlen(msg));
	ssl_mx.unlock();
}

inline void tcp_server_tls::echo_back_error(const short int thread_index, std::string msg){
	if(msg[0]=='[' || msg[0]=='{'){
		msg = "0{\"Error\":"+msg+"}";
	}
	else{
		msg = "0{\"Error\":\""+msg+"\"}";
	}
	ssl_mx.lock();
	SSL_write(ssl[thread_index], msg.c_str(), msg.length());
	ssl_mx.unlock();
}
	
inline void tcp_server_tls::echo_back_sql_error(const short int thread_index){
	ssl_mx.lock();
	SSL_write(ssl[thread_index], "0{\"Error\":\"SQL\"}",16);
	ssl_mx.unlock();
}

inline void tcp_server_tls::echo_back_result(const short int thread_index, bool error){
	ssl_mx.lock();
	if(error==true){
		SSL_write(ssl[thread_index], "0",1);
	}
	else{
		SSL_write(ssl[thread_index], "1",1);
	}
	ssl_mx.unlock();
}
