#include <iostream>
#include <chrono>
#include <thread>
#include <hast/client_core.hpp>

int main(){
	std::string msg;
	short int loop {0};
	client_core client;
	char error_flag;
	client.set_wait_maximum(1); //Maximum time of waiting reply. Set to 1 seconds. (default 2 seconds)
	std::vector<std::string> location; // The list of server's address.
	/**
	 * The list of server's address.
	 * tcp/ip: ip:port. (Ex: 127.0.0.1:8888)
	 * unix: path of socket file. (Ex: /tmp/s1.socket)
	 **/
	location.push_back("172.18.0.3:8888");
	short int to_s1 {0}; //Address of server is stored in position 0 of vector.
	client.import_location(&location); //Import the list to client class.
	while(loop<3){
		msg = "hi, this is client 1";
		error_flag = client.fire(to_s1,msg);
		if(error_flag==hast_client::SUCCESS){
			std::cout << "/****** Client Success ********/" << std::endl;
			std::cout << msg << std::endl;
		}
		else{
			std::cout << "/****** Client Fail ********/" << std::endl;
		}
		std::cout << "/******************************/" << std::endl;
		++loop;
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	return 0;
}
