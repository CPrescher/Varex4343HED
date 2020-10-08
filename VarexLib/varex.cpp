#include "varex.h"
#include "train_info.h"
#include "server.h"
#include <iostream>
#include <vector>
#include <string>

namespace varex {
	std::vector<Detector> get_detectors() {
		std::cout << "Initializing detectors..." << std::endl;
		unsigned int num_detectors = get_detector_num();
		std::cout << "Found " << num_detectors << " detectors." << std::endl;

		std::vector<Detector> detectors;
		ACQDESCPOS desc_pos = 0;

		for (int i = 0; i < num_detectors; i++) {
			varex::Detector* detector = new Detector();
			varex::get_next_varex(desc_pos, i, *detector);
			detector->enable_internal_trigger();
			detector->set_exposure_time(100);
			//detector->start_acquisition();
			detectors.push_back(*detector);
		}
		return detectors;
	}

	unsigned int get_detector_num() {
		unsigned int uiNumSensors;
		int iRet = Acquisition_EnumSensors(&uiNumSensors, true, false);
		return uiNumSensors;
	}

	void get_next_varex(ACQDESCPOS desc_pos, const int index, Detector& detector)
	{
		HACQDESC detector_handle = NULL;
		CHwHeaderInfo header_info;
		CHwHeaderInfoEx header_info_ex;
		if (Acquisition_GetNextSensor(&desc_pos, &detector_handle) != HIS_ALL_OK) {
			throw VarexException("Error accessing sensor.");
		};
		Acquisition_GetHwHeaderInfoEx(detector_handle, &header_info, &header_info_ex);

		int channel;
		UINT channel_type;
		if (Acquisition_GetCommChannel(detector_handle, &channel_type, &channel)) {
			throw VarexException("Failed to request communication channel for sensor.");
		}

		// detector info parameters:
		UINT frames, rows, columns, data_type, sort_flags;
		BOOL irq_enabled;
		DWORD acq_type, system_id, sync_mode, access;

		// initialize the detector
		Acquisition_Abort(detector_handle);
		Acquisition_SetCameraMode(detector_handle, 0);
		Acquisition_SetCameraGain(detector_handle, STDGAIN);

		if (Acquisition_GetConfiguration(detector_handle,
			&frames, &rows, &columns, &data_type, &sort_flags, &irq_enabled,
			&acq_type, &system_id, &sync_mode, &access) != HIS_ALL_OK) {
			throw VarexException("Error requesting configuration of sensor.");
		}

		detector.id = index;
		detector.handle = detector_handle;
		detector.rows = rows;
		detector.columns = columns;
		detector.buff_frames = BUFFERSPERDET;
		detector.liveframeP = (PixelT*)_aligned_malloc(BYTESPERFRAME(&detector), BUFALIGNMENT);
		detector.acqbuffP = (PixelT*)_aligned_malloc(BYTESPERBUFFER(&detector), BUFALIGNMENT);
		if (detector.liveframeP != 0 and detector.acqbuffP != 0) {
			memset(detector.liveframeP, 0, BYTESPERFRAME(&detector));
			memset(detector.acqbuffP, 0, BYTESPERBUFFER(&detector));
		}
		else {
			throw VarexException("unable to allocate memory.");
		}
		Acquisition_SetAcqData(detector.handle, &detector);

		if (Acquisition_DefineDestBuffers(detector.handle, detector.acqbuffP,
			detector.buff_frames, detector.rows, detector.columns) != HIS_ALL_OK) {
			throw VarexException("Error defining destination buffers.");
		}

		Acquisition_ResetFrameCnt(detector.handle);

		if (Acquisition_SetCallbacksAndMessages(detector.handle, NULL, 0, 0,
			frame_callback, NULL) != HIS_ALL_OK) {
			throw VarexException("Error while defining frame callback for detector.");
		}
	}

	Detector::Detector() {
		state = varex::AcquisitionState::Idle;
		trigger_mode = varex::TriggerMode::Internal;
		gain = STDGAIN;
		exposure_time = 12;
	}

	varex::AcquisitionState Detector::get_status() {
		return state;
	}

