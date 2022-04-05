#pragma once
#include "Acq.h"
#include <exception>
#include <vector>
#include <string>

#define STDGAIN 4

typedef UINT16 PixelT;

#define BUFALIGNMENT 4096
#define BUFFERSPERDET 50

#define PIXELSPERFRAME(detdescp) ((((UINT64)((detdescp)->rows)) * ((UINT64)((detdescp)->columns))))
#define BYTESPERFRAME(detdescp) (PIXELSPERFRAME(detdescp) * sizeof(PixelT))

#define PIXELSPERBUFFER(detdescp) (PIXELSPERFRAME(detdescp) * ((detdescp)->buff_frames))
#define BYTESPERBUFFER(detdescp) (PIXELSPERBUFFER(detdescp) * sizeof(PixelT))

namespace varex {
	enum class AcquisitionState {
		Idle=0, Collecting,
	};

	enum class TriggerMode {
		Internal=0, External
	};

	struct Detector {
		HACQDESC handle;
		UINT id;
		PixelT* acqbuffP;
		PixelT* liveframeP;
		UINT16 buff_frames;
		UINT16 buff_ind;
		WORD gain;
		UINT16 rows, columns;
		HANDLE acqEndEvent;

		Detector();

		void set_exposure_time(uint32_t milli_seconds);
		void set_gain(uint32_t gain);
		void enable_internal_trigger();
		void enable_external_trigger();
		TriggerMode get_trigger_mode();
		void start_acquisition();
		void stop_acquisition();
		void increment_buffer_index();

		void set_streaming_target(const std::string& ip, const std::string& port);
		void send_image(bool with_train_id = false);

		AcquisitionState get_status();
		uint32_t get_exposure_time();
		int get_gain();

	private:
		uint32_t exposure_time; // in ms;
		enum AcquisitionState state;
		enum TriggerMode trigger_mode;
		SOCKET streaming_socket;
		std::string streaming_address;
		std::string streaming_port;
	};

	std::vector<Detector> get_detectors();
	unsigned int get_detector_num();
	void get_next_varex(ACQDESCPOS desc_pos, const int index, Detector& detector);

	void CALLBACK frame_callback(HACQDESC detector_handle);

	class VarexException : public std::exception {
	public: 
		VarexException(const char* msg) : std::exception(msg) {};
	};

	long enable_logging();
}