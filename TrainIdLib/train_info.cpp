#include "train_info.h"
#include "com.h"
#include <thread>


namespace TrainUSB {
	TrainInfo read_train_info(HANDLE hPort) {
		return {
			read_string(hPort, 16),
			read_string(hPort, 8),
			read_string(hPort, 2)
		};
	}

	std::ostream& operator<<(std::ostream& os, const TrainInfo& train_info) {
		__int64 train_id_num = hex2int(train_info.id);
		std::cout << "Train: " << train_id_num << " ";
		std::cout << "Beam Mode: " << train_info.beam_mode << " ";
		std::cout << "CRC: " << train_info.crc;
		return os;
	}

	__int64 hex2int(std::string hex) {
		__int64 result;
		std::stringstream ss;
		ss << std::hex << hex;
		ss >> result;
		return result;
	}

	TrainInfo train_info;

	void start_update_train_info_thread() {
		HANDLE hPort = open_port(L"COM3");
		set_dcb(hPort);
		std::thread(update_train_info, hPort).detach();
	}

	void update_train_info(HANDLE hPort) {
		while (true) {
			align_to_start(hPort);
			train_info = TrainUSB::read_train_info(hPort);
		}
	}

	TrainInfo get_train_info() {
		return train_info;
	}

	__int64 get_current_train_id() {
		return hex2int(train_info.id);
	}
}

