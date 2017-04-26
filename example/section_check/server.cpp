#include <iostream>
#include <hast/unix_server.hpp>
unix_server server;

auto execute = [&](const short int index){
	std::string str;
	while(server.msg_recv(index)==true){
		std::cout << "/****** Server Success ********/" << std::endl;
		std::cout << "/****** Check-In ********/" << std::endl;
		server.check_in(index,server.raw_msg[index]);
		std::cout << "recv in thread: " << index << std::endl;
		std::cout << "msg is: " << server.raw_msg[index] << std::endl;
		server.check_out(index);
		std::cout << "/****** Check-Out ********/" << std::endl;
		std::cout << "/******************************/" << std::endl;
		server.echo_back_msg(index,"Got it!");
	}
	server.done(index);
	return;
};

int main(){
	server.execute = execute;
	server.check_data_racing = true;
	if(server.init(__FILE__)==false){
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	return 0;
}
