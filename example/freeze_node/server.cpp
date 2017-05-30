#include <iostream>
#include <hast/tcp_server_tls.hpp>
tcp_server_tls server;

auto execute = [&](const short int index){
	std::string str;
	while(server.msg_recv(index)==true){
		std::cout << "/****** Server Success ********/" << std::endl;
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
	server.msg_freeze = true;
	if(server.init("/home/tls/server_2/server.crt","/home/tls/server_2/server.key","8888")==false){
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	return 0;
}
