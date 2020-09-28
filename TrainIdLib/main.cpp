#include "train_info.h"

int main() {
	TrainUSB::start_update_train_info_thread();
	while (true) {
		Sleep(10);
		std::cout << "TrainID: " << TrainUSB::get_current_train_id() << std::endl;
	}
}