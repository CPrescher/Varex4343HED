#pragma once

#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>

namespace TrainUSB {
	struct TrainInfo {
		std::string id;
		std::string beam_mode;
		std::string crc;
	};

	TrainInfo read_train_info(HANDLE hPort);
	std::ostream& operator<<(std::ostream& os, const TrainInfo& train_info);

	__int64 hex2int(std::string hex);

	void start_update_train_info_thread();
	void update_train_info(HANDLE hPort);

	TrainInfo get_train_info();
	__int64 get_current_train_id();
}