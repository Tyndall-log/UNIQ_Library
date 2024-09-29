// SPDX-FileCopyrightText: © 2023 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "launchpad.h"

#include <utility>

using namespace std;
using namespace juce;

namespace uniq::launchpad
{
	// map<launchpad_kind, string> const launchpad_kind_name_map
	map<string, tuple<launchpad_kind, uint8_t, uint8_t>> const launchpad_manager::VPID_map = {
		//1235 -> Focusrite-Novation
		{"1235" "000e", {launchpad_kind::launchpad, 0_uc, 0}},
		{"1235" "0020", {launchpad_kind::launchpad_s, 0_uc, 0}},
		{"1235" "0036", {launchpad_kind::launchpad_mini, 0_uc, 0}},
		{"1235" "0051", {launchpad_kind::launchpad_pro, 0_uc, 0}},
		{"1235" "0069", {launchpad_kind::launchpad_mk2, 0_uc, 1}},
		{"1235" "006a", {launchpad_kind::launchpad_mk2, 0_uc, 2}},
		{"1235" "006b", {launchpad_kind::launchpad_mk2, 0_uc, 3}},
		{"1235" "006v", {launchpad_kind::launchpad_mk2, 0_uc, 4}},
		{"1235" "006d", {launchpad_kind::launchpad_mk2, 0_uc, 5}},
		{"1235" "006e", {launchpad_kind::launchpad_mk2, 0_uc, 6}},
		{"1235" "006f", {launchpad_kind::launchpad_mk2, 0_uc, 7}},
		{"1235" "0070", {launchpad_kind::launchpad_mk2, 0_uc, 8}},
		{"1235" "0071", {launchpad_kind::launchpad_mk2, 0_uc, 9}},
		{"1235" "0072", {launchpad_kind::launchpad_mk2, 0_uc, 10}},
		{"1235" "0073", {launchpad_kind::launchpad_mk2, 0_uc, 11}},
		{"1235" "0074", {launchpad_kind::launchpad_mk2, 0_uc, 12}},
		{"1235" "0075", {launchpad_kind::launchpad_mk2, 0_uc, 13}},
		{"1235" "0076", {launchpad_kind::launchpad_mk2, 0_uc, 14}},
		{"1235" "0077", {launchpad_kind::launchpad_mk2, 0_uc, 15}},
		{"1235" "0078", {launchpad_kind::launchpad_mk2, 0_uc, 16}},
		{"1235" "0103", {launchpad_kind::launchpad_x, 1_uc, 1}},
		{"1235" "0104", {launchpad_kind::launchpad_x, 1_uc, 2}},
		{"1235" "0105", {launchpad_kind::launchpad_x, 1_uc, 3}},
		{"1235" "0106", {launchpad_kind::launchpad_x, 1_uc, 4}},
		{"1235" "0107", {launchpad_kind::launchpad_x, 1_uc, 5}},
		{"1235" "0108", {launchpad_kind::launchpad_x, 1_uc, 6}},
		{"1235" "0109", {launchpad_kind::launchpad_x, 1_uc, 7}},
		{"1235" "010a", {launchpad_kind::launchpad_x, 1_uc, 8}},
		{"1235" "010b", {launchpad_kind::launchpad_x, 1_uc, 9}},
		{"1235" "010c", {launchpad_kind::launchpad_x, 1_uc, 10}},
		{"1235" "010d", {launchpad_kind::launchpad_x, 1_uc, 11}},
		{"1235" "010e", {launchpad_kind::launchpad_x, 1_uc, 12}},
		{"1235" "010f", {launchpad_kind::launchpad_x, 1_uc, 13}},
		{"1235" "0110", {launchpad_kind::launchpad_x, 1_uc, 14}},
		{"1235" "0111", {launchpad_kind::launchpad_x, 1_uc, 15}},
		{"1235" "0112", {launchpad_kind::launchpad_x, 1_uc, 16}},
		{"1235" "0113", {launchpad_kind::launchpad_mini_mk3, 1_uc, 1}},
		{"1235" "0114", {launchpad_kind::launchpad_mini_mk3, 1_uc, 2}},
		{"1235" "0115", {launchpad_kind::launchpad_mini_mk3, 1_uc, 3}},
		{"1235" "0116", {launchpad_kind::launchpad_mini_mk3, 1_uc, 4}},
		{"1235" "0117", {launchpad_kind::launchpad_mini_mk3, 1_uc, 5}},
		{"1235" "0118", {launchpad_kind::launchpad_mini_mk3, 1_uc, 6}},
		{"1235" "0119", {launchpad_kind::launchpad_mini_mk3, 1_uc, 7}},
		{"1235" "011a", {launchpad_kind::launchpad_mini_mk3, 1_uc, 8}},
		{"1235" "011b", {launchpad_kind::launchpad_mini_mk3, 1_uc, 9}},
		{"1235" "011c", {launchpad_kind::launchpad_mini_mk3, 1_uc, 10}},
		{"1235" "011d", {launchpad_kind::launchpad_mini_mk3, 1_uc, 11}},
		{"1235" "011e", {launchpad_kind::launchpad_mini_mk3, 1_uc, 12}},
		{"1235" "011f", {launchpad_kind::launchpad_mini_mk3, 1_uc, 13}},
		{"1235" "0120", {launchpad_kind::launchpad_mini_mk3, 1_uc, 14}},
		{"1235" "0121", {launchpad_kind::launchpad_mini_mk3, 1_uc, 15}},
		{"1235" "0122", {launchpad_kind::launchpad_mini_mk3, 1_uc, 16}},
		{"1235" "0123", {launchpad_kind::launchpad_pro_mk3, 1_uc, 1}},
		{"1235" "0124", {launchpad_kind::launchpad_pro_mk3, 1_uc, 2}},
		{"1235" "0125", {launchpad_kind::launchpad_pro_mk3, 1_uc, 3}},
		{"1235" "0126", {launchpad_kind::launchpad_pro_mk3, 1_uc, 4}},
		{"1235" "0127", {launchpad_kind::launchpad_pro_mk3, 1_uc, 5}},
		{"1235" "0128", {launchpad_kind::launchpad_pro_mk3, 1_uc, 6}},
		{"1235" "0129", {launchpad_kind::launchpad_pro_mk3, 1_uc, 7}},
		{"1235" "012a", {launchpad_kind::launchpad_pro_mk3, 1_uc, 8}},
		{"1235" "012b", {launchpad_kind::launchpad_pro_mk3, 1_uc, 9}},
		{"1235" "012c", {launchpad_kind::launchpad_pro_mk3, 1_uc, 10}},
		{"1235" "012d", {launchpad_kind::launchpad_pro_mk3, 1_uc, 11}},
		{"1235" "012e", {launchpad_kind::launchpad_pro_mk3, 1_uc, 12}},
		{"1235" "012f", {launchpad_kind::launchpad_pro_mk3, 1_uc, 13}},
		{"1235" "0130", {launchpad_kind::launchpad_pro_mk3, 1_uc, 14}},
		{"1235" "0131", {launchpad_kind::launchpad_pro_mk3, 1_uc, 15}},
		{"1235" "0132", {launchpad_kind::launchpad_pro_mk3, 1_uc, 16}},
	};
	map<string, launchpad_kind> const launchpad_manager::android_launchpad_map = {
		{"Focusrite - Novation Launchpad", launchpad_kind::launchpad},
		{"Focusrite - Novation Launchpad S", launchpad_kind::launchpad_s},
		{"Focusrite - Novation Launchpad Mini", launchpad_kind::launchpad_mini},
		{"Focusrite - Novation Launchpad Pro", launchpad_kind::launchpad_pro},
		{"Focusrite - Novation Launchpad MK2", launchpad_kind::launchpad_mk2},
		{"Focusrite - Novation Launchpad X-2", launchpad_kind::launchpad_x},
		{"Focusrite - Novation Launchpad Mini MK3-2", launchpad_kind::launchpad_mini_mk3},
		{"Focusrite - Novation Launchpad Pro MK3-2", launchpad_kind::launchpad_pro_mk3},
	};
	list<tuple<string, launchpad_kind>> const launchpad_manager::apple_launchpad_list = {
		{"Launchpad", launchpad_kind::launchpad},
		{"Launchpad S", launchpad_kind::launchpad_s},
		{"Launchpad Mini", launchpad_kind::launchpad_mini},
		{"Launchpad Pro", launchpad_kind::launchpad_pro},
		{"Launchpad MK2", launchpad_kind::launchpad_mk2},
		{"Launchpad X", launchpad_kind::launchpad_x},
		{"Launchpad Mini MK3", launchpad_kind::launchpad_mini_mk3},
		{"Launchpad Pro MK3", launchpad_kind::launchpad_pro_mk3},
	};

