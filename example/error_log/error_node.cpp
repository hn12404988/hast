#include <hast/tcp_server.hpp>
#include <iostream>
tcp_server server;

auto execute = [&](const short int index){
	std::string str;
	while(server.msg_recv(index)==true){
		std::cout << "/****** Error Node ********/" << std::endl;
		std::cout << "recv in thread: " << index << std::endl;
		std::cout << "msg is: " << server.raw_msg[index] << std::endl;
		std::cout << "/******************************/" << std::endl;
		server.echo_back_msg(index,"Got it!");
	}
	server.done(index);
	return;
};

int main(){
	server.execute = execute;
	if(server.init("8888")==false){
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	return 0;
}
