// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "system.h"

//static AudioDeviceManager deviceManager;

vector<uint8_t> hexStringToBytes(const string& hex);

class launchpad
{
private:
	class midicallback;
	class LED_global_timer;
	struct VRGB;

	static set<launchpad*> launchpad_list;
	static LED_global_timer LED_timer;
	static const map<string, tuple<string, uint8_t>> VPID_map;
	static SpinLock mutex;

//private:
	string kind_name;
	shared_ptr<AudioDeviceManager> deviceManager;
	unique_ptr<MidiInput> input;
	unique_ptr<MidiOutput> output;
	unique_ptr<midicallback> input_callback;
	bool automatic_transmission;
	
	const int LED_w = 10;
	const int LED_h = 10;
	vector<vector<VRGB>> LED_grid_current; //vrgb
	vector<vector<VRGB>> LED_grid_target;
	unique_ptr<uint8[]> LED_raw_data;

	class midicallback : public MidiInputCallback
	{
		void handleIncomingMidiMessage(MidiInput* /*source*/, const MidiMessage& message) override;
		void printHex(const uint8_t* data, size_t length);
	};

	class LED_global_timer : public HighResolutionTimer
	{
		void hiResTimerCallback() override;
	};

	struct VRGB
	{
		uint8 v; //velocity(0 ~ 127, 0xFF: RGB모드)
		uint8 r;
		uint8 g;
		uint8 b;
		bool operator==(const VRGB& lhs)
		{
			return lhs.v == this->v && lhs.r == this->r && lhs.g == this->g && lhs.b == this->b;
		}
	};

	void init();
public:
	//launchpad();
	//launchpad(String kind);
	launchpad(shared_ptr<AudioDeviceManager>, MidiDeviceInfo);
	~launchpad();

	void send_message_now(MidiMessage& message);
	void send_hex(const string& message);
	void send_LED();
	void set_rgb(uint8, uint8, uint8, uint8, uint8);
	void set_velocity(uint8, uint8, uint8);
	void set_porgream_mode(bool = true);
	static Array<MidiDeviceInfo> get_available_list();
};