	bool launchpad_manager::launchpad_change_callback_set_compare::operator()(
		const std::weak_ptr<std::function<void()>> &lhs, const std::weak_ptr<std::function<void()>> &rhs) const
	{
		return lhs.owner_before(rhs);
	}

	launchpad_manager::midi_device_info::midi_device_info(const MidiDeviceInfo&& info) : MidiDeviceInfo(info){}

	launchpad_manager::midi_device_info::midi_device_info(const MidiDeviceInfo &&info, const launchpad_kind kind) : MidiDeviceInfo(info)
	{
		this->kind = kind;
	}

	// launchpad_manager::midi_device_info::midi_device_info(const MidiDeviceInfo&& info, const String& name) : MidiDeviceInfo(info)
	// {
	// 	this->kind_name = name.toStdString();
	// }

	launchpad_manager::~launchpad_manager()
	{
		log::info("launchpad_manager 해제 중...");
		launchpad_automatic_map_.clear();
		message_thread_->call_sync([]
		{
			midi_device_list_connection_.reset();
			audio_device_manager_.reset();
		});
		message_thread_.reset();
		log::info("launchpad_manager 해제 완료");
	}

	launchpad_kind launchpad_manager::launchpad_kind_get(MidiDeviceInfo &mdi)
	{
#if JUCE_WINDOWS
		auto& identifier = mdi.identifier;

		if (!identifier.startsWith(R"(\\?\usb#)")) return launchpad_kind::none;

		int pos = identifier.indexOf(8, "vid_");
		const auto& vid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";

		pos = identifier.indexOf(pos + 9, "pid_");
		const auto& pid = 0 <= pos ? identifier.substring(pos + 4, pos + 8).toStdString() : "0000";

		//노베이션 vid인지 확인
		if (vid != "1235") return launchpad_kind::none;

		//허용하는 런치패드인지 확인
		const auto it = VPID_map.find(vid + pid);
		if (it == VPID_map.end())
		{
			log::warn("지원되는 런치패드가 아닙니다. VID: " + vid + ", PID: " + pid);
			return launchpad_kind::unknown; //지원되는 런치패드 아님.
		}

		pos = identifier.indexOf(pos + 9, "\\global");
		const auto global_num = 0 <= pos ? identifier.substring(pos + 1).toStdString() : "0";
		auto num = get<1>(it->second);
		const auto global = 0 < num ? "global-" + String(++num) : "global";

		if (global_num != global) return  launchpad_kind::none; //중복 건너뛰기
		return get<0>(it->second);
#elif JUCE_ANDROID
		const auto& identifier = mdi.identifier;
		const auto& name = mdi.name;
		if (!name.contains("Launchpad")) return launchpad_kind::none;
		const auto it = android_launchpad_map.find(name.toStdString());
		if (it == android_launchpad_map.end())
		{
			log::warn("지원되는 런치패드가 아닙니다. name: " + name.toStdString());
			return launchpad_kind::none; //지원되는 런치패드 아님.
		}
		return it->second;
#elif JUCE_MAC || JUCE_IOS
		const auto& identifier = mdi.identifier;
		const auto& name = mdi.name;
		if (!name.contains("Launchpad")) return launchpad_kind::none;
		if (name.contains("DAW")) return launchpad_kind::none;
		// const auto it = macos_launchpad_map.find(name.toStdString());
		auto it = std::ranges::find_if(apple_launchpad_list | views::reverse,
			[&name](const auto& v)
			{
				return name.contains(get<0>(v));
			});
		if (it == apple_launchpad_list.rend())
		{
			log::warn("지원되는 런치패드가 아닙니다. name: " + name.toStdString());
			return launchpad_kind::none; //지원되는 런치패드 아님.
		}
		return get<1>(*it);
#else
		log::warn("알 수 없는 플랫폼입니다.(런치패드를 인식하지 못할 수 있음)");
		auto& identifier = mdi.identifier;
		auto& name = mdi.name;
		if (name.contains("Launchpad"))
		{
			return launchpad_kind::launchpad;
		}
		return launchpad_kind::none;
#endif
	}

