#include <iostream>
#include <chrono>
#include <thread>
#include <hast/client_thread.hpp>

int main(){
	std::string msg;
	client_core c1;
	client_thread c2;
	short int to_s1 {0},error_flag;
	c2.set_wait_maximum(4);
	std::vector<std::string> location;
	location.push_back("server.socket");
	c1.import_location(&location);
	c2.import_location(&location);
	msg = "check";
	error_flag = c1.fireNcheck(to_s1,msg);
	if(error_flag==0){
		std::cout << "/****** c1 section_check server by msg successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c1 fail on section_check server ********/" << std::endl;
	}
	msg = "check";
	error_flag = c2.fireNforget(to_s1,msg);
	if(error_flag==0){
		std::cout << "/****** c2 fireNforget 'check' successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c2 fail on fireNforget 'check' ********/" << std::endl;
	}
	msg = "normal msg";
	error_flag = c2.fireNforget(to_s1,msg);
	if(error_flag==0){
		std::cout << "/****** c2 fireNforget 'normal msg' successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c2 fail on fireNforget 'normal msg' ********/" << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	if(c1.uncheck(to_s1)==0){
		std::cout << "/****** c1 unfreeze server successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c1 fail on unfreezing server ********/" << std::endl;
	}
	if(c2.join(to_s1)==true){
		std::cout << "/****** c2 join successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c2 fail on joining ********/" << std::endl;
	}
	return 0;
}