	void Detector::set_exposure_time(uint32_t milli_seconds) {
		exposure_time = milli_seconds;
		DWORD sync_time = milli_seconds * 1000; // set in microseconds 
		if (Acquisition_SetTimerSync(handle, &sync_time) == HIS_ALL_OK) {
			std::cout << "Exposure time for Varex " << id <<" set to: " 
				<< milli_seconds << " ms" << std::endl;
		}
		else {
			std::cout << "Failed to set exposure time for Varex " << id << 
				" to: " << milli_seconds << " ms" << std::endl;
		}
	}

	uint32_t Detector::get_exposure_time() {
		return exposure_time;
	}

	void Detector::set_gain(uint32_t gain) {
		if (gain >= 1 and gain <= 7) {
			if (Acquisition_SetCameraGain(handle, gain) == HIS_ALL_OK) {
				std::cout << "Gain for Varex " << id << " set to: " 
					<< gain << std::endl;
			}
			else {
				std::cout << "Failed to set gain for Varex " << id 
					<< " to: " << gain << std::endl;
			}
		}
		else {
			std::cout << "Could not set gain to " << gain 
				<< ". Value is out of range=[1, 7])" << std::endl;
		}
	}

	int Detector::get_gain() {
		return gain;
	}


	void Detector::enable_external_trigger() {
		if (Acquisition_SetFrameSyncMode(handle, HIS_SYNCMODE_EXTERNAL_TRIGGER) 
			== HIS_ALL_OK)
		{
			std::cout << "Set Varex " << id << 
				" to external trigger " << std::endl;
			trigger_mode = varex::TriggerMode::External;
		}
		else 
		{
			std::cout << "Failed to set Varex " << id <<
				" to external trigger " << std::endl;
		}
	}
	void Detector::enable_internal_trigger() {
		if(Acquisition_SetFrameSyncMode(handle, HIS_SYNCMODE_INTERNAL_TIMER)
			== HIS_ALL_OK)
		{
			std::cout << "Set Varex " << id <<
				" to internal trigger " << std::endl;
			trigger_mode = varex::TriggerMode::Internal;
		}
		else
		{
		std::cout << "Failed to set Varex " << id <<
			" to internal trigger " << std::endl;
		}
	}

	varex::TriggerMode Detector::get_trigger_mode() {
		return trigger_mode;
	}

	void Detector::start_acquisition() {
		if (Acquisition_Acquire_Image(handle, 1, 0, HIS_SEQ_CONTINUOUS, NULL, NULL, NULL)
			== HIS_ALL_OK)
		{
			std::cout << "Start acquisition for Varex " << id << "." << std::endl;
			state = AcquisitionState::Collecting;
		}
		else {
			std::cout << "Failed to start acquisition for Varex " << id 
					  << "." << std::endl;
		}
	}
	void Detector::stop_acquisition() {
		if (Acquisition_Abort(handle) == HIS_ALL_OK)
		{
			std::cout << "Stopped acquisition for Varex " << id << "." << std::endl;
			state = AcquisitionState::Idle;
		}
		else 
		{
			std::cout << "Failed to stop acquisition for Varex " << id 
				      << "." << std::endl;
		}
	}

	void Detector::set_streaming_target(const std::string& address, const std::string& port) {
		std::cout << "Setting Streaming Target to: " << address <<":" << port << std::endl;
		/*struct addrinfo* servinfo;
		get_addressinfo(address, port, servinfo);*/
	}

	void CALLBACK frame_callback(HACQDESC detector_handle) 
	{
		Detector* detector;
		Acquisition_GetAcqData(detector_handle, (void**)&detector);

		CHwHeaderInfo Info;
		CHwHeaderInfoEx InfoEx; 
		Acquisition_GetLatestFrameHeader(detector_handle, &Info, &InfoEx);

		DWORD dwActFrame, dwSecFrame;
		Acquisition_GetActFrame(detector_handle, &dwActFrame, &dwSecFrame);

		std::cout
			<< "Det " << detector->id
			<< " frame: " << InfoEx.wFrameCnt
			<< " exposure time: " << InfoEx.wRealInttime_milliSec << ","
			<< InfoEx.wRealInttime_microSec << " ms"
			<< " train id: " << TrainUSB::get_current_train_id()
			<< std::endl;

		if (InfoEx.wFrameCnt == 10) {
			std::cout << "trying to increase exposure time..." << std::endl;
			//detector->set_exposure_time(1000);
			detector->set_exposure_time(100);
		}
	}


	
}
