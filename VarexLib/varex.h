#pragma once
#include "Acq.h"
#include <exception>
#include <vector>

#define STDGAIN 4

typedef UINT16 PixelT;

#define BUFALIGNMENT 4096
#define BUFFERSPERDET 256

#define PIXELSPERFRAME(detdescp) ((((UINT64)((detdescp)->rows)) * ((UINT64)((detdescp)->columns))))
#define BYTESPERFRAME(detdescp) (PIXELSPERFRAME(detdescp) * sizeof(PixelT))

#define PIXELSPERBUFFER(detdescp) (PIXELSPERFRAME(detdescp) * ((detdescp)->buff_frames))
#define BYTESPERBUFFER(detdescp) (PIXELSPERBUFFER(detdescp) * sizeof(PixelT))

namespace varex {
	enum AcquisitionState {
		Idle = 0, InProgress, Live
	};

	struct Detector {
		HACQDESC handle;
		UINT id;
		enum AcquisitionState state;
		PixelT* acqbuffP;
		PixelT* liveframeP;
		UINT16 buff_frames;
		WORD gain;
		UINT16 rows, columns;
		HANDLE acqEndEvent;

		Detector();

		void set_exposure_time(unsigned int milli_seconds);
		void set_gain(unsigned int gain);
		void enable_internal_trigger();
		void enable_external_trigger();
		void start_acquisition();
	};

	std::vector<Detector> get_detectors();
	unsigned int get_detector_num();
	void get_next_varex(ACQDESCPOS desc_pos, const int index, Detector& detector);

	void CALLBACK frame_callback(HACQDESC detector_handle);

	class VarexException : public std::exception {
	public: 
		VarexException(const char* msg) : std::exception(msg) {};
	};
}