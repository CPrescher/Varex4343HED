#pragma once

#include <string>

namespace varex_server {
	int get_status(unsigned int detector_index);
	
	int start_collection(unsigned int detector_index);
	int stop_collection(unsigned int detector_index);

	int set_exposure_time(unsigned int detector_index, double new_time);
	double get_exposure_time(unsigned int detector_index);

	int set_gain(unsigned int detector_index, int new_gain);
	int get_gain(unsigned int detector_index);

	int set_streaming_target(unsigned int detector_index, const char* ip, const char* port);
	std::string get_streaming_target(unsigned int detector_index);
}
