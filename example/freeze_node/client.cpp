#include <iostream>
#include <chrono>
#include <thread>
#include <hast/client_thread_tls.hpp>
#include <hast/client_core_tls.hpp>

int main(){
	std::string msg,msg2;
	client_core_tls c1;
	client_thread_tls c2;
	short int to_s1 {0};
	char error_flag;
	c2.set_wait_maximum(10);
	std::vector<std::string> location;
	location.push_back("TLS:172.18.0.3:8888");
	c1.import_location(&location);
	c2.import_location(&location,2);
	msg = "freeze"; // change to (msg = '!') will freeze all thread in server
	error_flag = c1.fireNfreeze(to_s1,msg);
	if(error_flag==hast_client::SUCCESS){
		std::cout << "/****** c1 freeze server by msg successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c1 fail on freezing server ********/" << std::endl;
	}
	msg = "freeze";
	error_flag = c2.fireNstore(to_s1,msg);
	if(error_flag==hast_client::SUCCESS){
		std::cout << "/****** c2 fireNstore 'freeze' successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c2 fail on fireNstore 'freeze' ********/" << std::endl;
	}
	msg2 = "normal msg";
	error_flag = c2.fireNstore(to_s1,msg2);
	if(error_flag==hast_client::SUCCESS){
		std::cout << "/****** c2 fireNstore 'normal msg' successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c2 fail on fireNstore 'normal msg' ********/" << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	if(c1.unfreeze(to_s1)==0){
		std::cout << "/****** c1 unfreeze server successfully ********/" << std::endl;
	}
	else{
		std::cout << "/****** c1 fail on unfreezing server ********/" << std::endl;
	}
	c2.join(to_s1);
	std::cout << "/****** c2 join finish ********/" << std::endl;
	std::cout << "c2 reply of msg: " << msg << std::endl;
	std::cout << "c2 reply of msg2: " << msg2 << std::endl;
	return 0;
}
