#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include "server.h"
#include "handle.h"
#include "varex.h"
#include "train_info.h"

int main() {
	try {
		TrainUSB::start_update_train_info_thread();
		std::vector<varex::Detector> detectors = varex::get_detectors();
		SOCKET entry_socket = start_server("0.0.0.0", "3490");

		while (true) 
		{
			SOCKET client_socket = accept_client(entry_socket);
			std::thread t(handle_connection, client_socket, &detectors);
			t.detach();
		}
	}
	catch (varex::VarexException& ex) {
		std::cout << "### Exception occured! Error: " << ex.what() << std::endl;
	}
}