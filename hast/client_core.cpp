client_core::client_core(){
	epollfd = epoll_create1(0);
	ev.events = EPOLLIN;
}

client_core::~client_core(){
	for(i=0;i<amount;++i){
		close_socket(i);
	}
}

inline void client_core::close_runner(short int index){
	if(socketfd[index]!=-1){
		epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd[index],nullptr);
		shutdown(socketfd[index],SHUT_RDWR);
		close(socketfd[index]);
		socketfd[index] = -1;
	}
	location_list[index] = -1;
}

inline void client_core::close_socket(int socket){
	short int a;
	a = amount-1;
	for(;a>=0;--a){
		if(socketfd[a]==socket){
			break;
		}
	}
	if(a>=0){
		close_runner(a);
	}
}

std::string client_core::error_send(short int flag, short int location_index, std::string msg){
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
			msg = "{\"type\":\"socket\",\"from_node\":\""+node_name+"\",\"to_node\":\"NULL\",\"message\":\""+msg+"\",\"message2\":\""+std::to_string(flag)+"\"}";
		}
		else{
			msg = "{\"type\":\"socket\",\"from_node\":\""+node_name+"\",\"to_node\":\""+(*location)[location_index]+"\",\"message\":\""+msg+"\",\"message2\":\""+std::to_string(flag)+"\"}";
		}
		return msg;
	}
}

std::string client_core::reply_error_send(short int location_index, std::string msg, std::string reply){
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

void client_core::import_location(std::vector<std::string> *location, short int amount){
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
		for(i=0;i<this->amount;++i){
			socketfd.push_back(-1);
			location_list.push_back(-1);
		}
	}
}

inline bool client_core::build_on_i(short int &location_index){
	j = 1;
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

inline void client_core::build_runner(short int &location_index){
	i = amount-1;
	for(;i>=0;--i){
		if(socketfd[i]==-1){
			if(build_on_i(location_index)==true){
				location_list[i] = location_index;
			}
			else{
				i = -1;
			}
			return;
		}
	}
	if(i==-1){
		i = amount - 1;
		close_runner(i);
		build_runner(location_index);
	}
}

short int client_core::fire(short int &location_index,std::string &msg){
	if(msg==""){
		str = error_send(7,location_index,"Empty");
		error_fire(str);
		return 7;
	}
	for(i=0;i<amount;++i){
		if(location_list[i]==location_index){
			up();
			break;
		}
	}
	if(i==amount){
		build_runner(location_index);
	}
	if(i==-1){
		msg = error_send(1,location_index,msg);
		error_fire(msg);
		msg.clear();
		return 1;
	}
	else{
		if( send(socketfd[i] , msg.c_str() , msg.length() , 0) < 0){
			close_runner(i);
			for(i=0;i<amount;++i){
				if(location_list[i]==location_index){
					break;
				}
			}
			if(i==amount){
				build_runner(location_index);
			}
			if(i==-1){
				msg = error_send(1,location_index,msg);
				error_fire(msg);
				msg.clear();
				return 1;
			}
			if( send(socketfd[i] , msg.c_str() , msg.length() , 0) < 0){
				close_runner(i);
				msg = error_send(2,location_index,msg);
				error_fire(msg);
				msg.clear();
				return 2;
			}
		}
	}
	j = receive(msg);
	if(j>0){
		short int tmp_j {j};
		str = error_send(j,location_index,msg);
		close_runner(i);
		error_fire(str);
		msg.clear();
		return tmp_j;
	}
	else{
		if(str==""){
			str = error_send(4,location_index,msg);
			close_runner(i);
			error_fire(str);
			msg.clear();
			return 4;
		}
		else{
			if(str[0]=='0'){
				str = reply_error_send(location_index, msg,str);
				error_fire(str);
				//msg = "0"; Don't show error msg to client. Do this while on production.
			}
			else{
				msg = str;
			}
			return 0;
		}
	}
}

inline short int client_core::receive(std::string &msg){
	str.clear();
	for(;;){
		j = epoll_wait(epollfd, events, MAX_EVENTS, 1000*wait_maximum);
		if(j>0){
			--j;
			for(;j>=0;--j){
				if(events[j].data.fd==socketfd[i]){
					if(events[j].events!=1){
						return 10;
					}
					break;
				}
			}
			if(j==-1){
				continue;
			}
			else{
				for(;;){
					j = recv(socketfd[i], reply, transport_size, MSG_DONTWAIT);
					if(j>0){
						j += str.length();
						str.append(reply);
						str.resize(j);
						j = 0;
					}
					else if(j==-1){
						j = 0;
						break;
					}
					else if(j==0){
						str.clear();
						j = 3;
						break;
					}
				}
			}
			break;
		}
		else if(j==0){
			j = 4;
			break;
		}
		else if(j==-1){
			j = 6;
			break;
		}
	}
	if(j>0){
		return j;
	}
	else{
		return 0;
	}
}

inline void client_core::error_fire(std::string msg){
	if(msg!=""){
		if(error_socket_index!=-1){
			fire(error_socket_index,msg);
		}
	}
}

short int client_core::fireNclose(short int &location_index,std::string &msg){
	j = fire(location_index,msg);
	close_runner(i);
	i = j;
	return i;
}

short int client_core::fireNfreeze(short int &location_index,std::string &msg){
	if(msg==""){
		str = error_send(7,location_index,"Empty");
		error_fire(str);
		msg.clear();
		return 7;
	}
	msg.append("!");
	return fire(location_index,msg);
}

short int client_core::unfreeze(short int &location_index){
	str = "!";
	return fire(location_index,str);
}

short int client_core::fireNcheck(short int &location_index,std::string &msg){
	if(msg==""){
		str = error_send(7,location_index,"Empty");
		error_fire(str);
		msg.clear();
		return 7;
	}
	msg = "<"+msg+">";
	return fire(location_index,msg);
}

short int client_core::uncheck(short int &location_index){
	str = "<>";
	return fire(location_index,str);
}

inline void client_core::up(){
	if(i==0){
		return;
	}
	j = socketfd[i];
	socketfd[i] = socketfd[i-1];
	socketfd[i-1] = j;
	j = location_list[i];
	location_list[i] = location_list[i-1];
	location_list[i-1] = j;
	--i;
}

void client_core::set_wait_maximum(short int wait){
	wait_maximum = wait;
}

void client_core::set_error_node(short int socket_index,const char* file_name){
	node_name = file_name;
	error_socket_index = socket_index;
}
