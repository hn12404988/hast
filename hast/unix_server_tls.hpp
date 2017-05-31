#ifndef hast_unix_server_tls_hpp
#define hast_unix_server_tls_hpp

#include <hast/unix_config.hpp>
#include <hast/tls_server.hpp>
//#include <cstring> //errno

class unix_server_tls : public tls_server{
private:
	struct sockaddr_un addr;
public:
	bool init(const char* crt, const char* key, const char *file,short int unsigned max = 2);
};

#include <hast/unix_server_tls.cpp>
#endif /* hast_unix_server_tls_hpp */
