#include "server.h"
#include "varex.h"
#include "train_info.h"
#include <plog/Log.h>
#include <iostream>
#include <vector>
#include <string>


namespace varex {
	__int64 frame_count = 0;

	std::vector<Detector> get_detectors() {
		PLOGI << "Initializing detectors...";
		unsigned int num_detectors = get_detector_num();
		PLOGI << "Found " << num_detectors << " detectors.";

		std::vector<Detector> detectors;
		ACQDESCPOS desc_pos = 0;

		for (int i = 0; i < num_detectors; i++) {
			varex::Detector* detector = new Detector();
			varex::get_next_varex(desc_pos, i, *detector);
			detector->enable_internal_trigger();
			detector->set_exposure_time(100);
			detector->start_acquisition();
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
		detector.acqbuffP = (PixelT*)_aligned_malloc(BYTESPERBUFFER(&detector), BUFALIGNMENT);
		if (detector.acqbuffP != 0) {
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
		streaming_socket = NULL;
		streaming_address = "";
		streaming_port = "";
		buff_ind = 0;
	}

	varex::AcquisitionState Detector::get_status() {
		return state;
	}

	void Detector::set_exposure_time(uint32_t milli_seconds) {
		exposure_time = milli_seconds;
		DWORD sync_time = milli_seconds * 1000; // set in microseconds 
		if (Acquisition_SetTimerSync(handle, &sync_time) == HIS_ALL_OK) {
			PLOGI << "Exposure time for Varex " << id << " set to: "
				<< milli_seconds << " ms";
		}
		else {
			PLOGI << "Failed to set exposure time for Varex " << id <<
				" to: " << milli_seconds << " ms";
		}
	}

	uint32_t Detector::get_exposure_time() {
		return exposure_time;
	}

	void Detector::set_gain(uint32_t gain) {
		if (gain >= 1 and gain <= 7) {
			if (Acquisition_SetCameraGain(handle, gain) == HIS_ALL_OK) {
				PLOGI << "Gain for Varex " << id << " set to: "
					<< gain;
			}
			else {
				PLOGI << "Failed to set gain for Varex " << id
					<< " to: " << gain;
			}
		}
		else {
			PLOGI << "Could not set gain to " << gain << ". Value is out of range=[1, 7])";
		}
	}

	int Detector::get_gain() {
		return gain;
	}


	void Detector::enable_external_trigger() {
		if (Acquisition_SetFrameSyncMode(handle, HIS_SYNCMODE_EXTERNAL_TRIGGER) 
			== HIS_ALL_OK)
		{
			PLOGI << "Set Varex " << id << " to external trigger";
			trigger_mode = varex::TriggerMode::External;
		}
		else 
		{
			PLOGI << "Failed to set Varex " << id << " to external trigger";
		}
	}
	void Detector::enable_internal_trigger() {
		if(Acquisition_SetFrameSyncMode(handle, HIS_SYNCMODE_INTERNAL_TIMER)
			== HIS_ALL_OK)
		{
			PLOGI << "Set Varex " << id << " to internal trigger ";
			trigger_mode = varex::TriggerMode::Internal;
		}
		else
		{
			PLOGI << "Failed to set Varex " << id << " to internal trigger";
		}
	}

	varex::TriggerMode Detector::get_trigger_mode() {
		return trigger_mode;
	}

	void Detector::start_acquisition() {
		if (Acquisition_Acquire_Image(handle, buff_frames, 0, HIS_SEQ_CONTINUOUS, NULL, NULL, NULL)
			== HIS_ALL_OK)
		{
			PLOGI << "Start acquisition for Varex " << id;
			state = AcquisitionState::Collecting;
		}
		else {
			PLOGI << "Failed to start acquisition for Varex " << id;
		}
	}
	void Detector::stop_acquisition() {
		if (Acquisition_Abort(handle) == HIS_ALL_OK)
		{
			PLOGI << "Stopped acquisition for Varex " << id;
			state = AcquisitionState::Idle;
		}
		else 
		{
			PLOGI << "Failed to stop acquisition for Varex " << id;
		}
	}


	void Detector::send_image(bool with_train_id) {
		if (streaming_address == "") {
			return;
		}

		__int64 train_id = TrainUSB::get_current_train_id();
		if (streaming_socket) { 
			 set_streaming_target(streaming_address, streaming_port);
		}
		if (with_train_id and streaming_socket != NULL) {
			if (send(streaming_socket, (char*)&train_id, sizeof(&train_id), 0) != SOCKET_ERROR) {
				PLOGD << "Sent train_id: " << train_id;
			}
			else {
				PLOGW << "ERROR: could not send train_id: " << train_id;
			}
		}
		int image_size = rows * columns * sizeof(PixelT);
		if (send(streaming_socket, (char*)(acqbuffP + buff_ind * PIXELSPERFRAME(this)), BYTESPERFRAME(this), 0) != SOCKET_ERROR) {
			
		}
		else {
			PLOGW << "ERROR: could not send image to streaming target!";
		}
		
	}


	void Detector::set_streaming_target(const std::string& address, const std::string& port) {
		PLOGD << "Setting Streaming Target to: " << address << ":" << port;
		if (streaming_socket){ closesocket(streaming_socket); } //close old socket

		try {
			streaming_socket = connect_to_server(address.c_str(), port.c_str());
			PLOGD << "Connected to server: " << address << ":" << port;
			streaming_address = address;
			streaming_port = port;
		}
		catch (const std::exception& e) {
			PLOGW << "Could not connect to server: " << address << ":" << port;
			PLOGW << "Exception: " << e.what();
			streaming_socket = NULL;
		}

		PLOGD << "streaming socket: " << streaming_socket;

		Acquisition_SetAcqData(handle, this);
	}

	void Detector::increment_buffer_index() {
		buff_ind++;
		if (buff_ind == buff_frames) buff_ind = 0;
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


		PLOGI
			<< "Det " << detector->id
			<< " frame: " << ++frame_count
			<< ", " << InfoEx.wFrameCnt
			<< " exposure time: " << InfoEx.wRealInttime_milliSec << ","
			<< InfoEx.wRealInttime_microSec << " ms"
			<< " train id: " << TrainUSB::get_current_train_id();

		detector->send_image(true);

		detector->increment_buffer_index();
		Acquisition_SetReady(detector_handle, TRUE);
	}

	long enable_logging()
	{
		unsigned int uiRet = HIS_ALL_OK;
		BOOL bEnableLoging = TRUE;
		BOOL consoleOnOff = TRUE;
		XislLoggingLevels xislLogLvl = LEVEL_TRACE;
		BOOL bPerformanceLogging = FALSE;

		// enable loggin functionality
		uiRet = Acquisition_EnableLogging(bEnableLoging);
		if (uiRet != HIS_ALL_OK)
		{
			printf("Acquisition_EnableLogging Error nr.: %d", uiRet);
		}

		// define log outputfile and consolelogging
		uiRet = Acquisition_SetLogOutput("log.txt", consoleOnOff);
		if (uiRet != HIS_ALL_OK)
		{
			printf("Acquisition_SetLogOutput Error nr.: %d", uiRet);
		}

		// set the desired log out level
		uiRet = Acquisition_SetLogLevel(xislLogLvl);
		if (uiRet != HIS_ALL_OK)
		{
			printf("Acquisition_SetLogLevel Error nr.: %d", uiRet);
		}

		// log Performance which will report the time a function call needs
		uiRet = Acquisition_TogglePerformanceLogging(bPerformanceLogging);
		if (uiRet != HIS_ALL_OK)
		{
			printf("Acquisition_TogglePerformanceLogging Error nr.: %d", uiRet);
		}
		return uiRet;
	}
	
}
