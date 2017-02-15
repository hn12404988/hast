#include <string.h>
#include <string>
#include <iostream>

using namespace std;

void kill_pid(string name){
	FILE *in;
	char buff[300];
	int i;
	string send,send2;
	bool got_it {false};
	send = "ps axww | grep "+name;
	in = popen(send.c_str(),"r");
	send.clear();
	while(fgets(buff, 300, in)!=nullptr){
		send = buff;
		i = 0;
		if(send.find(" grep ")==std::string::npos && send.find("stop.o")==std::string::npos){
			send2.clear();
			while(send[0]==' '){
				send = send.substr(1);
			}
			while(send[i]!=' '){
				send2 += send[i];
				++i;
			}
			cout << "Kill " << name << " while pid id: " << send2 << endl;
			send2 = "kill -9 "+send2;
			system(send2.c_str());
			got_it = true;
		}
	}
	if(got_it==false){
		cout << "Node " << name << " is not running." << endl;
	}
	pclose(in);
}

int main(int argc, char* argv[]){
	int i;
	for(i=1;i<argc;++i){
		kill_pid(argv[i]);
	}
	return 0;
}
