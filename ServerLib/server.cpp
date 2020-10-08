#include "server.h"

long start_WSA()
// Runs the WSAStartup function with correct parameters and thus, intiates the
// Winsock DLL.
{
	WSAData win_socket_data;
	WORD dll_version = MAKEWORD(2, 2);
	return WSAStartup(dll_version, &win_socket_data);
}

void get_addressinfo(
	const char*& address,
	const char*& port,
	addrinfo*& server_info
)
// Writes a given ip4 address (e.g. "127.0.0.1") and port number (e.g. "5000") into
// an addinfo struct for later use with the socket library.
{
	struct addrinfo hints;
	int status;
	memset(&hints, 0, sizeof hints); // initialize hints;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(address, port, &hints, &server_info)) != 0) {
		throw std::runtime_error("getaddrinfo error: " +
			(std::string)(char*)gai_strerror(status));
	};
}


SOCKET start_listening(addrinfo*& server_info)
// Starts listening on the given server_info specification. The socket value be returned for 
// futher use.
{
	SOCKET s;
	s = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (s < 0) {
		throw std::runtime_error("Could not create socket!");
	}
	if (bind(s, server_info->ai_addr, server_info->ai_addrlen) == -1) {
		throw std::runtime_error("Could not bind socket!");
	};

	if (listen(s, SOMAXCONN) == -1) {
		throw std::runtime_error("Could not start listening on socket!");
	}
	std::cout << "Socket starts listening!" << std::endl;
	return s;
}


SOCKET start_server(const char* address, const char* port)
// starts the windows socket implementation and listening on given address
// and port. Standard address should be "0.0.0.0" to discover the server from
// outside
{
	start_WSA();
	struct addrinfo* servinfo;
	get_addressinfo(address, port, servinfo);
	return start_listening(servinfo);
}

SOCKET accept_client(SOCKET server_socket)
// Waits for a client to connect and returns SOCKET value after one has connected.
{
	std::cout << "Waiting for new connection." << std::endl;

	SOCKET accepted_socket;
	struct sockaddr_in client_address;
	socklen_t addr_size = sizeof client_address;

	accepted_socket = accept(server_socket, (struct sockaddr*)&client_address, &addr_size);
	if (accepted_socket == -1) {
		throw std::runtime_error("Error in accepting connection");
	}

	// Disables the Nagle algorithm for send coalescing:
	DWORD doptval = 1;
	setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&doptval, sizeof(doptval));

	return accepted_socket;
}

std::string get_client_ip(SOCKET s)
// This function reads out the ip of a connected socket and returns it as a string.
{
	struct sockaddr_in client_address;
	socklen_t addr_size = sizeof client_address;
	if (getpeername(s, (struct sockaddr*)&client_address, &addr_size) != 0) {
		throw std::runtime_error("Could not read client ip.");
	};
	char* clientip = new char[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_address.sin_addr, clientip, INET_ADDRSTRLEN);
	return (std::string) clientip;
}