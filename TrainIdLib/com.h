#pragma once
#include <windows.h>
#include <string>

HANDLE open_port(LPCWSTR PortSpecifier);
void close_port(HANDLE hPort);
DCB set_dcb(HANDLE hPort);

char read_byte(HANDLE hPort);
void align_to_start(HANDLE hPort);
std::string read_string(HANDLE hPort, int num_characters);

constexpr char stx = '\x02';
constexpr char etx = '\x03';