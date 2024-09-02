// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "unipack.h"

using namespace std;
using namespace juce;
using namespace uniq::project;


namespace uniq::unipack
{
	// template<unipack::autoplay_command_type type>
	// bool unipack::on_off_touch(const shared_ptr<timeline> &main_timeline, const String &line,
	// 	const StringArray &tokens, const vector<keysound_info> keysound_list[8][8][8])
	// {
	// 	using act = autoplay_command_type;
	// 	string command;
	// 	if constexpr (type == act::on) command = "on";
	// 	else if constexpr (type == act::off) command = "off";
	// 	else if constexpr (type == act::touch) command = "touch";
	// 	if (tokens.size() < 3)
	// 	{
	// 		log::warn(command + " 명령어에 인자가 부족합니다: \"" + line.toStdString() + "\"");;
	// 		return true;
	// 	}
	// 	uint8_t y = 9 - tokens[1].getIntValue();
	// 	uint8_t x = tokens[2].getIntValue();
	// 	if (y < 0 || 8 < y || x < 0 || 8 < x)
	// 	{
	// 		log::warn(command + " 명령어에 범위를 벗어난 값이 있습니다: \"" + line.toStdString() + "\"");
	// 		return true;
	// 	}
	// 	// log::info("touch: " + to_string(x) + ", " + to_string(y));
	// 	if constexpr (type == act::on || type == act::touch)
	// 	{
	// 		auto group = timeline_group::create();
	// 		group->button_x.set(static_cast<int8_t>(x));
	// 		group->button_y.set(static_cast<int8_t>(y));
	// 		group->press_duration = -1ms; //정의되지 않은 값
	// 		// group->segment = sound_source_map["test"];
	// 		main_timeline->group_add(group);
	// 	}
	// 	else if constexpr (type == act::off)
	// 	{
	// 		// auto group = timeline_group::create();
	// 		// group->button_x.set(static_cast<int8_t>(x));
	// 		// group->button_y.set(static_cast<int8_t>(y));
	// 		// group->press_duration = -1ms; //정의되지 않은 값
	// 		// group->segment = sound_source_map["test"];
	// 		// main_timeline->group_add(group);
	// 	}
	// 	return false;
	// }

	auto unipack::bom_skip(InputStream &input) -> void
	{
		input.setPosition(0);
		if (input.readNextLine().startsWithChar(0xFEFF))
		{
			input.setPosition(3);
		}
		else
		{
			input.setPosition(0);
		}
	}

	auto unipack::find_iter(std::vector<std::tuple<String, int>> &zip_list, const String &path, const String &name)
		-> vector<tuple<String, int>>::iterator
	{
		auto find_index_compare = [](const tuple<String, int>& a, const String& b) {
			auto f = get<0>(a).compareNatural(b, false);
			if (f == 0) f = b.compareNatural(get<0>(a), true);
			return f < 0;
		};
		auto iter1 = std::lower_bound(zip_list.begin(), zip_list.end(),
									  path + name.toLowerCase(), find_index_compare);
		if (iter1 == zip_list.end())
			return zip_list.end();
		// if (get<0>(*iter1).length() != path.length() + name.length() ||
		// 	!get<0>(*iter1).startsWith(path) || !get<0>(*iter1).endsWithIgnoreCase(name))
		// 	return zip_list.end();
		auto iter2 = std::lower_bound(iter1, zip_list.end(),
									  path + name, find_index_compare);
		if (iter2 == zip_list.end())
			return zip_list.end();
		if (get<0>(*iter2).compare(path + name) != 0)
			return std::move(iter1);
		return std::move(iter2);
	}

