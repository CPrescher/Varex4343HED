#include "server.h"
#include "varex.h"
#include "handle.h"
#include <plog/Log.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

using std::vector;
using std::string;

constexpr int c_recv_buffer_size = 100;

void handle_connection(SOCKET s, std::vector<varex::Detector>* p_detectors)
// handles the communication between client and the Varex control server
// it is awaiting commands from the client and acts accordingly
{
	string ip = get_client_ip(s);

	PLOGD << ip << " connected. Waiting for Message.";
	PLOGD << p_detectors->size() << " detectors are available.";

	char recv_buffer[c_recv_buffer_size];
	while (recv(s, recv_buffer, c_recv_buffer_size - 1, NULL) > 0) {
		string command = read_command(recv_buffer);
		try {
			handle_command(s, p_detectors, command);
			//send_success(s);
		}
		catch(const std::exception& e) {
			PLOGD << "Unable to understand command: " << command;
			PLOGD << "Exception: " << e.what();
			//send_failed(s);
		};
	}
	PLOGD << "Connection was closed by the client";
}

void handle_command(SOCKET s, std::vector<varex::Detector>* p_detectors, const string& command) {
	vector<string> command_parts = split_command(command);
	int det_index = stoi(command_parts.at(1));
	string verb = command_parts.at(2);

	if (verb == "set") {
		set_param(s, p_detectors->at(det_index), command_parts);
	}
	else if (verb == "get") {
		get_param(s, p_detectors->at(det_index), command_parts);
	}
	else if (verb == "start_acquisition") {
		p_detectors->at(det_index).start_acquisition();
	}
	else if (verb == "stop_acquisition") {
		p_detectors->at(det_index).stop_acquisition();
	}
	else {
		PLOGD << "Unable to understand command: " << command;
	}
}

void set_param(SOCKET s, varex::Detector& detector, const vector<string>& command_parts) {
	string parameter = command_parts.at(3);
	if (parameter == "gain") {
		int value = stoi(command_parts.at(4));
		detector.set_gain(value);
	}
	else if (parameter == "exposure_time") {
		double value = stod(command_parts.at(4));
		detector.set_exposure_time(value);
	}
	else if (parameter == "trigger_mode") {
		int value = stoi(command_parts.at(4));
		if (value) {
			detector.enable_external_trigger();
		}
		else {
			detector.enable_internal_trigger();
		}
	}
	else if (parameter == "streaming_target") {
		detector.set_streaming_target(command_parts.at(4), command_parts.at(5));
	}
	else {

	}
}

void get_param(SOCKET s, varex::Detector& detector, const vector<string>& command_parts) {
	string parameter = command_parts.at(3);
	if (parameter == "status") {
		char status = (char)detector.get_status();
		send(s, (char *)&status, sizeof(status), NULL);
	}
	else if (parameter == "gain") {
		char gain = detector.get_gain();
		PLOGD << "Gain requested.";
		send(s, &gain, sizeof(gain), NULL);
	}
	else if (parameter == "exposure_time") {
		int exposure_time = detector.get_exposure_time();
		PLOGD << "Exposure time requested.";
		send(s, (char*)&exposure_time, sizeof(exposure_time), NULL);
	}
	else if (parameter == "trigger_mode") {
		char trigger_mode = (char)detector.get_trigger_mode();
		PLOGD << "Trigger mode requested.";
		send(s, (char*)&trigger_mode, sizeof(trigger_mode), NULL);
	}
	/*
	else if (parameter == "streaming_target") {
		string target = varex_server::get_streaming_target(det_index);
	}*/
	// still needs to send it back...
}

string read_command(const char* recv_buffer) {
	std::istringstream recv_stream(recv_buffer);
	string recv_str;
	std::getline(recv_stream, recv_str, ';');
	return recv_str;
}

vector<string> split_command(const string& command) {
	return split(command, ':');
}

vector<string> split(const string& str, char delimiter)
{
	vector<string> tokens;
	string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

void send_success(SOCKET s) {
	send(s, "1", 1, NULL);
}

void send_failed(SOCKET s) {
	send(s, "0", 1, NULL);
}