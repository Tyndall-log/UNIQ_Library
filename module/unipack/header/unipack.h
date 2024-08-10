// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include "audio.h"
#include "launchpad.h"
#include "uniq.h"
#include "lightshow.h"

#include <juce_core/juce_core.h>

namespace uniq::unipack
{
	class unipack
	{
		struct keysound_info
		{
			std::string name;
			int repeat = 1;
			uint8_t wormhole = 0;
			bool using_flag = false; //한번이라도 사용되었는지
		};
		struct keyled_info
		{
			const std::string order_name;
			int repeat = 1;
			lightshow::rgbav_sequence_grid rgbav_grid;
		};
		struct keyled_info_compare
		{
			bool operator()(const keyled_info &lhs, const keyled_info &rhs) const
			{
				return lhs.order_name < rhs.order_name;
			}
		};
		enum class autoplay_command_type
		{
			on,
			off,
			touch,
		};
		static auto bom_skip(juce::InputStream &input) -> void;
		// template<autoplay_command_type>
		// static bool on_off_touch(const std::shared_ptr<timeline> &main_timeline, const juce::String &line,
		// 	const juce::StringArray &tokens, const std::vector<keysound_info> keysound_list[8][8][8]);
		static auto find_iter(std::vector<std::tuple<juce::String, int>> &zip_list, const juce::String &path, const juce::String &name)
			-> std::vector<std::tuple<juce::String, int>>::iterator;
		static auto keysound_part(juce::ZipFile &zip, std::vector<std::tuple<juce::String, int>> &zip_list, const juce::String &root_path,
			std::vector<keysound_info> keysound_list[8][8][8])
			-> bool;
		static auto keyled_part(juce::ZipFile &zip, std::vector<std::tuple<juce::String, int>> &zip_list, const juce::String &root_path,
			std::vector<keyled_info> keyled_list[8][8][8])
			-> bool;
	public:

		static auto load(const std::string& zip_path) -> std::shared_ptr<uniq>;
		static bool save(const std::string& path, const std::shared_ptr<uniq>& unipack);
	};
}