	auto unipack::keysound_part(ZipFile &zip, vector<tuple<String, int>> &zip_list, const String &root_path,
	                            vector<keysound_info> keysound_list[8][8][8])
		-> bool
	{
		const auto keySound_iter = find_iter(zip_list, root_path, "keySound");
		if (keySound_iter == zip_list.end())
		{
			log::warn("keySound 파일이 존재하지 않습니다.");
			return false;
		}
		unique_ptr<InputStream> keySound_stream(zip.createStreamForEntry(get<1>(*keySound_iter)));
		if (!keySound_stream)
		{
			log::warn("keySound 파일을 읽을 수 없습니다.");
			return false;
		}
		// log::info(keySound_stream->readEntireStreamAsString().replace("\r","").toStdString());
		// vector<keysound_info> keysound_list_ptr[8][8][8];
		bom_skip(*keySound_stream);
		while(!keySound_stream->isExhausted())
		{
			auto line = keySound_stream->readNextLine();
			if (line.String::isEmpty()) continue;
			auto tokens = StringArray::fromTokens(line, false);
			if (tokens.size() < 4)
			{
				log::warn("keySound 파일에 해석할 수 없는 줄이 있습니다: \"" + line.toStdString() + "\"");
				continue;
			}
			uint8_t chain = tokens[0].getIntValue();
			uint8_t row = tokens[1].getIntValue();
			uint8_t col = tokens[2].getIntValue();
			auto sound_name = tokens[3].trim().toStdString();
			if (chain < 1 || 8 < chain || row < 1 || 8 < row || col < 1 || 8 < col)
			{
				log::warn("keySound 파일에 범위를 벗어난 값이 있습니다: \"" + line.toStdString() + "\"");
				continue;
			}
			if (sound_name.empty())
			{
				log::warn("keySound 파일에 sound_name이 비어있습니다: \"" + line.toStdString() + "\"");
				continue;
			}
			// ReSharper disable once CppUseStructuredBinding
			auto &keysound = keysound_list[chain - 1][col - 1][8 - row].emplace_back();
			keysound.name = sound_name;
			if (5 <= tokens.size())
			{
				keysound.repeat = tokens[4].getIntValue();
				if (6 <= tokens.size())
				{
					keysound.wormhole = tokens[5].getIntValue();
				}
			}
		}
		return true;
	}