	std::string launchpad_manager::launchpad_device_identifier_get(MidiDeviceInfo &mdi)
	{
#if JUCE_WINDOWS
		String s = mdi.identifier;
		return s.toStdString();
#elif JUCE_ANDROID
		String s = mdi.identifier;
		return s[0] == '-' ? s.substring(1).toStdString() : s.toStdString();
#elif JUCE_MAC || JUCE_IOS
		return mdi.identifier.upToFirstOccurrenceOf(" ", false, false).toStdString();
#else
		return mdi.identifier.toStdString();
#endif
	}

	vector<launchpad_manager::midi_device_info> launchpad_manager::get_available_input_list()
	{
		vector<midi_device_info> devices;
		auto id_list = map<string, uint8_t>();

		const auto mt = message_thread::get();
		auto availableDevices = mt->call_sync([]
		{
			return MidiInput::getAvailableDevices();
		});

		log::info("Available MIDI Input Devices:");
		for (auto& deviceInfo : availableDevices)
		{
			log::info(deviceInfo.name.toStdString() + " " + deviceInfo.identifier.toStdString());
			auto kind = launchpad_kind_get(deviceInfo);
			if (kind == launchpad_kind::none) continue;
			devices.emplace_back(std::move(deviceInfo), kind);
		}

		return devices;
	}

	vector<launchpad_manager::midi_device_info> launchpad_manager::get_available_output_list()
	{
		vector<midi_device_info> devices;
		auto id_list = map<string, uint8_t>();

		const auto mt = message_thread::get();
		auto availableDevices = mt->call_sync([]
		{
			return MidiOutput::getAvailableDevices();
		});

		log::info("Available MIDI Output Devices:");
		for (auto& deviceInfo : availableDevices)
		{
			log::info(deviceInfo.name.toStdString() + " " + deviceInfo.identifier.toStdString());
			auto kind = launchpad_kind_get(deviceInfo);
			if (kind == launchpad_kind::none) continue;
			devices.emplace_back(std::move(deviceInfo), kind);
		}

		return devices;
	}

