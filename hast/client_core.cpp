client_core::client_core(){
	epollfd = epoll_create1(0);
	ev.events = EPOLLIN;
}

client_core::~client_core(){
	if(socketfd!=nullptr){
		delete [] socketfd;
		socketfd = nullptr;
	}
	if(location_list!=nullptr){
		delete [] location_list;
		location_list = nullptr;
	}
}

inline void client_core::close_runner(short int runner_index){
	if(socketfd[runner_index]!=-1){
		epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd[runner_index],nullptr);
		shutdown(socketfd[runner_index],SHUT_RDWR);
		close(socketfd[runner_index]);
		socketfd[runner_index] = -1;
	}
	location_list[runner_index] = -1;
}

std::string client_core::error_msg(const char flag, short int location_index, std::string msg){
	if(error_socket_index==-1){
		std::cout << "Client didn't set error node, so here are the error messages." << std::endl;
		std::cout << "Flag: " << flag << std::endl;
		std::cout << "To: " << (*location)[location_index] << std::endl;
		std::cout << "Message: " << msg << std::endl;
		return "";
	}
	else if(error_socket_index==location_index){
		std::cout << "Error node is dead, so here are the error messages." << std::endl;
		std::cout << "Node: " << node_name << std::endl;
		std::cout << "Flag: " << flag << std::endl;
		std::cout << "To: " << (*location)[location_index] << std::endl;
		std::cout << "Message: " << msg << std::endl;
		return "";
	}
	else{
		if(location_index<0){
			msg = "{\"type\":\"socket\",\"from_node\":\""+node_name+"\",\"to_node\":\"NULL\",\"message\":\""+msg+"\",\"message2\":\""+flag+"\"}";
		}
		else{
			msg = "{\"type\":\"socket\",\"from_node\":\""+node_name+"\",\"to_node\":\""+(*location)[location_index]+"\",\"message\":\""+msg+"\",\"message2\":\""+flag+"\"}";
		}
		return msg;
	}
}

std::string client_core::reply_error_msg(short int location_index, std::string msg, std::string reply){
	if(error_socket_index==-1){
		std::cout << "Client didn't set error node, so here are the reply error messages." << std::endl;
		std::cout << "To: " << (*location)[location_index] << std::endl;
		std::cout << "Message: " << msg << std::endl;
		std::cout << "Reply: " << reply << std::endl;
		return "";
	}
	else if(error_socket_index==location_index){
		std::cout << "Error node is dead, so here are the reply error messages." << std::endl;
		std::cout << "Node: " << node_name << std::endl;
		std::cout << "To: " << (*location)[location_index] << std::endl;
		std::cout << "Message: " << msg << std::endl;
		std::cout << "Reply: " << reply << std::endl;
		return "";
	}
	else{
		if(location_index<0){
			msg = "{\"type\":\"request\",\"from_node\":\""+node_name+"\",\"to_node\":\"NULL\",\"message\":\""+msg+"\",\"message2\":\""+reply+"\"}";
		}
		else{
			msg = "{\"type\":\"request\",\"from_node\":\""+node_name+"\",\"to_node\":\""+(*location)[location_index]+"\",\"message\":\""+msg+"\",\"message2\":\""+reply+"\"}";
		}
		return msg;
	}
}

void client_core::import_location(std::vector<std::string> *location, short int unsigned amount){
	short int a;
	if(location!=nullptr){
		reset_addr(hast::tcp_socket::CLIENT);
		addr.sun_family = AF_UNIX;
		this->location = location;
		if(amount==0){
			this->amount = location->size();
		}
		else{
			this->amount = amount;
		}
		if(socketfd==nullptr && location_list==nullptr){
			socketfd = new int [amount];
			location_list = new short int [amount];
		}
		for(a=0;a<this->amount;++a){
			socketfd[a] = -1;
			location_list[a] = -1;
		}
	}
}

inline bool client_core::build_on_i(short int i, short int location_index){
	int j {1};
	if((*location)[location_index].find(":")!=std::string::npos){
		//tcp
		std::string ip,port;
		short int pos;
		ip = (*location)[location_index];
		pos = ip.find(":");
		if(pos==std::string::npos){
			return false;
		}
		port = ip.substr(pos+1);
		ip.resize(pos);
		reset_addr(hast::tcp_socket::CLIENT);
		if(getaddrinfo(ip.c_str(),port.c_str(),&hints,&res)!=0){
			return false;
		}
		for(; res != nullptr; res = res->ai_next) {
			if ((socketfd[i] = socket(res->ai_family, res->ai_socktype,
										 res->ai_protocol)) == -1) {
				continue;
			}
			if (setsockopt(socketfd[i], SOL_SOCKET, SO_REUSEADDR, &j, sizeof(j)) == -1) {
				close_runner(i);
				continue;
			}
			if (connect(socketfd[i], res->ai_addr, res->ai_addrlen)<0) {
				close_runner(i);
				continue;
			}
			break;
		}
		if(res==nullptr){
			return false;
		}
	}
	else{
		//unix
		if (( socketfd[i] = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			return false;
		}
		if (setsockopt(socketfd[i], SOL_SOCKET, SO_REUSEADDR, &j, sizeof(j)) == -1) {
			close_runner(i);
			return false;
		}
		strcpy(addr.sun_path, (*location)[location_index].c_str());
		j = strlen(addr.sun_path) + sizeof(addr.sun_family);
		if (connect(socketfd[i], (struct sockaddr *)&addr, j) == -1) {
			close_runner(i);
			return false;
		}
	}
	ev.data.fd = socketfd[i];
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd[i],&ev)==-1){
		close_runner(i);
		return false;
	}
	return true;
}

