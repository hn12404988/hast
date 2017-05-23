#include <iostream>
#include <hast/unix_server.hpp>
unix_server server;

auto execute = [&](const short int index){
	std::string str;
	while(server.msg_recv(index)==true){
		std::cout << "Thread: " << index << std::endl;
		std::cout << "MSG: " << server.raw_msg[index] << std::endl;
		server.check_in(index,server.raw_msg[index]);
		std::cout << "check in: " << index << std::endl;
		server.check_out(index);
		std::cout << "check out: " << index << std::endl;
		server.echo_back_msg(index,"Got it!");
	}
	server.done(index);
	return;
};

int main(){
	server.execute = execute;
	server.section_check = true;
	if(server.init(__FILE__,2)==false){
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	return 0;
}
