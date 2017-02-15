#ifndef unix_server_h
#define unix_server_h
#include <hast/unix_config.h>
#include <hast/socket_server.h>
//#include <cstring> //errno

class unix_server : public socket_server{
private:
	struct sockaddr_un addr;
public:
unix_server():
	socket_server(){}
	bool init(const char *file,short int unsigned max = 0);
};

#include <hast/unix_server.cpp>
#endif
