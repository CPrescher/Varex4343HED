#pragma once
#include "Acq.h"
#include <iostream>

typedef UINT16 PixelT;

constexpr UINT STDGAIN = 4;
constexpr UINT BUFALIGNMENT = 4096;
constexpr UINT ROWS = 2880;
constexpr UINT COLUMNS = 2880;
constexpr UINT BUFFER_FRAMES = 10;

__int64 current_frame = 0;  //global frame counter

void CALLBACK frame_callback(HACQDESC detector_handle) {
	std::cout << "current frame number: " << ++current_frame << std::endl;
}


int main() {
	unsigned int uiNumSensors;
	Acquisition_EnumSensors(&uiNumSensors, true, false); 

	HACQDESC detector_handle = NULL;
	ACQDESCPOS desc_pos = 0;
	if (Acquisition_GetNextSensor(&desc_pos, &detector_handle) != HIS_ALL_OK) {
		exit;
	};

	Acquisition_Abort(detector_handle);
	Acquisition_SetCameraMode(detector_handle, 0);
	Acquisition_SetCameraGain(detector_handle, 4);

	__int64 buffer_size = ROWS * COLUMNS * BUFFER_FRAMES * sizeof(PixelT);
	PixelT* acqbuffP = (PixelT*)_aligned_malloc(buffer_size, BUFALIGNMENT);
	if (acqbuffP != 0) {
		memset(acqbuffP, 0, buffer_size);
	}
	else {
		exit;
	}

	int ires = Acquisition_DefineDestBuffers(detector_handle, acqbuffP, 1, ROWS, COLUMNS);
	if (ires != HIS_ALL_OK) {
		std::cout << "ERROR: " << ires << " while defining DestBuffers." << std::endl;
	}


	int milli_seconds = 100;
	DWORD sync_time = milli_seconds * 1000;
	Acquisition_SetFrameSyncMode(detector_handle, HIS_SYNCMODE_INTERNAL_TIMER); 
	Acquisition_SetTimerSync(detector_handle, &sync_time) == HIS_ALL_OK;

	if (Acquisition_SetCallbacksAndMessages(detector_handle, NULL, 0, 0,
		frame_callback, NULL) != HIS_ALL_OK) {
		std::cout << "Error while defining frame callback for detector." << std::endl;
	}
	Acquisition_Acquire_Image(detector_handle, BUFFER_FRAMES, 0, HIS_SEQ_CONTINUOUS, NULL, NULL, NULL);

	while (true) {};
	std::cout << "SUCCESS" << std::endl;
}