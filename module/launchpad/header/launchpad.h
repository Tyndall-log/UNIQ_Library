// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include "lightshow.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace uniq::launchpad
{
	class launchpad;

	class launchpad_manager
	{
		static const std::map<std::string, std::tuple<std::string, juce::uint8>> VPID_map;
		static const std::map<std::string, std::string> android_launchpad_map;
		static const std::list<std::tuple<std::string, std::string>> apple_launchpad_list;

		struct launchpad_change_callback_set_compare
		{
			// using is_transparent = void;
			bool operator()(const std::weak_ptr<std::function<void()>>& lhs, const std::weak_ptr<std::function<void()>>& rhs) const;
		};

	public:
		class midi_device_info : public juce::MidiDeviceInfo
		{
		public:
			std::string kind_name = "none";
			explicit midi_device_info(const MidiDeviceInfo&& info);
			midi_device_info(const MidiDeviceInfo&& info, const juce::String& name);
		};
		struct input_output
		{
			std::optional<midi_device_info> input;
			std::optional<midi_device_info> output;
		};

	protected:
		launchpad_manager() = default;
		~launchpad_manager();

	private:
		inline static std::weak_ptr<launchpad_manager> instance_weak_;
		inline static juce::MidiDeviceListConnection midi_device_list_connection_;
		inline static std::map<std::string, std::shared_ptr<launchpad>> launchpad_automatic_map_; //string: launchpad_device_identifier_get()
		// inline static std::map<std::string, std::shared_ptr<launchpad>> launchpad_map_; //string: launchpad_device_identifier_get()
		// inline static std::set<std::shared_ptr<launchpad>> launchpad_automatic_set_;
		inline static std::set<std::shared_ptr<launchpad>> launchpad_set_;
		// inline static std::map<std::string, input_output> input_output_map_;
		inline static std::shared_ptr<message_thread> message_thread_;
		inline static std::shared_ptr<audio_device_manager> audio_device_manager_;
		inline static std::future<void> launchpad_map_update_future_;
		inline static std::set<std::weak_ptr<std::function<void()>>, launchpad_change_callback_set_compare> launchpad_change_callback_set_;

		static std::string launchpad_kind_name_get(juce::MidiDeviceInfo& mdi);
		static std::string launchpad_device_identifier_get(juce::MidiDeviceInfo& mdi);
		static auto get_available_input_list() -> std::vector<midi_device_info>;
		static auto get_available_output_list() -> std::vector<midi_device_info>;
		static bool launchpad_register(std::shared_ptr<launchpad> lp);
		static bool launchpad_unregister(std::shared_ptr<launchpad> lp);
		static void launchpad_map_update();
		[[nodiscard]] static bool init();
		static void RAC(const std::shared_ptr<launchpad> &lp, bool connect_flag);

	public:
		static std::shared_ptr<launchpad_manager> instance_get();
		static std::shared_ptr<launchpad_manager> instance_get_without_creating();
		static bool launchpad_contains(std::shared_ptr<launchpad> lp);
		static void launchpad_change_callback_register(const std::shared_ptr<std::function<void()>> &callback);
		static bool launchpad_change_callback_unregister(const std::shared_ptr<std::function<void()>> &callback);
		static auto launchpad_list_get() -> std::vector<std::shared_ptr<launchpad>>;
	};

	class launchpad : public core::ID<launchpad>
	{
		using midi_device_info = launchpad_manager::midi_device_info;
		using input_output = launchpad_manager::input_output;
		class midi_callback;
		class LED_global_timer;
		struct VRGB;

		static std::set<launchpad*> launchpad_list;
		static std::unique_ptr<LED_global_timer> LED_timer;
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
		void input_button_callback(const std::uint8_t*, int) const;
	public:
		//TODO: 커스텀 가능한 런치패드 객체로 변경해야 함(input/output을 수동으로 설정할 수 있도록 해야 함)
		template <typename... K>
		static std::shared_ptr<launchpad> create(K&&... args) = delete;

		//launchpad();
		//launchpad(String kind);
		//launchpad(std::shared_ptr<juce::AudioDeviceManager>, const midi_device_info&);
		launchpad(std::shared_ptr<juce::AudioDeviceManager>&);
		launchpad(std::shared_ptr<juce::AudioDeviceManager>&, const midi_device_info&, const midi_device_info&);
		launchpad(const std::shared_ptr<audio_device_manager>&, const midi_device_info*, const midi_device_info*);
		launchpad(const std::shared_ptr<audio_device_manager>&, const input_output&);
		~launchpad();
		
		bool midi_input_set(const midi_device_info&);
		bool midi_output_set(const midi_device_info&);
		void message_send_now(juce::MidiMessage&);
		void hex_send(const juce::String&);
		void hex_send(const juce::uint8*, std::size_t);
		void LED_send();
		void rgbav_grid_calculate();
		void rgb_set(std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t);
		void velocity_set(std::uint8_t, std::uint8_t, std::uint8_t);
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
		[[nodiscard]] std::string input_identifier_get() const;
		[[nodiscard]] std::string output_identifier_get() const;
		[[nodiscard]] std::string input_kind_name_get() const;
		[[nodiscard]] std::string output_kind_name_get() const;
		[[nodiscard]] std::string input_name_get() const;
		[[nodiscard]] std::string output_name_get() const;
	};
	
	std::vector<juce::uint8> hexStringToBytes(const juce::String&);
}