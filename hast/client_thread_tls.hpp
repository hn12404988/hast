#ifndef hast_client_thread_tls_hpp
#define hast_client_thread_tls_hpp

#include <hast/client_thread.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

/**
 * Address start with: `TLS:` is communicating under TLS.
 **/

class client_thread_tls : public client_thread{
protected:
	const SSL_METHOD *method; //nullptr?
	SSL_CTX *ctx {nullptr};
	bool *TLS {nullptr}; // Record which server is transported under TLS in location.
	SSL **ssl {nullptr}; // runner list with TLS
	bool TLS_init();
	inline short int build_runner(short int location_index) override;
	inline void close_runner(short int runner_index) override;
	char write(short int &runner_index, short int location_index, std::string &msg) override;
	char read(short int runner_index, std::string &reply_str) override;
public:
	client_thread_tls();
	~client_thread_tls();
	bool import_location(std::vector<std::string> *location, short int unsigned amount = 0);
};
#include <hast/client_thread_tls.cpp>
#endif /* hast_client_thread_tls_hpp */

