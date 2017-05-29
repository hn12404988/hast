#include <iostream>
#include <chrono>
#include <thread>
#include <hast/client_core.hpp>

int main(){
	std::string msg;
	client_core client;
	char error_flag;
	std::vector<std::string> location;
	location.push_back("server.socket");
	short int to_s1 {0};
	client.import_location(&location);
	msg = "hast shutdown";
	error_flag = client.shutdown_server(to_s1,msg);
	if(error_flag==hast_client::SUCCESS){
		std::cout << "/****** Client Successfully Shutting Down Server********/" << std::endl;
	}
	else{
		std::cout << "/****** Client Fail on Shutting Down Server********/" << std::endl;
	}
	return 0;
}
