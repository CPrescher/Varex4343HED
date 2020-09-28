#pragma once

//We need to add "Ws2_32.lib" to the LINKER settings:
#pragma comment(lib, "Ws2_32.lib")

//Socket header files
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>

#define SCK_VERSION2 0x0202

long start_WSA();
void get_addressinfo(const char*& address, const char*& port, addrinfo*& servinfo);
std::string get_client_ip(SOCKET s);

SOCKET start_listening(addrinfo*& servinfo);
SOCKET start_server(const char* address, const char* port);
SOCKET accept_client(SOCKET server_socket);