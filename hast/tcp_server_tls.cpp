bool tcp_server_tls::init(const char* crt, const char* key, hast::tcp_socket::port port, short int unsigned max){
	/**
	 * Copy from tcp_server
	 **/
	if(max==0){
		return false;
	}
	max_thread = max;
	server_thread::init();
	if(getaddrinfo(NULL, port.c_str(), &hints, &res)!=0){
		//fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return false;
	}
	struct addrinfo *p{NULL};
	int flag {1};
	for(p = res; p != NULL; p = p->ai_next) {
		if ((host_socket = socket(p->ai_family, p->ai_socktype,
						   p->ai_protocol)) == -1) {
			continue;
		}
		if (setsockopt(host_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) {
			close(host_socket);
			continue;
		}
		if (bind(host_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(host_socket);
			continue;
		}
		break;
	}
	freeaddrinfo(res);
	res = nullptr;
	if(p==NULL){
		return false;
	}
    if(listen(host_socket,listen_pending)==0){
		//true
	}
	else{
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

