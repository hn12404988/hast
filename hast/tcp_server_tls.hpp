#ifndef hast_tcp_server_tls_hpp
#define hast_tcp_server_tls_hpp

#include <hast/tcp_server.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

class tcp_server_tls : public tcp_server{
protected:
	SSL_CTX *ctx {nullptr};
	std::map<int,SSL*> ssl_map;
	SSL **ssl {nullptr};
	std::mutex ssl_mx;
	void close_socket(const int socket_index) override;
	void reset_accept(int socket_index,SSL *ssl);
public:
	~tcp_server_tls();
	bool init(const char* crt, const char* key, hast::tcp_socket::port port,short int unsigned max = 2);
	void start_accept();
	bool msg_recv(const short int thread_index);
	inline void echo_back_msg(const short int thread_index, std::string &msg);
	inline void echo_back_msg(const short int thread_index, const char* msg);
	inline void echo_back_error(const short int thread_index, std::string msg);
	inline void echo_back_sql_error(const short int thread_index);
	inline void echo_back_result(const short int thread_index, bool error);
};

#include <hast/tcp_server_tls.cpp>
#endif /* hast_tcp_server_tls_hpp */
