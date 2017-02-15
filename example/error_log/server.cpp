#include <hast/unix_server.h>
#include <iostream>
unix_server server;

auto execute = [&](const short int index){
	std::string str;
	while(server.msg_recv(index)==true){
		if(server.raw_msg[index]=="1"){
			std::cout << "/****** Server Success ********/" << std::endl;
			std::cout << "recv in thread: " << index << std::endl;
			std::cout << "msg is: " << server.raw_msg[index] << std::endl;
			std::cout << "/************ Error Reply ****************/" << std::endl;
			server.echo_back_msg(server.socketfd[index],"0");
		}
		else{
			std::cout << "/****** Server Success ********/" << std::endl;
			std::cout << "recv in thread: " << index << std::endl;
			std::cout << "msg is: " << server.raw_msg[index] << std::endl;
			std::cout << "/************ No Reply **************/" << std::endl;
		}
	}
	server.done(index);
	return;
};

int main(){
	server.execute = execute;
	if(server.init(__FILE__)==false){
		std::cout << "Server can't init at file: " << __FILE__ << std::endl;
	}
	else{
		server.start_accept();
	}
	return 0;
}