	auto unipack::keyled_part(ZipFile &zip, vector<tuple<String, int>> &zip_list,
		const String &root_path, vector<keyled_info> keyled_list[8][8][8]) -> bool
	{
		//keyLED 폴더 불러오기
		auto keyLED_iter = find_iter(zip_list, root_path, "keyLED/");
		if (keyLED_iter == zip_list.end())
		{
			log::warn("keyLED 폴더가 존재하지 않습니다.");
			return false;
		}
		// log::info("keyLED 폴더 불러오기");
		const auto keyLED_path = get<0>(*keyLED_iter).upToLastOccurrenceOf("/", true, false);

		set<keyled_info, keyled_info_compare> keyled_info_set[8][8][8];
		for(;keyLED_iter != zip_list.end() && get<0>(*keyLED_iter).startsWith(keyLED_path);++keyLED_iter)
		{
			auto keyLED_name = get<0>(*keyLED_iter).substring(keyLED_path.length());
			if (keyLED_name.isEmpty() || keyLED_name.contains("/")) continue;
			auto keyLED_name_tokens = StringArray::fromTokens(keyLED_name, false);
			if (keyLED_name_tokens.size() < 3)
			{
				log::warn("keyLED 파일 이름이 잘못되었습니다: \"" + keyLED_name.toStdString() + "\"");
				continue;
			}
			const uint8_t chain = keyLED_name_tokens[0].getIntValue();
			const uint8_t row = keyLED_name_tokens[1].getIntValue();
			const uint8_t col = keyLED_name_tokens[2].getIntValue();
			const uint8_t repeat = keyLED_name_tokens.size() < 4 ? 1 : keyLED_name_tokens[3].getIntValue();
			String order_name = keyLED_name_tokens.size() < 5 ? "" : keyLED_name_tokens[4];
			unique_ptr<InputStream> keyLED_stream(zip.createStreamForEntry(get<1>(*keyLED_iter)));
			if (!keyLED_stream)
			{
				log::warn("keyLED 파일을 읽을 수 없습니다.");
				continue;
			}
			// if (row==7 && col==2 && chain==1)
			// 	log::info(keyLED_stream->readEntireStreamAsString().replace("\r","").toStdString());
			// log::info(keyLED_stream->readEntireStreamAsString().replace("\r","").toStdString());
			// keyLED_stream->setPosition(0);
			bom_skip(*keyLED_stream);
			lightshow::rgbav_sequence_grid::sequence_time_t delay{0};
			lightshow::rgbav_sequence_grid rgbav_grid(10, 10);
			while(!keyLED_stream->isExhausted())
			{
				auto line = keyLED_stream->readNextLine();
				if (line.isEmpty()) continue;
				auto led_tokens = StringArray::fromTokens(line, false);
				if (led_tokens.size() < 2)
				{
					log::warn("keyLED 파일에 해석할 수 없는 줄이 있습니다: \"" + line.toStdString() + "\"");
					continue;
				}
				String command = led_tokens[0].trim().toLowerCase();
				if (command == "on" || command == "o")
				{
					if (led_tokens.size() < 4)
					{
						log::warn("on 명령어에 인자가 부족합니다: \"" + line.toStdString() + "\"");
						continue;
					}
					uint8_t led_row;
					uint8_t led_col;
					if (led_tokens[1].containsOnly("0123456789"))
					{
						led_row = led_tokens[1].getIntValue();
						led_col = led_tokens[2].getIntValue();
						if (led_col < 1 || 8 < led_col || led_row < 1 || 8 < led_row)
						{
							log::warn("on 명령어에 범위를 벗어난 값이 있습니다: \"" + line.toStdString() + "\"");
							continue;
						}
					}
					else
					{
						const uint8_t mc = led_tokens[2].getIntValue();
						if (mc < 1 || 32 < mc)
						{
							log::warn("on 명령어에 범위를 벗어난 값이 있습니다: \"" + line.toStdString() + "\"");
							continue;
						}
						//왼쪽 상단부터 시계 방향으로 1~32
						if (mc < 9) { led_col = mc; led_row = 0; }
						else if (mc < 17) { led_col = 9; led_row = mc - 8; }
						else if (mc < 25) { led_col = 25 - mc; led_row = 9; }
						else { led_col = 0; led_row = 33 - mc; }
					}
					// ReSharper disable once CppTooWideScopeInitStatement
					String led_color_hex = led_tokens[3].trim();
					if (led_color_hex == "auto" || led_color_hex == "a")
					{
						if (led_tokens.size() < 5)
						{
							log::warn("on 명령어에 velocity가 없습니다: \"" + line.toStdString() + "\"");
							continue;
						}
						const uint8_t velocity = led_tokens[4].getIntValue();
						rgbav_grid.rgbav_add(led_col, 9 - led_row, delay, lightshow::rgbav{velocity});
					}
					else
					{
						const auto hex = hexStringToBytes(led_color_hex);
						if (hex.size() != 3)
						{
							log::warn("on 명령어에 HEX 색상이 잘못되었습니다: \"" + line.toStdString() + "\"");
							continue;
						}
						const auto led_color = lightshow::rgbav{hex[0], hex[1], hex[2], 0};
						rgbav_grid.rgbav_add(led_col, 9 - led_row, delay, led_color);
					}
				}
				else if (command == "off" || command == "f")
				{
					if (led_tokens.size() < 3)
					{
						log::warn("off 명령어에 인자가 부족합니다: \"" + line.toStdString() + "\"");
						continue;
					}
					uint8_t led_row;
					uint8_t led_col;
					if (led_tokens[1].containsOnly("0123456789"))
					{
						led_row = led_tokens[1].getIntValue();
						led_col = led_tokens[2].getIntValue();
						if (led_col < 1 || 8 < led_col || led_row < 1 || 8 < led_row)
						{
							log::warn("off 명령어에 범위를 벗어난 값이 있습니다: \"" + line.toStdString() + "\"");
							continue;
						}
					}
					else
					{
						const uint8_t mc = led_tokens[2].getIntValue();
						if (mc < 1 || 32 < mc)
						{
							log::warn("off 명령어에 범위를 벗어난 값이 있습니다: \"" + line.toStdString() + "\"");
							continue;
						}
						//왼쪽 상단부터 시계 방향으로 1~32
						if (mc < 9) { led_col = mc; led_row = 0; }
						else if (mc < 17) { led_col = 9; led_row = mc - 8; }
						else if (mc < 25) { led_col = 25 - mc; led_row = 9; }
						else { led_col = 0; led_row = 33 - mc; }
					}
					rgbav_grid.rgbav_add(led_col, 9 - led_row, delay, lightshow::rgbav{});
				}
				else if (command == "delay" || command == "d")
				{
					if (led_tokens.size() < 2)
					{
						log::warn("delay 명령어에 인자가 부족합니다: \"" + line.toStdString() + "\"");
						continue;
					}
					delay += led_tokens[1].getIntValue() * 1ms;
				}
				else
				{
					log::warn("keyLED 파일에 알 수 없는 명령어가 있습니다: \"" + line.toStdString() + "\"");
				}
			}
			keyled_info_set[chain - 1][col - 1][8 - row].emplace(keyled_info{order_name.toStdString(), repeat, rgbav_grid});
		}
		for (int chain = 0; chain < 8; chain++)
		{
			for (int col = 0; col < 8; col++)
			{
				for (int row = 0; row < 8; row++)
				{
					for (auto &keyled : keyled_info_set[chain][col][row])
					{
						keyled_list[chain][col][row].emplace_back(keyled);
					}
				}
			}
		}
		return true;
	}

