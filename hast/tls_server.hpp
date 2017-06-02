#ifndef hast_tls_server_hpp
#define hast_tls_server_hpp

#include <hast/socket_server.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

class tls_server : public hast::socket_server{
protected:
	SSL_CTX *ctx {nullptr};
	std::map<int,SSL*> ssl_map;
	SSL **ssl {nullptr};
	std::mutex ssl_mx;
	void close_socket(const int socket_index, int line = 0) override;
	void pending_first() override;
	inline bool read(short int thread_index) override;
	inline bool write(short int thread_index, std::string &msg) override;
	inline bool write(short int thread_index, const char* msg) override;
	inline void recv_epoll() override;
	inline bool write(SSL* ssl, const char* cmsg);
	void reset_accept(int socket_index,SSL *ssl);
public:
	~tls_server();
	void start_accept() override;
	void close_socket(const short int thread_index) override;
};

#include <hast/tls_server.cpp>
#endif /* hast_tls_server_hpp */