	void launchpad_manager::launchpad_map_update()
	{
		if (!instance_weak_.expired()) log::info("미디 장치의 변경이 감지되었습니다.");
		if (launchpad_map_update_future_.valid())
		{
			launchpad_map_update_future_.wait();
		}
		launchpad_map_update_future_ = message_thread_->call_async([&]
		{
			auto adm = audio_device_manager_->get_adm();
			auto input_list = get_available_input_list();
			auto output_list = get_available_output_list();
			unordered_map<string, input_output> input_output_map; //string : launchpad_device_identifier_get
			for (auto& input : input_list)
			{
				input_output_map[launchpad_device_identifier_get(input)].input = input;
			}
			for (auto& output : output_list)
			{
				input_output_map[launchpad_device_identifier_get(output)].output = output;
			}
			vector<string> input_output_remove_list;
			vector<string> input_output_add_list;
			for(auto lam_it = launchpad_automatic_map_.begin(); lam_it != launchpad_automatic_map_.end();)
			{
				auto& lam_it_k = lam_it->first;
				auto& lam_it_v = lam_it->second;
				if (!input_output_map.contains(lam_it_k))
				{
					log::info(lam_it_v->input_kind_name_get() + "가 연결이 끊어졌습니다.");
					RAC(lam_it_v, false);
					lam_it = launchpad_automatic_map_.erase(lam_it);
					input_output_remove_list.push_back(lam_it_k);
					continue;
				}
				++lam_it;
			}
			for (auto &[k, v] : input_output_map)
			{
				if (!launchpad_automatic_map_.contains(k))
				{
					const auto& name = launchpad_kind_name_map.at(v.input->kind);
					log::info(name + "가 연결 되었습니다.");
					const auto lp = launchpad::ID::create(adm, v.input.value(), v.output.value());
					RAC(lp, true);
					launchpad_automatic_map_[k] = lp;
					input_output_add_list.push_back(k);
				}
			}
		});
		message_thread_->call_async([&] {
			auto it = launchpad_change_callback_set_.begin();
			while (it != launchpad_change_callback_set_.end())
			{
				if (auto sp = it->lock())
				{
					(*sp)();
					++it;
				}
				else
				{
					it = launchpad_change_callback_set_.erase(it);
				}
			}
		});
	}

	bool launchpad_manager::init()
	{
		message_thread_ = message_thread::get_without_creating();
		if (!message_thread_)
		{
			log::error("launchpad_manager를 초기화 하기 전에 message_thread를 생성해야 합니다.");
			return false;
		}
		audio_device_manager_ = audio_device_manager::get();
		launchpad_map_update();
		message_thread_->call_async([&] {
			midi_device_list_connection_ = MidiDeviceListConnection::make(launchpad_map_update);
			log::info("launchpad_manager 초기화 완료");
		});
		return true;
	}

	void launchpad_manager::RAC(const std::shared_ptr<launchpad> &lp, bool connect_flag)
	{
		using namespace uniq::core::api;
		struct connect_data
		{
			id_t id_;
			bool connect_flag_;
			void* input_name_;
			void* input_kind_name_;
			void* input_identifier_;
			void* output_name_;
			void* output_kind_name_;
			void* output_identifier_;
			~connect_data()
			{
				free(input_name_);
				free(input_kind_name_);
				free(input_identifier_);
				free(output_name_);
				free(output_kind_name_);
				free(output_identifier_);
			}
		};
		const auto cd = new connect_data{
			lp->ID_get(),
			connect_flag,
			strdup(lp->input_name_get().c_str()),
			strdup(lp->input_kind_name_get().c_str()),
			strdup(lp->input_identifier_get().c_str()),
			strdup(lp->output_name_get().c_str()),
			strdup(lp->output_kind_name_get().c_str()),
			strdup(lp->output_identifier_get().c_str())
		};
		callback_manager.RAC(static_cast<id_t>(predefined_ID::launchpad_manager), cd);
	}

	std::shared_ptr<launchpad_manager> launchpad_manager::instance_get()
	{
		auto instance = instance_weak_.lock();
		if (instance) return instance;
		struct make_shared_enabler : launchpad_manager
		{
			make_shared_enabler() = default;
		};
		log::info("launchpad_manager 초기화 중...");
		instance = make_shared<make_shared_enabler>();
		if (!instance->init())
		{
			log::error("launchpad_manager 초기화 실패");
			return nullptr;
		}
		instance_weak_ = instance;
		return instance;
	}

	std::shared_ptr<launchpad_manager> launchpad_manager::instance_get_without_creating()
	{
		return instance_weak_.lock();
	}

	bool launchpad_manager::launchpad_contains(std::shared_ptr<launchpad> lp)
	{
		for(auto& [k, v] : launchpad_automatic_map_)
		{
			if (v == lp) return true;
		}
		for(auto& v : launchpad_set_)
		{
			if (v == lp) return true;
		}
		return false;
	}

	void launchpad_manager::launchpad_change_callback_register(const std::shared_ptr<std::function<void()>> &callback)
	{
		launchpad_change_callback_set_.insert(callback);
	}

	bool launchpad_manager::launchpad_change_callback_unregister(const std::shared_ptr<std::function<void()>> &callback)
	{
		return launchpad_change_callback_set_.erase(callback) != 0;
	}

