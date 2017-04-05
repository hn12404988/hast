#ifndef hast_tcp_server_h
#define hast_tcp_server_h
#include <hast/tcp_config.h>
#include <hast/socket_server.h>

class tcp_server : public tcp_config , public socket_server{
public:
tcp_server():
	socket_server(){
		reset_addr(hast::tcp_socket::SERVER);
	}
	bool init(hast::tcp_socket::port port, short int unsigned max = 0);
};

#include <hast/tcp_server.cpp>
#endif
