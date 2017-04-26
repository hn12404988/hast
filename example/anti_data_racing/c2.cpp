#include <iostream>
#include <chrono>
#include <thread>
#include <hast/client_thread.hpp>

int main(){
	std::string msg;
	client_thread client;
	client.set_wait_maximum(10);
	short int to_server {0},error_flag, count {4};
	std::vector<std::string> location;
	location.push_back("anti_server.socket");
	client.import_location(&location);
	msg = "anti_data_racing";
	for(;count>0;--count){
		msg = "anti_data_racing";
		error_flag = client.fireNforget(to_server,msg);
		if(error_flag==0){
			std::cout << "/****** c2 fireNforget Success ********/" << std::endl;
		}
		else{
			std::cout << "/****** c2 fireNforget Fail ********/" << std::endl;
		}
	}
	std::cout << "c2 waiting for all fireNforget are processed by server"<< std::endl;
	if(client.join(to_server)==true){
		std::cout << "c2 join fireNforget successfully" << std::endl;
	}
	else{
		std::cout << "c2 fail on join fireNforget" << std::endl;
	}
	return 0;
}
