#ifndef hast_unix_server_hpp
#define hast_unix_server_hpp
#include <hast/unix_config.hpp>
#include <hast/socket_server.hpp>
//#include <cstring> //errno

class unix_server : public hast::socket_server{
private:
	struct sockaddr_un addr;
public:
	bool init(const char *file,short int unsigned max = 2);
};

#include <hast/unix_server.cpp>
#endif /* hast_unix_server_hpp */
