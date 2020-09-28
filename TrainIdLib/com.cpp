#include "com.h"

// actual implementations
HANDLE open_port(LPCWSTR PortSpecifier) {
	return CreateFile(
		PortSpecifier,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
}

void close_port(HANDLE hPort) {
	CloseHandle(hPort);
}

DCB set_dcb(HANDLE hPort) {
	DCB dcb;
	GetCommState(hPort, &dcb);
	dcb.BaudRate = CBR_115200; //115200 Baud
	dcb.ByteSize = 8; //8 data bits
	dcb.Parity = NOPARITY; //no parity
	dcb.StopBits = ONESTOPBIT; //1 stop
	SetCommState(hPort, &dcb);
	return dcb;
}


char read_byte(HANDLE hPort) {
	char Byte = 0;
	DWORD dwBytesTransferred;
	ReadFile(hPort, &Byte, 1, &dwBytesTransferred, 0); //read 1 byte
	return Byte;
}

void align_to_start(HANDLE hPort) {
	while (read_byte(hPort) != stx);
}

std::string read_string(HANDLE hPort, int num_characters) {
	std::string result;
	for (int i = 0; i < num_characters; i++) {
		result.push_back(read_byte(hPort));
	}
	return result;
}