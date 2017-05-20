bool unix_server::init(const char *file, short int unsigned max){
	if(max==0){
		return false;
	}
	if(anti_data_racing==true && max==1){
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
	return true;
}
