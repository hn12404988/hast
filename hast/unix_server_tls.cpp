bool unix_server_tls::init(const char* crt, const char* key, const char *file, short int unsigned max){
	/**
	 * Copy from unix_server
	 **/
	if(max==0){
		return false;
	}
	max_thread = max;
	server_thread::init();
	int flag {1},i;
	std::string socket_name;
	socket_name.append(file);
	if(socket_name.find(".cpp")!=std::string::npos){
		i = socket_name.length();
		socket_name.resize(i-4);
		socket_name.append(".socket");
	}
	if ((host_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		//std::cout << strerror(errno) << std::endl;
		return false;
    }
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_name.c_str());
    unlink(addr.sun_path);
    i = strlen(addr.sun_path) + sizeof(addr.sun_family);
    if (bind(host_socket, (struct sockaddr *)&addr, i) == -1) {
		//std::cout << strerror(errno) << std::endl;
		return false;
    }
	if (setsockopt(host_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) {
		//std::cout << strerror(errno) << std::endl;
		close(host_socket);
		return false;
	}
    if (listen(host_socket, listen_pending) == -1) {
		//std::cout << strerror(errno) << std::endl;
		return false;
    }
	/**
	 * 
	 **/
	short int a;
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
