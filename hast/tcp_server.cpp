bool tcp_server::init(hast::tcp_socket::port port, short int unsigned max){
	if(max==0){
		return false;
	}
	if(anti_data_racing==true && max==1){
		return false;
	}
	max_thread = max;
	server_thread::init();
	if(getaddrinfo(NULL, port.c_str(), &hints, &res)!=0){
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
	if(p==NULL){
		return false;
	}
	freeaddrinfo(res); 
    if(listen(host_socket,listen_pending)==0){
		return true;
	}
	else{
		return false;
	}
}


