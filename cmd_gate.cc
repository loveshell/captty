#include "partty.h"
#include "uniext.h"
#include <kazuhiki/network.h>

void usage(void)
{
	std::cout
		<< "\n"
		<< "* Partty Gate (listen guest and relay it to the server)\n"
		<< "   [listen gate]$ partty-gate  [options]  # use default port ["<<Partty::GATE_DEFAULT_PORT<<"]\n"
		<< "   [listen gate]$ partty-gate  [options]  <port number>\n"
		<< "   [listen gate]$ partty-gate  [options]  <listen address>[:<port number>]\n"
		<< "   [listen gate]$ partty-gate  [options]  <network interface name>[:<port number>]\n"
		<< "\n"
		<< "   options:\n"
		<< "     -r                       use raw mode instead of telnet\n"
		<< "\n"
		<< std::endl;
}

int listen_socket(struct sockaddr_storage& saddr)
{
	int ssock = socket(saddr.ss_family, SOCK_STREAM, 0);
	if( ssock < 0 ) {
		perror("socket");
		return -1;
	}
	int on = 1;
	if( setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0 ) {
		perror("setsockopt");
		return -1;
	}
	socklen_t saddr_len = (saddr.ss_family == AF_INET) ?
		sizeof(struct sockaddr_in) :
		sizeof(struct sockaddr_in6);
	if( bind(ssock, (struct sockaddr*)&saddr, saddr_len) < 0 ) {
		perror("can't bind the address");
		return -1;
	}
	if( listen(ssock, 5) < 0 ) {
		perror("listen");
		return -1;
	}
	return ssock;
}

int main(int argc, char* argv[])
{
	struct sockaddr_storage saddr;
	bool use_raw = false;
	try {
		using namespace Kazuhiki;
		Parser kz;
		--argc;  ++argv;  // skip argv[0]
		kz.on("-r", "--raw", Accept::Boolean(use_raw));
		kz.break_parse(argc, argv);
		if( argc == 0 ) {
			Convert::AnyAddress(saddr, Partty::GATE_DEFAULT_PORT);
		} else if( argc == 1 ) {
			Convert::FlexibleListtenAddress(argv[0], saddr, Partty::GATE_DEFAULT_PORT);
		} else {
			usage();
			return 1;
		}

	} catch (Kazuhiki::ArgumentError& e) {
		std::cerr << "error: " << e.what() << std::endl;
		usage();
		return 1;
	}

	int ssock = listen_socket(saddr);
	if( ssock < 0 ) {
		return 1;
	}

	try {
		if( use_raw ) {
			Partty::RawGate::config_t config(ssock);
			Partty::RawGate gate(config);
			return gate.run();
		} else {
			Partty::Gate::config_t config(ssock);
			Partty::Gate gate(config);
			return gate.run();
		}
	} catch (Partty::partty_error& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}
}