	auto launchpad_manager::launchpad_list_get() -> std::vector<std::shared_ptr<launchpad>>
	{
		if (launchpad_map_update_future_.valid())
		{
			launchpad_map_update_future_.wait();
		}
		else
		{
			if (instance_weak_.expired())
			{
				log::error("launchpad_manager가 초기화되지 않았습니다.");
				return {};
			}
		}
		auto range = launchpad_automatic_map_ | std::views::values;
		return {range.begin(), range.end()};
	}

	void launchpad::midi_callback::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
	{
		//printHex(message.getRawData(), message.getRawDataSize());
		if (callback_function) callback_function(message.getRawData(), message.getRawDataSize());
		// for (auto& &[k, f] : callback_function_map_)
		// {
		// 	f(const_cast<uint8_t*>(message.getRawData()), message.getRawDataSize());
		// }
	}
	
	void launchpad::midi_callback::printHex(const uint8_t* data, size_t length)
	{
		log::println("MIDI_IN: " + String::toHexString(data, static_cast<int>(length)).toStdString());
	}

	void launchpad::midi_callback::callback_set(function<void(const uint8_t*, int)> callback)
	{
		callback_function = std::move(callback);
	}

	// int launchpad::midi_callback::callback_add(std::function<void(std::uint8_t *, int)> &&callback)
	// {
	// 	callback_function_map_[++callback_function_map_index] = std::move(callback);
	// 	return callback_function_map_index;
	// }
	//
	// bool launchpad::midi_callback::callback_remove(const int callback_id)
	// {
	// 	return callback_function_map_.erase(callback_id) != 0;
	// }

	void launchpad::init()
	{
	
	}
	


	void launchpad::input_button_callback(const uint8_t* const data, const int size) const
	{
		if (size < 3) return;
		auto& note = data[0];
		const auto x = data[1] % 10;
		const auto y = min(max(data[1] / 10, 0), 9);
		const auto v = data[2];
		SpinLock::ScopedLockType lock(mutex);
		auto input_callback_function_map = input_callback_function_map_;
		auto input_button_down_callback_map = input_button_down_callback_map_;
		auto input_button_up_callback_map = input_button_up_callback_map_;
		SpinLock::ScopedUnlockType unlock(mutex);
		for (auto& [k, f] : input_callback_function_map)
		{
			f(data, size);
		}
		if (0x90 <= note && note <= 0x9F || 0xB0 <= note && note <= 0xBF)
		{
			if (0 < v)
			{
				for (auto& [k, f] : input_button_down_callback_map)
					f(x, y, v);
			}
			else
			{
				for (auto& [k, f] : input_button_up_callback_map)
					f(x, y);
			}
		}
		else if (0x80 <= note && note <= 0x8F)
		{
			for (auto& [k, f] : input_button_up_callback_map)
				f(x, y);
		}
	}
	
	launchpad::launchpad(shared_ptr<AudioDeviceManager>& adm)
	{
		deviceManager = adm;
		input_callback = make_unique<midi_callback>();
		// input_callback->callback_set(input_button_callback);
		input_callback->callback_set([this](const uint8_t* data, const int size)
		{
			input_button_callback(data, size);
		});
		{
			SpinLock::ScopedLockType lock(mutex);
			if (launchpad_list.empty())
			{
				LED_timer = make_unique<LED_global_timer>();
				LED_timer->startTimer(interval);
			}
			launchpad_list.insert(this);
			LED_grid_current = vector<vector<VRGB>>(LED_w, vector<VRGB>(LED_h));
			LED_grid_target = vector<vector<VRGB>>(LED_w, vector<VRGB>(LED_h));
			LED_raw_data = make_unique<uint8_t[]>(static_cast<size_t>(LED_w) * LED_h * 5 + 6);
			copy_n("00'20'29'02'0D'03"_hex, 6, LED_raw_data.get()); //기본 명령어 헤더
			automatic_transmission = true;
			immediate_transmission = false;
		}
		lightshow_ = lightshow::lightshow::create();
	}
	
	launchpad::launchpad(shared_ptr<AudioDeviceManager>& adm, const midi_device_info& mdi_input, const midi_device_info& mdi_output)
	: launchpad(adm)
	{
		midi_input_set(mdi_input);
		midi_output_set(mdi_output);
	}

	launchpad::launchpad(const shared_ptr<audio_device_manager>& adm, const midi_device_info* mdi_input, const midi_device_info* mdi_output)
	: launchpad(adm->get_adm())
	{
		if (mdi_input) midi_input_set(*mdi_input);
		if (mdi_output) midi_output_set(*mdi_output);
	}

	launchpad::launchpad(const std::shared_ptr<audio_device_manager>& adm, const input_output& io)
		: launchpad(adm->get_adm())
	{
		if (io.input) midi_input_set(*io.input);
		if (io.output) midi_output_set(*io.output);
	}

