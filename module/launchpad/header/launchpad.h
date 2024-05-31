// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include "lightshow.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace uniq
{
	class launchpad : public ID<launchpad>
	{
	private:
		class midi_callback;
		class LED_global_timer;
		struct VRGB;
		
		static std::set<launchpad*> launchpad_list;
		static std::unique_ptr<LED_global_timer> LED_timer;
		static const std::map<std::string, std::tuple<std::string, juce::uint8>> VPID_map;
		static juce::SpinLock mutex;
		
		std::string midi_input_kind_name;
		std::string midi_output_kind_name;
		std::shared_ptr<juce::AudioDeviceManager> deviceManager;
		std::unique_ptr<juce::MidiInput> input;
		std::unique_ptr<juce::MidiOutput> output;
		std::unique_ptr<midi_callback> input_callback;
		bool automatic_transmission;
		bool immediate_transmission;
		int interval = 4; //ms 단위

		inline static int input_callback_function_map_index = 0;
		std::map<int, std::function<void(const std::uint8_t*, int)>> input_callback_function_map_;
		std::map<int, std::function<void(std::uint8_t, std::uint8_t, std::uint8_t)>> input_button_down_callback_map_;
		std::map<int, std::function<void(std::uint8_t, std::uint8_t)>> input_button_up_callback_map_;

		const int LED_w = 10;
		const int LED_h = 10;
		std::vector<std::vector<VRGB>> LED_grid_current; //vrgb
		std::vector<std::vector<VRGB>> LED_grid_target;
		std::unique_ptr<juce::uint8[]> LED_raw_data;

		using rgbav_id_array = lightshow::lightshow::rgbav_id_array;
		rgbav_id_array rgbav_grid_current;
		rgbav_id_array rgbav_grid_target;
		std::shared_ptr<lightshow::lightshow> lightshow_;
		
		class midi_callback : public juce::MidiInputCallback
		{
			// void(launchpad::*callback_function)(const std::uint8_t*, int) = nullptr;
			std::function<void(const std::uint8_t*, int)> callback_function;
			// std::set<std::function<void(std::uint8_t*, int)>> callback_function_set_;
			// std::set<void(*)(std::uint8_t*, int)> callback_function_set_;
			// std::map<int, std::function<void(std::uint8_t*, int)>> callback_function_map_;
			// inline static int callback_function_map_index = 0;
			void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override;
			void printHex(const uint8_t*, size_t);
		public:
			void callback_set(std::function<void(const std::uint8_t*, int)> callback);
			// int callback_add(std::function<void(std::uint8_t*, int)> &&callback);
			// bool callback_remove(int callback_id);
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
		static std::string launchpad_kind_name_get(juce::MidiDeviceInfo& mdi);
		void input_button_callback(const std::uint8_t*, int) const;
	public:
		class midi_device_info : public juce::MidiDeviceInfo
		{
		public:
			std::string kind_name = "none";
			midi_device_info(const juce::MidiDeviceInfo&& info);
			midi_device_info(const juce::MidiDeviceInfo&& info, const juce::String& name);
		};
		
		//launchpad();
		//launchpad(String kind);
		//launchpad(std::shared_ptr<juce::AudioDeviceManager>, const midi_device_info&);
		launchpad(std::shared_ptr<juce::AudioDeviceManager>&);
		launchpad(std::shared_ptr<juce::AudioDeviceManager>&, const midi_device_info&, const midi_device_info&);
		launchpad(const std::shared_ptr<audio_device_manager>&, const midi_device_info*, const midi_device_info*);
		~launchpad();
		
		bool midi_input_set(const midi_device_info&);
		bool midi_output_set(const midi_device_info&);
		void message_send_now(juce::MidiMessage&);
		void hex_send(const juce::String&);
		void hex_send(const juce::uint8*, std::size_t);
		void LED_send();
		void rgbav_grid_calculate();
		void rgb_set(juce::uint8, juce::uint8, juce::uint8, juce::uint8, juce::uint8);
		void velocity_set(juce::uint8, juce::uint8, juce::uint8);
		void program_mode_set(bool = true);
		void automatic_transmission_set(bool = true);
		void immediate_transmission_set(bool = true);
		static void immediate_transmission_global_timer_set(int);

		auto lightshow_get() -> std::shared_ptr<lightshow::lightshow>;

		[[nodiscard]]
		int input_callback_add(std::function<void(const std::uint8_t *, int)>&& callback);
		bool input_callback_remove(int callback_id);
		[[nodiscard]]
		auto input_button_down_callback_add(std::function<void(std::uint8_t, std::uint8_t, std::uint8_t)>&& callback) -> int;
		bool input_button_down_callback_remove(int callback_id);
		[[nodiscard]]
		auto input_button_up_callback_add(std::function<void(std::uint8_t, std::uint8_t)>&& callback) -> int;
		bool input_button_up_callback_remove(int callback_id);
		static std::vector<midi_device_info> get_available_input_list();
		static std::vector<midi_device_info> get_available_output_list();
		std::string input_identifier_get();
		std::string output_identifier_get();
	};
	
	std::vector<juce::uint8> hexStringToBytes(const juce::String&);
}