	auto unipack::load(const string &zip_path) -> std::shared_ptr<project::project>
	{
		const File file(zip_path);
		if (!file.existsAsFile())
		{
			log::error("파일이 존재하지 않습니다.");
			return nullptr;
		}
		ZipFile zip(file);
		if (zip.getNumEntries() <= 0)
		{
			log::error("압축 파일이 아닙니다.");
			return nullptr;
		}

		vector<tuple<String, int>> zip_list;
		zip_list.reserve(zip.getNumEntries());
		for (int i = 0; i < zip.getNumEntries(); i++)
		{
			zip_list.emplace_back(zip.getEntry(i)->filename, i);
		}

		// //test
		// {
		// 	string zl[] = {
		// 		"info",
		// 		"Info",
		// 		"INFO",
		// 		"keySound",
		// 		"zzz",
		// 		"unipack1/info",
		// 		"unipack1/Info",
		// 		"unipack1/INFO",
		// 		"Unipack1/info",
		// 		"Unipack1/Info",
		// 		"Unipack1/INFO",
		// 		"UNIPACK1/info",
		// 		"UNIPACK1/Info",
		// 		"UNIPACK1/INFO",
		// 		"unipack1/keySound",
		// 		"unipack2/info",
		// 		"unipack2/keySound",
		// 		"unipack12/info",
		// 		"unipack12/keySound",
		// 	};
		//
		// 	zip_list.clear();
		// 	for (int i = 0; i < std::size(zl); i++)
		// 	{
		// 		zip_list.emplace_back(zl[i], i);
		// 	}
		// }

		//정렬
		auto compare = [](const tuple<String, int>& a, const tuple<String, int>& b) {
			auto f = get<0>(a).compareNatural(get<0>(b), false);
			if (f == 0) f = get<0>(b).compareNatural(get<0>(a), true);
			if (f == 0) f = get<1>(a) - get<1>(b);
			return f < 0;
		};
		ranges::sort(zip_list, compare);

		// //출력
		// for (auto& [name, index] : zip_list)
		// {
		// 	log::info(name.toStdString()+", "+to_string(index));
		// }

		vector<String> root_path_list;
		for (auto& [name, index] : zip_list)
		{
			if (name.compareIgnoreCase("info") == 0)
			{
				if (root_path_list.empty())
					root_path_list.emplace_back("");
				continue;
			}
			if (name.endsWithIgnoreCase("/info"))
			{
				auto root_path = name.substring(0, name.length() - 4);
				if (!root_path_list.empty() && root_path_list.back().compare(root_path) == 0)
					continue;
				root_path_list.emplace_back(root_path);
			}
		}

		if (root_path_list.empty())
		{
			log::error("info 파일이 존재하지 않습니다.");
			return nullptr;
		}

		// // 출력
		// for (auto& name : root_path_list)
		// {
		// 	log::info("root_path: "s+name.toStdString());
		// }


		// auto find_index = [&](const String& path, const String& name) {
		// 	const auto iter1 = std::lower_bound(zip_list.begin(), zip_list.end(),
		// 		path + name.toLowerCase(), find_index_compare);
		// 	if (iter1 == zip_list.end() || get<0>(*iter1).length() != path.length() + name.length() ||
		// 		!get<0>(*iter1).startsWith(path) || !get<0>(*iter1).endsWithIgnoreCase(name))
		// 		return -1;
		// 	const auto iter2 = std::lower_bound(iter1, zip_list.end(),
		// 		path + name, find_index_compare);
		// 	if (iter2 == zip_list.end())
		// 		return -1;
		// 	if (get<0>(*iter2).compare(path + name) != 0)
		// 		return get<1>(*iter1);
		// 	return get<1>(*iter2);
		// };

		// for (auto& root_path : root_path_list)
		{
			const String root_path = root_path_list[0];

			//info 파일 읽기
			shared_ptr<project::project> uniq;
			{
				const auto info_iter = find_iter(zip_list, root_path, "info");
				if (info_iter == zip_list.end())
				{
					log::error("info 파일이 존재하지 않습니다.");
					return nullptr;
				}
				unique_ptr<InputStream> info_stream(zip.createStreamForEntry(get<1>(*info_iter)));
				if (!info_stream)
				{
					log::error("info 파일을 읽을 수 없습니다.");
					return nullptr;
				}
				// log::info(info_stream->readEntireStreamAsString().replace("\r","").toStdString());
				uniq = project::project::create();
				bom_skip(*info_stream);
				while(!info_stream->isExhausted())
				{
					auto line = info_stream->readNextLine();
					if (line.isEmpty()) continue;
					auto key = line.upToFirstOccurrenceOf("=", false, false).trim();
					auto value = line.fromFirstOccurrenceOf("=", false, false).trim();
					if (key.isEmpty() || value.isEmpty()) continue;
					key = key.toLowerCase();
					if (key == "title")
						uniq->title_set(value.toStdString());
					else if (key == "producername")
						uniq->producer_name_set(value.toStdString());
				}
			}
			// cin.get();

			//sounds 폴더 불러오기
			map<string, shared_ptr<audio_source>> sound_source_map;
			while(uniq)
			{
				auto sound_iter = find_iter(zip_list, root_path, "sounds/");
				if (sound_iter == zip_list.end())
				{
					log::warn("sounds 폴더가 존재하지 않습니다.");
					break;
				}
				uniq->player_get()->device_manager_get()->wait_ready(); //준비될 때까지 대기
				const auto sounds_path = get<0>(*sound_iter).upToLastOccurrenceOf("/", true, false);
				for(;sound_iter != zip_list.end() && get<0>(*sound_iter).startsWith(sounds_path);++sound_iter)
				{
					auto sound_name = get<0>(*sound_iter).substring(sounds_path.length());
					if (sound_name.isEmpty() || sound_name.contains("/")) continue;
					unique_ptr<InputStream> sound_stream(zip.createStreamForEntry(get<1>(*sound_iter)));
					if (!sound_stream)
					{
						log::warn("sounds 파일을 읽을 수 없습니다.");
						continue;
					}
					String sound_ext;
					if (sound_name.contains("."))
						sound_ext = sound_name.fromLastOccurrenceOf(".", false, false);
					else
						sound_ext = "unknown";
					auto sound_path = zip_path + ":" + get<0>(*sound_iter).toStdString();
					// log::info(sound_ext.toStdString() + ", " + sound_path + ", " + sound_name.toStdString());
					// uniq->internal.audio_load(std::move(sound_stream),
					// 	sound_ext.toStdString(), sound_path, sound_name.toStdString());
					auto sound_source = uniq->internal.audio_load(std::move(sound_stream),
						sound_ext.toStdString(), sound_path, sound_name.toStdString());
					if (!sound_source)
					{
						log::warn("\"" + sound_name.toStdString() + "\" 오디오 로드 실패");
						continue;
					}
					sound_source_map.emplace(sound_name.toStdString(), std::move(sound_source));
				}
				break;
			}

			//keySound 파일 읽기
			vector<keysound_info> keysound_grid[8][8][8]; //chain, x, y
			if (uniq)
			{
				keysound_part(zip, zip_list, root_path, keysound_grid);
			}

			//keyLED 폴더 읽기
			vector<keyled_info> keyled_grid[8][8][8]; //chain, x, y
			if(uniq)
			{
				keyled_part(zip, zip_list, root_path, keyled_grid);
			}

			//autoPlay 파일 읽기
			vector<shared_ptr<timeline_page>> timeline_page_list;
			vector<shared_ptr<timeline>> timeline_list;
			while(uniq)
			{
				const auto autoPlay_iter = find_iter(zip_list, root_path, "autoPlay");
				if (autoPlay_iter == zip_list.end())
				{
					log::warn("autoPlay 파일이 존재하지 않습니다.");
					break;
				}
				unique_ptr<InputStream> autoPlay_stream(zip.createStreamForEntry(get<1>(*autoPlay_iter)));
				if (!autoPlay_stream)
				{
					log::warn("autoPlay 파일을 읽을 수 없습니다.");
					break;
				}
				timeline_list.emplace_back(uniq->timeline_create("autoPlay"));
				auto main_timeline = timeline_list.back();
				// log::info(autoPlay_stream->readEntireStreamAsString().replace("\r","").toStdString());
				project::project::cue_point_t cumulative_delay{0};
				auto current_chain_num = 0;
				auto chain_delay = 0us;
				uint16_t press_count[8][8] = {};
				auto on_off_touch = [&]<autoplay_command_type type>(const String &line, const StringArray &tokens)
				{
					using act = autoplay_command_type;
					string command;
					if constexpr (type == act::on) command = "on";
					else if constexpr (type == act::off) command = "off";
					else if constexpr (type == act::touch) command = "touch";

					if (current_chain_num == 0)
					{
						// log::warn("chain 명령어가 먼저 나와야 합니다: \"" + line.toStdString() + "\"");
						current_chain_num = 1;

						auto page = uniq->timeline_page_find_floor(0us);
						page->next_page_set(page, {9, 8});
						timeline_page_list.emplace_back(page);
					}

					if (tokens.size() < 3)
					{
						log::warn(command + " 명령어에 인자가 부족합니다: \"" + line.toStdString() + "\"");;
						return false;
					}
					uint8_t y = 9 - tokens[1].getIntValue();
					uint8_t x = tokens[2].getIntValue();
					if (y < 0 || 8 < y || x < 0 || 8 < x)
					{
						log::warn(command + " 명령어에 범위를 벗어난 값이 있습니다: \"" + line.toStdString() + "\"");
						return false;
					}
					// log::info("touch: " + to_string(x) + ", " + to_string(y));
					chain_delay = 1ms;
					if constexpr (type == act::on || type == act::touch)
					{
						auto group = timeline_group::create();
						group->button_x.set(static_cast<int8_t>(x));
						group->button_y.set(static_cast<int8_t>(y));
						group->press_duration = -1ms; //정의되지 않은 값
						// group->segment = sound_source_map["test"];
						//keyled_list
						const auto &keyled_list = keyled_grid[current_chain_num - 1][x - 1][y - 1];
						if (!keyled_list.empty())
						{
							const auto &keyled = keyled_list[press_count[x - 1][y - 1] % keyled_list.size()];
							// group->rgbav_grid = make_shared<lightshow::rgbav_sequence_grid>(keyled.rgbav_grid);
							group->lightshow_data = lightshow::lightshow_data::create(keyled.rgbav_grid, keyled.repeat);
						}

						//keysound_list
						const auto &keysound_list = keysound_grid[current_chain_num - 1][x - 1][y - 1];
						if (keysound_list.empty())
						{
							log::warn("autoPlay가 빈 버튼을 누릅니다: \"" + line.toStdString() + "\"");
							return false;
						}
						const auto &keysound = keysound_list[press_count[x - 1][y - 1] % keysound_list.size()];
						press_count[x - 1][y - 1]++;
						auto sound_source_iter = sound_source_map.find(keysound.name);
						if (sound_source_iter == sound_source_map.end())
						{
							log::warn("누락된 keysound: \"" + keysound.name + "\"");
							return false;
						}
						group->segment = sound_source_iter->second->segment_create(0);
						group->start_cue = timeline_cue::create(cumulative_delay);
						main_timeline->group_add(group);
						// cout << "d "<<group->segment->cue_length_get() << endl;
					}
					else if constexpr (type == act::off)
					{
						// auto group = timeline_group::create();
						// group->button_x.set(static_cast<int8_t>(x));
						// group->button_y.set(static_cast<int8_t>(y));
						// group->press_duration = -1ms; //정의되지 않은 값
						// group->segment = sound_source_map["test"];
						// main_timeline->group_add(group);
					}
					return true;
				};
				bom_skip(*autoPlay_stream);
				while(!autoPlay_stream->isExhausted())
				{
					auto line = autoPlay_stream->readNextLine();
					if (line.isEmpty()) continue;
					auto tokens = StringArray::fromTokens(line, false);
					if (tokens.size() < 2)
					{
						log::warn("autoPlay 파일에 해석할 수 없는 줄이 있습니다: \"" + line.toStdString() + "\"");
						continue;
					}
					// log::info("line: " + line.toStdString());
					String command = tokens[0].trim().toLowerCase();
					if (command == "chain" || command == "c")
					{
						auto chain_num = tokens[1].getIntValue();
						if (chain_num < 1 || 8 < chain_num)
						{
							log::warn("autoPlay 파일에 범위를 벗어난 chain_num이 있습니다: \"" + line.toStdString() + "\"");
							continue;
						}
						// if (current_chain_num == chain_num) continue;
						current_chain_num = chain_num;
						//press_count 초기화
						fill_n(&press_count[0][0], 8 * 8, 0);
						if (timeline_page_list.empty())
						{
							auto page = uniq->timeline_page_find_floor(0us);
							page->next_page_set(page, {9, 8});
							timeline_page_list.emplace_back(page);
						}
						else
						{
							// auto first_page = timeline_page_list.front();
							auto last_page = timeline_page_list.back();
							auto page = uniq->timeline_page_create(cumulative_delay + chain_delay);
							for (auto y = 1; y <= 8; y++)
							{
								auto tp = last_page->next_page_get({9, y});
								if (9 - chain_num != y)
								{
									if (tp) page->next_page_set(tp, {9, y});
								}
								else
								{
									if (tp)
									{
										auto it = timeline_page_list.rbegin();
										while (it != timeline_page_list.rend())
										{
											if (tp != (*it)->next_page_get({9, y})) break;
											(*it)->next_page_set(page, {9, y});
											if (tp == *it) break;
											++it;
										}
										page->next_page_set(tp, {9, y});
									}
									else
									{
										for (const auto& e : timeline_page_list)
										{
											e->next_page_set(page, {9, y});
										}
										page->next_page_set(page, {9, y});
									}
								}
							}
							timeline_page_list.emplace_back(page);
						}

						// //페이지 연결 상태 표시
						// for (auto& page : timeline_page_list)
						// {
						// 	String s = String::formatted("%05d:", page->ID_get());
						// 	for (auto y = 8; y >= 1; y--)
						// 	{
						// 		auto tp = page->next_page_get({9, y});
						// 		if (tp)
						// 		{
						// 			//id를 4자리에 맞추어 출력
						// 			s += String::formatted(" %05d", tp->ID_get());
						// 		}
						// 	}
						// 	log::info(s.toStdString());
						// }
					}
					else if (command == "on" || command == "o")
					{
						on_off_touch.operator()<autoplay_command_type::on>(line, tokens);
					}
					else if (command == "off" || command == "f")
					{
						on_off_touch.operator()<autoplay_command_type::off>(line, tokens);
					}
					else if (command == "touch" || command == "t")
					{
						on_off_touch.operator()<autoplay_command_type::touch>(line, tokens);
					}
					else if (command == "delay" || command == "d")
					{
						auto delay = static_cast<int>(tokens[1].getDoubleValue() * 1000);
						if (delay < 0)
						{
							log::warn("음수 delay가 있습니다: \"" + line.toStdString() + "\"");
							continue;
						}
						cumulative_delay += 1us * delay;
						chain_delay = 0us;
					}
					else
					{
						log::warn("autoPlay 파일에 알 수 없는 명령어가 있습니다: \"" + line.toStdString() + "\"");
					}

				}
				break;
			}

			return uniq;
		}

		return nullptr;
	}

	bool unipack::save(const std::string &path, const std::shared_ptr<project::project> &unipack)
	{
		//TODO: unipack::save 구현
		return false;
	}
}