	launchpad::~launchpad()
	{
		{
			SpinLock::ScopedLockType lock(mutex);
			launchpad_list.erase(this);
			if (launchpad_list.empty())
			{
				LED_timer->stopTimer();
				LED_timer.reset();
			}
		}
		deviceManager.reset();
	}

	bool launchpad::parent_ID_add(const id_t id)
	{
		if (parent_ID_set_.contains(id)) return false;
		parent_ID_set_.insert(id);
		return true;
	}

	bool launchpad::parent_ID_remove(const id_t id)
	{
		return parent_ID_set_.erase(id) != 0;
	}

	auto launchpad::parent_ID_get() const -> std::set<id_t>
	{
		return parent_ID_set_;
	}

	size_t launchpad::parent_ID_count() const
	{
		return parent_ID_set_.size();
	}

	bool launchpad::midi_input_set(const midi_device_info& mdi_input)
	{
		if (input)
		{
			input->stop();
			input.reset();
		}
		input = MidiInput::openDevice(mdi_input.identifier, input_callback.get());
		if (!input)
		{
			log::error("input is null");
			return false;
		}
		midi_input_kind = mdi_input.kind;
		input->start();
		return true;
		
	}
	
	bool launchpad::midi_output_set(const midi_device_info& mdi_output)
	{
		if (output)
		{
			output.reset();
		}
		output = MidiOutput::openDevice(mdi_output.identifier);
		if (!output)
		{
			log::error("output is null");
			return false;
		}
		midi_output_kind = mdi_output.kind;
		switch (midi_output_kind)
		{
			case launchpad_kind::launchpad:
				copy_n("00'20'29'02'00'00"_hex, 6, LED_raw_data.get()); //TODO: 확인 필요
			break;
			case launchpad_kind::launchpad_s:
				copy_n("00'20'29'02'00'00"_hex, 6, LED_raw_data.get()); //TODO: 확인 필요
			break;
			case launchpad_kind::launchpad_mini:
				copy_n("00'20'29'02'00'00"_hex, 6, LED_raw_data.get()); //TODO: 확인 필요
			break;
			case launchpad_kind::launchpad_pro:
				copy_n("00'20'29'02'10'0B"_hex, 6, LED_raw_data.get());
			break;
			case launchpad_kind::launchpad_mk2:
				copy_n("00'20'29'02'18'0B"_hex, 6, LED_raw_data.get());
			break;
			case launchpad_kind::launchpad_x:
				copy_n("00'20'29'02'0C'03"_hex, 6, LED_raw_data.get());
			break;
			case launchpad_kind::launchpad_mini_mk3:
				copy_n("00'20'29'02'0D'03"_hex, 6, LED_raw_data.get());
			break;
			case launchpad_kind::launchpad_pro_mk3:
				copy_n("00'20'29'02'0E'03"_hex, 6, LED_raw_data.get());
			break;
			default:
				log::error("지원되지 않는 launchpad_kind입니다.");
				return false;
		}
		return true;
	}
	
	void launchpad::message_send_now(MidiMessage& message)
	{
		if (!output)
		{
			log::println("output is null");
			return;
		}
		output->sendMessageNow(message);
	}
	
	void launchpad::hex_send(const String& hex)
	{
		if (!output) return;
		auto k = hexStringToBytes(hex);
		auto m = MidiMessage::createSysExMessage(&k[0], static_cast<int>(k.size()));
		message_send_now(m);
	}
	
	void launchpad::hex_send(const uint8_t* hex, size_t length)
	{
		if (!output) return;
		auto m = MidiMessage::createSysExMessage(hex, static_cast<int>(length));
		message_send_now(m);
	}
	
