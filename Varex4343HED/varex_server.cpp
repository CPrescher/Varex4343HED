#include "varex_server.h"
#include <iostream>

namespace varex_server {
	int get_status(unsigned int detector_index) {
		std::cout << "Requested status of Detector " <<
			detector_index << "." << std::endl;
		return 0;
	}

	int start_collection(unsigned int detector_index) {
		std::cout << "Starting data collection for detector " <<
			detector_index << "." << std::endl;
		return 0;
	}

	int stop_collection(unsigned int detector_index) {
		std::cout << "Stopping data collection for detector " <<
			detector_index << "." << std::endl;
		return 0;
	}

	int set_exposure_time(unsigned int detector_index, double new_time) {
		std::cout << "Setting exposure time of detector " <<
			detector_index << " to " << new_time << std::endl;
		return 0;
	}

	double get_exposure_time(unsigned int detector_index) {
		std::cout << "Requesting exposure time for detector " <<
			detector_index << "." << std::endl;
		return 0.1;
	}

	int set_gain(unsigned int detector_index, int new_gain) {
		std::cout << "Setting gain for detector " <<
			detector_index << " to " << new_gain << "." << std::endl;
		return 0;
	}

	int get_gain(unsigned int detector_index) {
		std::cout << "Getting gain for detector " <<
			detector_index << "." << std::endl;
		return 4;
	}

	int set_streaming_target(unsigned int detector_index, const char* ip, const char* port) {
		std::cout << "Setting streaming target for detector " << detector_index << " to "
			<< ip << ":" << port << "." << std::endl;
		return 0;
	}

	std::string get_streaming_target(unsigned int detector_index) {
		std::cout << "Getting streaming target for detector " << detector_index << std::endl;
		return "0.0.0.0:1234";
	}
}
