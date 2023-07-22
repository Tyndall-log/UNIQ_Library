// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "../system.h"

namespace uniq
{
	class launchpad
	{
	private:
		class midicallback;
		class LED_global_timer;
		struct VRGB;

		static std::set<launchpad*> launchpad_list;
		static LED_global_timer LED_timer;
		static const std::map<std::string, std::tuple<std::string, juce::uint8>> VPID_map;
		static juce::SpinLock mutex;

		std::string kind_name;
		std::shared_ptr<juce::AudioDeviceManager> deviceManager;
		std::unique_ptr<juce::MidiInput> input;
		std::unique_ptr<juce::MidiOutput> output;
		std::unique_ptr<midicallback> input_callback;
		bool automatic_transmission;

		const int LED_w = 10;
		const int LED_h = 10;
		std::vector<std::vector<VRGB>> LED_grid_current; //vrgb
		std::vector<std::vector<VRGB>> LED_grid_target;
		std::unique_ptr<juce::uint8[]> LED_raw_data;

		class midicallback : public juce::MidiInputCallback
		{
			void handleIncomingMidiMessage(juce::MidiInput* /*source*/, const juce::MidiMessage& message) override;
			void printHex(const uint8_t* data, size_t length);
		};

		class LED_global_timer : public juce::HighResolutionTimer
		{
			void hiResTimerCallback() override;
		};

		struct VRGB
		{
			juce::uint8 v; //velocity(0 ~ 127, 0xFF: RGB모드)
			juce::uint8 r;
			juce::uint8 g;
			juce::uint8 b;
			bool operator==(const VRGB&) const = default;
		};

		void init();
	public:
		//launchpad();
		//launchpad(String kind);
		launchpad(std::shared_ptr<juce::AudioDeviceManager>, juce::MidiDeviceInfo);
		~launchpad();

		void send_message_now(juce::MidiMessage&);
		void send_hex(const juce::String&);
		void send_hex(const juce::uint8*, std::size_t);
		void send_LED();
		void set_rgb(juce::uint8, juce::uint8, juce::uint8, juce::uint8, juce::uint8);
		void set_velocity(juce::uint8, juce::uint8, juce::uint8);
		void set_porgream_mode(bool = true);
		static juce::Array<juce::MidiDeviceInfo> get_available_list();
	};

	std::vector<juce::uint8> hexStringToBytes(const juce::String&);
}


