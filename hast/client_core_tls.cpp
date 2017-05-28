client_core_tls::client_core_tls(){
	
}

client_core_tls::~client_core_tls(){
	short int a;
	if(ssl!=nullptr){
		for(a=0;a<amount;++a){
			if(ssl[a]!=nullptr){
				SSL_free(ssl[a]);
				ssl[a] = nullptr;
			}
		}
		delete [] ssl;
		ssl = nullptr;
	}
	if(TLS!=nullptr){
		delete [] TLS;
		TLS = nullptr
	}
	if(ctx!=nullptr){
		SSL_CTX_free(ctx);
	}
}

bool client_core_tls::TLS_init(){
	/* ---------------------------------------------------------- *
	 * These function calls initialize openssl for correct work.  *
	 * ---------------------------------------------------------- */
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	/* ---------------------------------------------------------- *
	 * initialize SSL library and register algorithms             *
	 * ---------------------------------------------------------- */
	if(SSL_library_init() < 0){
		//std::cout << "Could not initialize the OpenSSL library !" << std::endl;
		return false;
	}
	/* ---------------------------------------------------------- *
	 * Set SSLv2 client hello, also announce SSLv3 and TLSv1      *
	 * ---------------------------------------------------------- */
	method = SSLv23_client_method();
	/* ---------------------------------------------------------- *
	 * Try to create a new SSL context                            *
	 * ---------------------------------------------------------- */
	if ( (ctx = SSL_CTX_new(method)) == NULL){
		return false;
		//std::cout << "Unable to create a new SSL context structure." << std::endl;
	}
	/* ---------------------------------------------------------- *
	 * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
	 * ---------------------------------------------------------- */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
}

bool client_core_tls::import_location(std::vector<std::string> *location, short int unsigned amount){
	if(TLS_init()==false){
		return false;
	}
	short int a;
	a = location->size();
	if(a==0){
		return false;
	}
	TLS = new bool [a];
	--a
	for(;a>=0;--a){
		if((*location)[a].substr(0,3)=="TLS"){
			(*location)[a] = (*location)[a].substr(3);
			TLS[a] = true;
		}
		else{
			TLS[a] = false;
		}
	}
	if(amount==0){
		a = location->size();
	}
	else{
		a = amount;
	}
	ssl = new *SSL [a];
	--a;
	for(;a>=0;--a){
		ssl[a] = nullptr;
	}
	client_core::import_location(location,amount);
	return true;
}
