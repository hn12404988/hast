#include <iostream>
#include <chrono>
#include <thread>
#include <hast/client_core.hpp>

int main(){
	std::string msg;
	client_core client;
	short int to_s1 {1},error_flag;
	std::vector<std::string> location;
	location.push_back("127.0.0.1:8888");
	location.push_back("server.socket");
	client.import_location(&location);
	client.set_error_node(0,__FILE__);

	msg = "1";
	error_flag = client.fire(to_s1,msg);
	if(error_flag==0){
		std::cout << "/****** Client Success ********/" << std::endl;
		std::cout << msg << std::endl;
	}
	else{
		std::cout << "/****** Client Fail ********/" << std::endl;
	}
	
	msg = "0";
	error_flag = client.fire(to_s1,msg);
	if(error_flag==0){
		std::cout << "/****** Client Success ********/" << std::endl;
		std::cout << msg << std::endl;
	}
	else{
		std::cout << "/****** Client Fail ********/" << std::endl;
	}
	std::cout << "/******************************/" << std::endl;
	//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	return 0;
}
