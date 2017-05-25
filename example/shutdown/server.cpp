#include <hast/unix_server.hpp>
#include <iostream>

unix_server server;

auto execute = [&](const short int index){
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
	server.call_shutdown = true;
	server.set_shutdown_code("hast shutdown");
	if(server.init(__FILE__)==false){
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	std::cout << "Server Shut Down" << std::endl;
	return 0;
}
