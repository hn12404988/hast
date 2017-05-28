#ifndef hast_client_core_tls_hpp
#define hast_client_core_tls_hpp

#include <hast/client_core.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

/******************** Error Flag *****************************
 * 1: Server doesn't exist, or socket has problem.
 * 2: Fail on sending message.
 * 3: Server's execution crash.
 * 4: No reply.
 * 5: waiting list of cient_thread jam.
 * 6: Fail on epoll.
 * 7: Invalid message format.
 * 8: runner is not enough (client_thread).
 * 9: thread joinable is false (client_thread).
 * 10: epoll events is not 1.
 *************************************************************/

/**
 * Address start with: `TLS` is transported under TLS.
 **/

class client_core_tls : public client_core{
protected:
	const SSL_METHOD *method; //nullptr?
	SSL_CTX *ctx {nullptr};
	bool *TLS {nullptr}; // Record which server is transported under TLS in location.
	SSL **ssl {nullptr}; // runner list with TLS
	bool TLS_init();
	inline void close_runner(short int index);
public:
	client_core_tls();
	~client_core_tls();
	bool import_location(std::vector<std::string> *location, short int unsigned amount = 0);
};
#include <hast/client_core_tls.cpp>
#endif /* hast_client_core_tls_hpp */