	void launchpad::LED_send()
	{
		if (!output) return;
		rgbav_grid_calculate();

		auto p = LED_raw_data.get() + 6;
		for (auto x = 0; x < LED_w; x++)
		{
			auto& c_x = LED_grid_current[x];
			auto& t_x = LED_grid_target[x];
			auto& c2_x = rgbav_grid_current[x];
			auto& t2_x = rgbav_grid_target[x];
			// auto& rf_x = reset_flag[x];
			for (auto y = 0; y < LED_h; y++)
			{
				auto& c_xy = c_x[y];
				auto& t_xy = t_x[y];
				auto& c2_xyc = c2_x[y].color;
				auto& c2_xyi = c2_x[y].id;
				auto& t2_xyc = t2_x[y].color;
				auto& t2_xyi = t2_x[y].id;
				// auto& rf_xy = rf_x[y];

				bool t_xy_off_flag = t_xy.v == 0xFF ? t_xy.r == 0 && t_xy.g == 0 && t_xy.b == 0 : t_xy.v == 0;
				int mode; // 0: none, 1: vrgb, 2: rgbav
				if (t_xy_off_flag)
				{
					if (c2_xyc != t2_xyc) mode = 2;
					else
					{
						if (c_xy == t_xy) mode = 0;
						else mode = 1;
					}
				}
				else
				{
					if (c_xy == t_xy) mode = 0;
					else mode = 1;
				}
				// if (rf_xy)
				// {
				// 	// t2_xyc.off_set();
				// 	mode = 2;
				// }

				c_xy = t_xy;
				c2_xyc = t2_xyc;
				c2_xyi = t2_xyi;
				// rf_xy = false;

				if (mode == 0) continue;
				// log::info("x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", mode: " + std::to_string(mode));
				if (mode == 1)
				{
					if (t_xy.v == 0xFF)
					{
						*p++ = 0x03;
						*p++ = static_cast<uint8_t>(y * 10 + x);
						*p++ = t_xy.r;
						*p++ = t_xy.g;
						*p++ = t_xy.b;
					}
					else
					{
						*p++ = 0x00;
						*p++ = static_cast<uint8_t>(y * 10 + x);
						*p++ = t_xy.v;
					}
				}
				else
				{
					if (!t2_xyc.is_velocity())
					{
						*p++ = 0x03;
						*p++ = static_cast<uint8_t>(y * 10 + x);
						*p++ = t2_xyc.r / 2;
						*p++ = t2_xyc.g / 2;
						*p++ = t2_xyc.b / 2;
					}
					else
					{
						*p++ = 0x00;
						*p++ = static_cast<uint8_t>(y * 10 + x);
						*p++ = t2_xyc.velocity_get();
						// if (x==1 && y==1)
						// {
						// 	log::info("velocity: " + std::to_string(t2_xyc.velocity_get()));
						// 	if (t2_xyc.is_off())
						// 		log::info("off");
						// }
					}
				}
			}
		}
		if (p - LED_raw_data.get() <= 6) return;
		// log::info("LED_send");
		//printHex(p - 5, 5);
		auto m = MidiMessage::createSysExMessage(LED_raw_data.get(), static_cast<int>(p - LED_raw_data.get()));
		output->sendMessageNow(m);
	}

	void launchpad::rgbav_grid_calculate()
	{
		auto now = std::chrono::steady_clock::now();
		auto standard_time = lightshow_->standard_time_get();
		using sequence_time_t = lightshow::rgbav_sequence_grid::sequence_time_t;
		auto time = std::chrono::duration_cast<sequence_time_t>(now - standard_time);
		auto grid = lightshow_->rgbav_array_get(time);
		// auto
		for (auto x = 0; x < LED_w; x++)
		{
			auto& t_x = rgbav_grid_target[x];
			for (auto y = 0; y < LED_h; y++)
			{
				auto& t_xy = t_x[y];
				const auto& rgbav_id = grid[x][y];
				t_xy = rgbav_id;
				// if (x==2 && y==1)
				// {
				// 	log::info("velocity1: " + std::to_string(t_xy.color.velocity_get()));
				// 	if (t_xy.color.is_off())
				// 		log::info("off1");
				// }
			}
		}
	}

	void launchpad::rgb_set(const uint8_t x, const uint8_t y, const uint8_t r, const uint8_t g, const uint8_t b)
	{
		if (LED_w < x || LED_h < y)
		{
			log::error("x 또는 y 범위 오류");
			return;
		}
		SpinLock::ScopedLockType lock(mutex);
		LED_grid_target[x][y] = VRGB{ 0xFF,static_cast<uint8_t>(r >> 1),static_cast<uint8_t>(g >> 1),static_cast<uint8_t>(b >> 1) };
		if (immediate_transmission)
		{
			auto p = LED_raw_data.get() + 6;
			*p++ = 0x03;
			*p++ = static_cast<uint8_t>(y * 10 + x);
			*p++ = r;
			*p++ = g;
			*p++ = b;
			auto m = MidiMessage::createSysExMessage(LED_raw_data.get(), static_cast<int>(p - LED_raw_data.get()));
			output->sendMessageNow(m);
			LED_grid_current[x][y] = VRGB{ 0xFF,static_cast<uint8_t>(r >> 1),static_cast<uint8_t>(g >> 1),static_cast<uint8_t>(b >> 1) };
		}
	}
	
	void launchpad::velocity_set(const uint8_t x, const uint8_t y, const uint8_t v)
	{
		if (LED_w < x || LED_h < y)
		{
			log::error("x 또는 y 범위 오류");
			return;
		}
		SpinLock::ScopedLockType lock(mutex);
		LED_grid_target[x][y] = VRGB{ v, 0, 0, 0 };
		if (immediate_transmission)
		{
			auto p = LED_raw_data.get() + 6;
			*p++ = 0x00;
			*p++ = static_cast<uint8_t>(y * 10 + x);
			*p++ = v;
			auto m = MidiMessage::createSysExMessage(LED_raw_data.get(), static_cast<int>(p - LED_raw_data.get()));
			output->sendMessageNow(m);
			LED_grid_current[x][y] = VRGB{ v,0,0,0 };
		}
	}
	
