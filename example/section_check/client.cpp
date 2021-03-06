#include <iostream>
#include <chrono>
#include <thread>
#include <hast/client_thread.hpp>

int main(){
	std::string msg;
	client_core c1;
	client_thread c2;
	short int to_s1 {0};
	char error_flag;
	c2.set_wait_maximum(4);
	std::vector<std::string> location;
	location.push_back("server.socket");
	c1.import_location(&location);
	c2.import_location(&location);
	msg = "check";
	error_flag = c1.fireNcheck(to_s1,msg);
	if(error_flag==hast_client::SUCCESS){
		std::cout << "/****** c1 section_check server by msg successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c1 fail on section_check server ********/" << std::endl;
	}
	msg = "check";
	error_flag = c2.fireNforget(to_s1,msg);
	if(error_flag==hast_client::SUCCESS){
		std::cout << "/****** c2 fireNforget 'check' successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c2 fail on fireNforget 'check' ********/" << std::endl;
	}
	msg = "normal msg";
	error_flag = c2.fireNforget(to_s1,msg);
	if(error_flag==hast_client::SUCCESS){
		std::cout << "/****** c2 fireNforget 'normal msg' successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c2 fail on fireNforget 'normal msg' ********/" << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	if(c1.uncheck(to_s1)==hast_client::SUCCESS){
		std::cout << "/****** c1 uncheck server successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c1 fail on unchecking server ********/" << std::endl;
	}
	c2.join(to_s1);
	std::cout << "/****** c2 finish join ********/" << std::endl;
	return 0;
}
