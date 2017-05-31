#ifndef hast_tcp_server_tls_hpp
#define hast_tcp_server_tls_hpp

#include <hast/tcp_config.hpp>
#include <hast/tls_server.hpp>

class tcp_server_tls : public tcp_config , public tls_server{
public:
	tcp_server_tls():
		tls_server(){
		reset_addr(hast::tcp_socket::SERVER);
	}
	bool init(const char* crt, const char* key, hast::tcp_socket::port port,short int unsigned max = 2);
};

#include <hast/tcp_server_tls.cpp>
#endif /* hast_tcp_server_tls_hpp */