	void launchpad::program_mode_set(const bool flag)
	{
		const uint8* t;
		const uint8* f;
		switch (midi_output_kind)
		{
			case launchpad_kind::launchpad:
			case launchpad_kind::launchpad_s:
			case launchpad_kind::launchpad_mini:
				log::warn("program_mode_set은 지원되지 않는 launchpad_kind입니다.");
			break;
			case launchpad_kind::launchpad_pro:
				t = "00'20'29'02'10'2C'03"_hex;
				f = "00'20'29'02'10'2C'00"_hex;
			break;
			case launchpad_kind::launchpad_mk2:
				log::warn("program_mode_set은 지원되지 않는 launchpad_kind입니다.");
			break;
			case launchpad_kind::launchpad_x:
				t = "00'20'29'02'0C'0E'01"_hex;
				f = "00'20'29'02'0C'0E'00"_hex;
			break;
			case launchpad_kind::launchpad_mini_mk3:
				t = "00'20'29'02'0D'0E'01"_hex;
				f = "00'20'29'02'0D'0E'00"_hex;
			break;
			case launchpad_kind::launchpad_pro_mk3:
				t = "00'20'29'02'0E'0E'01"_hex;
				f = "00'20'29'02'0E'0E'00"_hex;
			break;
			default:
				log::error("지원되지 않는 launchpad_kind입니다.");
				return;
		}
		MidiMessage 메시지 = MidiMessage::createSysExMessage(flag ? t : f, 7);
		message_send_now(메시지);
	}
	
	void launchpad::automatic_transmission_set(const bool flag)
	{
		if (flag == automatic_transmission) return;
		SpinLock::ScopedLockType lock(mutex);
		automatic_transmission = flag;
	}
	
	void launchpad::immediate_transmission_set(const bool flag)
	{
		if (flag == immediate_transmission) return;
		SpinLock::ScopedLockType lock(mutex);
		immediate_transmission = flag;
	}
	
	void launchpad::immediate_transmission_global_timer_set(const int ms)
	{
		LED_timer->startTimer(ms);
	}

	auto launchpad::lightshow_get() -> std::shared_ptr<lightshow::lightshow>
	{
		return lightshow_;
	}

	int launchpad::input_callback_add(std::function<void(const std::uint8_t *, int)> &&callback)
	{
		// input_callback->callback_set(std::move(callback));
		input_callback_function_map_[++input_callback_function_map_index] = std::move(callback);
		return input_callback_function_map_index;
	}

	bool launchpad::input_callback_remove(const int callback_id)
	{
		return input_callback_function_map_.erase(callback_id) != 0;
	}

	auto launchpad::input_button_down_callback_add(
		std::function<void(std::uint8_t x, std::uint8_t y, std::uint8_t velocity)> &&callback) -> int
	{
		input_button_down_callback_map_[++input_callback_function_map_index] = std::move(callback);
		return input_callback_function_map_index;
	}

	bool launchpad::input_button_down_callback_remove(int callback_id)
	{
		return input_button_down_callback_map_.erase(callback_id) != 0;
	}

	auto launchpad::input_button_up_callback_add(std::function<void(std::uint8_t x, std::uint8_t y)> &&callback) -> int
	{
		input_button_up_callback_map_[++input_callback_function_map_index] = std::move(callback);
		return input_callback_function_map_index;
	}

	bool launchpad::input_button_up_callback_remove(int callback_id)
	{
		return input_button_up_callback_map_.erase(callback_id) != 0;
	}
	
	string launchpad::input_identifier_get() const
	{
		return input->getIdentifier().toStdString();
	}
	
	string launchpad::output_identifier_get() const
	{
		return output->getIdentifier().toStdString();
	}

	launchpad_kind launchpad::input_kind_get() const
	{
		return midi_input_kind;
	}

	launchpad_kind launchpad::output_kind_get() const
	{
		return midi_output_kind;
	}

	std::string launchpad::input_kind_name_get() const
	{
		return launchpad_kind_name_map.at(midi_input_kind);
	}

	std::string launchpad::output_kind_name_get() const
	{
		return launchpad_kind_name_map.at(midi_output_kind);
	}

	std::string launchpad::input_name_get() const
	{
		return input->getName().toStdString();
	}

	std::string launchpad::output_name_get() const
	{
		return output->getName().toStdString();
	}

	vector<uint8_t> hexStringToBytes(const String& input)
	{
		vector<uint8_t> result;
		for (int i = 0; i < input.length(); i++)
		{
			if (isxdigit(input[i]))
			{
				// 유효한 16진수 문자인지 체크
				// 두 문자씩 읽어서 uint8_t 타입으로 변환
				uint8_t byte = static_cast<uint8_t>(stoi(input.substring(i, i + 2).toStdString(), nullptr, 16));
				result.push_back(byte);
				i++; // 한 바이트씩 읽기 위해 인덱스를 1 증가
			}
		}
		return result;
	}
	
	unique_ptr<launchpad::LED_global_timer> launchpad::LED_timer = nullptr;
	set<launchpad*> launchpad::launchpad_list = set<launchpad*>();
	SpinLock launchpad::mutex = SpinLock();
	
	void launchpad::LED_global_timer::hiResTimerCallback()
	{
		SpinLock::ScopedLockType lock(mutex);
		for (auto& l : launchpad_list)
		{
			if (!l->automatic_transmission) continue;
			l->LED_send();
		}
		//printf("!");
	}
}