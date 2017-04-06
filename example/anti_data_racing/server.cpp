#include <iostream>
#include <hast/unix_server.h>
unix_server server;

auto execute = [&](const short int index){
	std::string str;
	while(server.msg_recv(index)==true){
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "recv in thread: " << index << std::endl;
		std::cout << "msg is: " << server.raw_msg[index] << std::endl;
		server.echo_back_msg(index,"Got it!");
	}
	server.done(index);
	return;
};

int main(){
	server.execute = execute;
	server.anti_data_racing = true;
	if(server.init("anti_server.socket")==false){
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	return 0;
}
