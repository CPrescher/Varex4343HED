#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <plog/Log.h> // logging library
#include <plog/Appenders/ConsoleAppender.h>
#include "server.h"
#include "handle.h"
#include "varex.h"
#include "train_info.h"

int main() {
	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("varex.log", 10000000, 5); // Create the 1st appender.
	static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender; // Create the 2nd appender.
	plog::init(plog::info, &fileAppender).addAppender(&consoleAppender); // Initialize the logger with the both appenders.

	try {
		TrainUSB::start_update_train_info_thread();
		std::vector<varex::Detector> detectors = varex::get_detectors();
		//varex::enable_logging();
		start_WSA();
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