char client_core::shutdown_server(short int &location_index,std::string &shutdown_code){
	short int runner_bk;
	char a;
	a = fire_return(location_index,shutdown_code,runner_bk);
	if(a==hast_client::SUCCESS){
		close_runner(runner_bk);
		shutdown_code = "shutdown";
		return fire(location_index,shutdown_code);
	}
	else{
		return a;
	}
}

char client_core::fire_return(short int &location_index,std::string &msg, short int &runner_bk){
	short int runner_index;
	char a;
	std::string reply;
	if(msg==""){
		error_fire(error_msg(hast_client::FORMAT,location_index,"Empty"));
		return hast_client::FORMAT;
	}
	runner_index = get_runner(location_index);
	if(runner_index==-1){
		runner_index = build_runner(location_index);
	}
	if(runner_index==-1){
		msg = error_msg(hast_client::EXIST,location_index,msg);
		error_fire(msg);
		msg.clear();
		runner_bk = -1;
		return hast_client::EXIST;
	}
	a = write(runner_index,location_index,msg);
	runner_bk = runner_index;
	if(a!=hast_client::SUCCESS){
		runner_bk = -1;
		return a;
	}
	a = receive(runner_index, reply);
	if(a!=hast_client::SUCCESS){
		reply = error_msg(a,location_index,msg);
		close_runner(runner_index);
		runner_bk = -1;
		error_fire(reply);
		msg.clear();
		return a;
	}
	else{
		if(reply==""){
			reply = error_msg(hast_client::REPLY,location_index,msg);
			close_runner(runner_index);
			runner_bk = -1;
			error_fire(reply);
			msg.clear();
			return hast_client::REPLY;
		}
		else{
			if(reply[0]=='0'){
				msg = reply_error_msg(location_index, msg,reply);
				error_fire(msg);
				msg = reply; //Do this in dev mode.
				//msg = "0"; Don't show error msg to client. Do this while on production.
			}
			else{
				msg = reply;
			}
			return hast_client::SUCCESS;
		}
	}
}

char client_core::fire(short int &location_index,std::string &msg){
	short int runner_index;
	char a;
	std::string reply;
	if(msg==""){
		error_fire(error_msg(hast_client::FORMAT,location_index,"Empty"));
		return hast_client::FORMAT;
	}
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
	a = write(runner_index,location_index,msg);
	if(a!=hast_client::SUCCESS){
		return a;
	}
	a = receive(runner_index, reply);
	if(a!=hast_client::SUCCESS){
		reply = error_msg(a,location_index,msg);
		close_runner(runner_index);
		error_fire(reply);
		msg.clear();
		return a;
	}
	else{
		if(reply==""){
			reply = error_msg(hast_client::REPLY,location_index,msg);
			close_runner(runner_index);
			error_fire(reply);
			msg.clear();
			return hast_client::REPLY;
		}
		else{
			if(reply[0]=='0'){
				msg = reply_error_msg(location_index, msg,reply);
				error_fire(msg);
				msg = reply; //Do this in dev mode.
				//msg = "0"; Don't show error msg to client. Do this while on production.
			}
			else{
				msg = reply;
			}
			return hast_client::SUCCESS;
		}
	}
}

inline char client_core::receive(short int runner_index,std::string &reply){
	int len;
	for(;;){
		len = epoll_wait(epollfd, events, MAX_EVENTS, wait_maximum);
		if(len>0){
			--len;
			for(;len>=0;--len){
				if(events[len].data.fd==socketfd[runner_index]){
					if(events[len].events!=1){
						return hast_client::EPOLL_EV;
					}
					break;
				}
			}
			if(len==-1){
				continue;
			}
			else{
				return read(runner_index,reply);
			}
		}
		else if(len==0){
			return hast_client::REPLY;
		}
		else if(len==-1){
			return hast_client::EPOLL;
		}
	}
}

inline void client_core::error_fire(std::string msg){
	if(msg!=""){
		if(error_socket_index!=-1){
			fire(error_socket_index,msg);
		}
	}
}

char client_core::fireNclose(short int &location_index,std::string &msg){
	short int runner_bk;
	char a;
	a = fire_return(location_index,msg,runner_bk);
	if(a==hast_client::SUCCESS){
		close_runner(runner_bk);
		return hast_client::SUCCESS;
	}
	else{
		return a;
	}
}

char client_core::fireNfreeze(short int &location_index,std::string &msg){
	msg.append("!");
	return fire(location_index,msg);
}

char client_core::unfreeze(short int &location_index){
	std::string tmp_msg {"!"};
	return fire(location_index,tmp_msg);
}

char client_core::fireNcheck(short int &location_index,std::string &msg){
	msg = "<"+msg+">";
	return fire(location_index,msg);
}

char client_core::uncheck(short int &location_index){
	std::string tmp_msg {"<>"};
	return fire(location_index,tmp_msg);
}

inline short int client_core::up(short int runner_index){
	if(runner_index==0){
		return 0;
	}
	int a;
	a = socketfd[runner_index];
	socketfd[runner_index] = socketfd[runner_index-1];
	socketfd[runner_index-1] = a;
	a = location_list[runner_index];
	location_list[runner_index] = location_list[runner_index-1];
	location_list[runner_index-1] = a;
	--runner_index;
	return runner_index;
}

void client_core::set_wait_maximum(short int wait){
	wait_maximum = wait*1000;
}

void client_core::set_error_node(short int socket_index,const char* file_name){
	node_name = file_name;
	error_socket_index = socket_index;
}

std::vector<std::string> client_core::get_error_flag(){
	std::vector<std::string> list {"Success",
			"Server doesn't exist, or socket has problem",
			"Fail on sending message",
			"Server's execution crash",
			"No reply",
			"Fail on epoll",
			"Invalid message format",
			"thread joinable is false (client_thread)",
			"epoll events is not 1"};
	return list;
}
