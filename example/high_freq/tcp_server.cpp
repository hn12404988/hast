#include <hast/tcp_server.hpp>
#include <iostream>

tcp_server server; // In global scope, so every thread can use.

auto execute = [&](const short int index){
	std::string str; //Some variable you want to use during process.
	while(server.msg_recv(index)==true){
		std::cout << "/****** Server Success ********/" << std::endl;
		std::cout << "recv in thread: " << index << std::endl;
		std::cout << "msg is: " << server.raw_msg[index] << std::endl;
		std::cout << "/******************************/" << std::endl;
		server.echo_back_msg(index,"Got it!");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	server.done(index); // Close this thread.
	return;
};

int main(){
	server.execute = execute; //Bind the lambda function to the server.
	if(server.init("8888")==false){ //Name of this file as the name of socket file. (.cpp will be change to .socket)
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	return 0;
}
