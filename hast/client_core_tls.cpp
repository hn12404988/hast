client_core_tls::client_core_tls(){
	
}

client_core_tls::~client_core_tls(){
	short int a;
	if(ssl!=nullptr){
		for(a=0;a<amount;++a){
			if(ssl[a]!=nullptr){
				SSL_free(ssl[a]);
				ssl[a] = nullptr;
			}
		}
		delete [] ssl;
		ssl = nullptr;
	}
	if(TLS!=nullptr){
		delete [] TLS;
		TLS = nullptr;
	}
	if(ctx!=nullptr){
		SSL_CTX_free(ctx);
		ctx = nullptr;
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

bool client_core_tls::TLS_init(){
	const SSL_METHOD *method;
	/* ---------------------------------------------------------- *
	 * These function calls initialize openssl for correct work.  *
	 * ---------------------------------------------------------- */
	OpenSSL_add_ssl_algorithms();
	SSL_load_error_strings();
	/* ---------------------------------------------------------- *
	 * initialize SSL library and register algorithms             *
	 * ---------------------------------------------------------- */
	if(SSL_library_init() < 0){
		//std::cout << "Could not initialize the OpenSSL library !" << std::endl;
		return false;
	}
	/* ---------------------------------------------------------- *
	 * Set SSLv2 client hello, also announce SSLv3 and TLSv1      *
	 * ---------------------------------------------------------- */
	method = SSLv23_client_method();
	/* ---------------------------------------------------------- *
	 * Try to create a new SSL context                            *
	 * ---------------------------------------------------------- */
	if ( (ctx = SSL_CTX_new(method)) == nullptr){
		//std::cout << "Unable to create a new SSL context structure." << std::endl;
		return false;
	}
	/* ---------------------------------------------------------- *
	 * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
	 * ---------------------------------------------------------- */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
	return true;
}

bool client_core_tls::import_location(std::vector<std::string> *location, short int unsigned amount){
	if(TLS_init()==false){
		return false;
	}
	short int a;
	a = location->size();
	if(a==0){
		return false;
	}
	TLS = new bool [a];
	--a;
	for(;a>=0;--a){
		if((*location)[a].substr(0,4)=="TLS:"){
			(*location)[a] = (*location)[a].substr(4);
			TLS[a] = true;
		}
		else{
			TLS[a] = false;
		}
	}
	client_core::import_location(location,amount);
	a = client_core::amount;
	if(ssl==nullptr){
		ssl = new SSL* [a];
		--a;
		for(;a>=0;--a){
			ssl[a] = nullptr;
		}
	}
	return true;
}

inline short int client_core_tls::build_runner(short int location_index){
	short int runner_index;
	int err;
	runner_index = client_core::build_runner(location_index);
	if(runner_index==-1){
		return -1;
	}
	if(TLS[location_index]==true){
		ssl[runner_index] = SSL_new(ctx);
		SSL_set_fd(ssl[runner_index], socketfd[runner_index]);
		for(;;){
			err = SSL_connect(ssl[runner_index]);
			if(err==1){
				//std::cout << "SSL_connect success" << std::endl;
				break;
			}
			else if(err==0){
				//std::cout << "SSL_connect fail 0" << std::endl;
				close_runner(runner_index);
				return -1;
			}
			else{
				err = SSL_get_error(ssl[runner_index],err);
				if (err==SSL_ERROR_WANT_READ || err==SSL_ERROR_WANT_WRITE){
					//std::cout << "SSL_connect attempt" << std::endl;
					continue;
				}
				else{
					//std::cout << "SSL_connect fail err" << std::endl;
					close_runner(runner_index);
					return -1;
				}
			}
		}
	}
	return runner_index;
}

inline void client_core_tls::close_runner(short int runner_index){
	if(ssl[runner_index]!=nullptr){
		SSL_free(ssl[runner_index]);
		ssl[runner_index] = nullptr;
	}
	client_core::close_runner(runner_index);
}

char client_core_tls::write(short int &runner_index, short int location_index, std::string &msg){
	if(TLS[location_index]==false){
		return client_core::write(runner_index,location_index,msg);
	}
	else{
		if( SSL_write(ssl[runner_index], msg.c_str(), msg.length()) <= 0){
			close_runner(runner_index);
			runner_index = get_runner(location_index);
			if(runner_index==-1){
				runner_index = build_runner(location_index);
			}
			if(runner_index==-1){
				msg = error_msg(hast_client::EXIST,location_index,msg);
				error_fire(msg);
				msg.clear();
				return hast_client::EXIST;
			}
			if( SSL_write(ssl[runner_index], msg.c_str(), msg.length()) < 0){
				close_runner(runner_index);
				runner_index = -1;
				msg = error_msg(hast_client::SEND,location_index,msg);
				error_fire(msg);
				msg.clear();
				return hast_client::SEND;
			}
		}
		return hast_client::SUCCESS;
	}
}

char client_core_tls::read(short int runner_index, std::string &reply_str){
	if(TLS[location_list[runner_index]]==false){
		return client_core::read(runner_index,reply_str);
	}
	else{
		reply_str.clear();
		int len;
		char reply[transport_size];
		for(;;){
			len = SSL_read(ssl[runner_index], reply, transport_size);
			if(len>0){
				reply_str.append(reply,len);
			}
			else if(len==-1){
				return hast_client::SUCCESS;
			}
			else if(len==0){
				reply_str.clear();
				return hast_client::SSL_r;
			}
		}
	}
}
