// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "audio.h"


using namespace std;
using namespace juce;

namespace uniq::internal
{
	shared_ptr<AudioFormatManager> audio_format_manager::get()
	{
		auto format_manager = format_manager_weak_.lock();
		if (format_manager) return format_manager;
		format_manager = make_shared<AudioFormatManager>();
		format_manager_weak_ = format_manager;
		format_manager->registerBasicFormats();
		return format_manager;
	}

#pragma region audio_data

	shared_ptr<audio_data> audio_data::load(const string &path)
	{
		const auto format_manager = audio_format_manager::get();
		const unique_ptr<AudioFormatReader> reader(format_manager->createReaderFor(File(path)));
		if (reader == nullptr)
		{
			log::println("audio_data::load: reader is nullptr");
			return nullptr;
		}
		auto buffer = AudioBuffer<float>(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
		reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
		auto data = make_shared<audio_data>();
		data->buffer_ = buffer;
		data->sample_rate_ = static_cast<unsigned int>(reader->sampleRate);
		data->extension_ = File(path).getFileExtension().toStdString();
		data->path_ = path;
		data->name_ = File(path).getFileNameWithoutExtension().toStdString();
		return data;
	}

	shared_ptr<audio_data> audio_data::load(unique_ptr<InputStream> input_stream,
		const string &extension, const string &path, const string &name)
	{
		const auto format_manager = audio_format_manager::get();
		const unique_ptr<AudioFormatReader> reader(format_manager->createReaderFor(move(input_stream)));
		if (reader == nullptr)
		{
			log::println("audio_data::load: reader is nullptr");
			return nullptr;
		}
		auto buffer = AudioBuffer<float>(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
		reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
		auto data = make_shared<audio_data>();
		data->buffer_ = buffer;
		data->sample_rate_ = static_cast<unsigned int>(reader->sampleRate);
		data->extension_ = extension;
		data->path_ = path.empty() ? "unknown"s : path;
		data->name_ = name.empty() ? "unknown"s : name;
		return data;
	}

	auto fade_low_data::get_gain(const chrono::duration<int32_t, micro> &time) const -> float
	{
		if (time < start_time_)
		{
			return reverse_ ? 1.f : 0.f;
		}
		if (end_time_ < time)
		{
			return reverse_ ? 0.f : 1.f;
		}
		if (end_time_ == start_time_)
		{
			return 0.5f;
		}
		auto t = static_cast<float>(time.count() - start_time_.count()) / static_cast<float>(end_time_.count() - start_time_.count());
		return fade_function_->get_gain(reverse_ ? 1.f - t : t);
	}

	auto fade_low_data::get_gain(const float t) const -> float
	{
		if (t < 0.f)
		{
			return reverse_ ? 1.f : 0.f;
		}
		if (1.f < t)
		{
			return reverse_ ? 0.f : 1.f;
		}
		return fade_function_->get_gain(reverse_ ? 1.f - t : t);
	}

#pragma endregion audio_data

#pragma region audio_custom_source

	audio_custom_source::play_data::next_s::~next_s()
	{
		if (buffer_)
		{
			for (int i = 0; i < channel_num_; ++i)
			{
				delete[] (buffer_[i] - padding);
			}
			delete[] buffer_;
		}
	}

	audio_custom_source::play_data::~play_data()
	{
		if (buffer_)
		{
			for (int i = 0; i < channel_num_; ++i)
			{
				delete[] (buffer_[i] - padding);
			}
			delete[] buffer_;
		}
	}

	void audio_custom_source::sync_refresh()
	{
		//sync_waiting_data_set_ 정리
		while (!sync_waiting_data_set_.empty())
		{
			auto it = sync_waiting_data_set_.begin();
			if ((*it)->sync_start_position_ <= next_sample_position_)
			{
				sync_playing_data_set_.insert(*it);
				sync_waiting_data_set_.erase(it);
			}
			else break;
		}

		//sync_playing_data_set_ 정리
		while (!sync_playing_data_set_.empty())
		{
			auto it = sync_playing_data_set_.begin();
			if ((*it)->sync_end_position_ < next_sample_position_)
			{
				sync_playing_data_set_.erase(it);
			}
			else break;
		}
	}

	auto audio_custom_source::play_refresh()
		-> unique_ptr<vector<vector<shared_ptr<play_data>>>>
	{
		auto combo_list = make_unique<vector<vector<shared_ptr<play_data>>>>();
		auto waiting_data_map = map<id_t, queue<shared_ptr<play_data>>>();

		//waiting_data_set_ 정리(waiting_data_map으로 이동)
		while (!waiting_data_set_.empty())
		{
			const auto& w_it = waiting_data_set_.begin();
			auto& w = *w_it;
			if (next_sample_position_ < w->audio_start_position_)
				break; //같아도 종료(아직 재생할 수 없음)
			// waiting_data_map[w->id_].push(w); //최적화
			const auto& [it, is_inserted] = waiting_data_map.try_emplace(w->id_);
			it->second.push(w);
			waiting_data_set_.erase(w_it);
		}

		for (auto p_it = playing_data_set_.begin(); p_it != playing_data_set_.end();)
		{
			auto& p = *p_it;
			auto& data_buffer = p->buffer_;
			auto& combo = combo_list->emplace_back();
			combo.push_back(p);
			for (const auto& n : p->next_que)
			{
				auto it = waiting_data_map.find(n.id_);
				if (it == waiting_data_map.end())
					continue;
				auto& q = it->second;
				const auto& w = q.front();
				combo.push_back(w);
				q.pop();
				if (q.empty())
					waiting_data_map.erase(it);
			}

			if (combo.size() == 1)
			{
				++p_it;
				continue;
			}

			p_it = playing_data_set_.erase(p_it);


			// if (p->audio_end_position_ < next_sample_position_) //오디오가 일찍 끝난 경우
			// {
			// 	//다음 오디오 데이터 확인
			// }
			// auto rate = p->sample_rate_ / sample_rate_ * speed_target_;
			// auto position = static_cast<sample_position_t>(p->audio_start_position_ * rate);
			// auto end_position = static_cast<sample_position_t>(p->audio_end_position_ * rate);
			// auto& buffer = buffer_;
		}

		//남은 waiting_data_map 처리
		for (auto &q: waiting_data_map | views::values)
		{
			while (!q.empty())
			{
				auto& w = q.front();
				auto& combo = combo_list->emplace_back();
				combo.push_back(w);
				q.pop();
			}
		}

		// playing_data_set_ 정리
		for (auto combo : *combo_list)
		{
			const auto& last = combo.back();
			if (last->audio_end_position_ <= next_sample_position_) //연속 데이터가 끝난 경우
				continue;
			playing_data_set_.insert(last);
		}

		//NOTE: 여기서 fade_out_ 중첩 처리를 해야 할 수도 있음.

		return move(combo_list);
	}


	// void audio_custom_source::buffer_ready(const int &target_channel_num, const int &target_sample_num)
	// {
	// 	sample_position_t buffer_needed_size = 0;
	// 	for (auto p : playing_data_set_)
	// 	{
	// 		constexpr sample_position_t padding = 10;
	// 		auto rate = p->sample_rate_ / sample_rate_ * speed_target_;
	// 		buffer_needed_size = max(static_cast<sample_position_t>(target_sample_num * rate + padding),
	// 								 buffer_needed_size);
	// 	}
	// 	if (buffer_.getNumChannels() != target_channel_num
	// 		|| buffer_.getNumSamples() < buffer_needed_size)
	// 	{
	// 		buffer_.setSize(target_channel_num, buffer_needed_size);
	// 	}
	// }

	bool audio_custom_source::chennal_mapping_copy_with_padding(mapping_data &md)
	{
		const auto& data = md.in.data;
		const auto& sample_start = md.in.sample_start;
		const auto& sample_end = md.in.sample_end;
		const auto& channel_num_ = md.in.channel_num;

		shared_lock data_buffer_lock(data->mutex_);
		const auto& data_buffer = data->buffer_;
		const auto& data_buffer_channel_num = data_buffer.getNumChannels();
		const auto& data_buffer_sample_num = data_buffer.getNumSamples();

		//오디오 데이터 범위 보정
		if (data_buffer_sample_num <= sample_end)
		{
			md.in.sample_end = data_buffer_sample_num;
		}

		if (sample_end <= sample_start)
		{
			log::error("sample_end <= sample_start");
			return false;
		}

		//padding값 자동 채움
		if (2 < data_buffer_channel_num)
		{
			log::error("2 채널 이하만 지원합니다.(채널 수: " + to_string(data_buffer_sample_num) + ")");
			//TODO: 2.1채널 이상의 오디오 지원
			return false;
		}
		auto buffer = new float*[channel_num_];
		for (int i = 0; i < channel_num_; ++i)
		{
			buffer[i] = new float[sample_end - sample_start + padding * 2];
		}

		//i는 data_buffer의 인덱스
		const auto& i_start = sample_start < padding ? 0 : sample_start - padding;
		const auto& i_end = sample_end + padding < data_buffer_sample_num ? sample_end + padding : data_buffer_sample_num;
		const auto& buffer_offset = padding - sample_start;
		if (channel_num_ == data_buffer_channel_num)
		{
			//채널 수가 같으면 그대로 복사
			for (int c_i = 0; c_i < channel_num_; ++c_i)
			{
				const auto& data_buffer_channel_data = data_buffer.getReadPointer(c_i);
				const auto& buffer_c = buffer[c_i];
				for (auto i = i_start; i < i_end; ++i)
				{
					buffer_c[buffer_offset + i] = data_buffer_channel_data[i];
				}
			}
		}
		else
		{
			//채널 수 맞추기(현재 1, 2 채널만 지원)
			if (channel_num_ == 1 && data_buffer_channel_num == 2)
			{
				//1채널로 매핑(평균)
				const auto& l_p = data_buffer.getReadPointer(0);
				const auto& r_p = data_buffer.getReadPointer(1);
				const auto& buffer_0 = buffer[0];
				for (auto i = i_start; i < i_end; ++i)
				{
					buffer_0[buffer_offset + i] = (l_p[i] + r_p[i]) * 0.5f;
				}
			}
			else if (channel_num_ == 2 && data_buffer_channel_num == 1)
			{
				//2채널로 매핑(복사)
				const auto& mono_p = data_buffer.getReadPointer(0); // 모노 채널 데이터 포인터
				const auto& buffer_0 = buffer[0];
				const auto& buffer_1 = buffer[1];
				for (auto i = i_start; i < i_end; ++i)
				{
					const float& monoSample = mono_p[i];
					buffer_0[buffer_offset + i] = monoSample;
					buffer_1[buffer_offset + i] = monoSample;
				}
			}
			else
			{
				log::error("채널 매핑 실패(" + to_string(data_buffer_channel_num) + " -> " + to_string(channel_num_) + ")");
				return false;
			}
		}

		for (int i = 0; i < channel_num_; ++i)
		{
			buffer[i] += padding;
		}

		md.out.buffer = buffer;

		// { //debugging
		// 	cout << "i_start: "<< i_start << ", i_end: " << i_end << endl;
		// }
		return true;
	}

	audio_custom_source::~audio_custom_source()
	{
		// log::info("audio_custom_source::~audio_custom_source");
		unique_lock lock(sl_);
		next_sample_position_ = 0;
		sync_data_map_.clear();
		sync_playing_data_set_.clear();
		sync_waiting_data_set_.clear();
		playing_data_set_.clear();
		waiting_data_set_.clear();
		buffer_.setSize(0, 0);
		if (low_buffer_)
		{
			for (int i = 0; i < channel_num_; ++i)
			{
				delete[] (low_buffer_[i] - padding);
			}
			delete[] low_buffer_;
		}
	}

	void audio_custom_source::prepareToPlay(const int samplesPerBlockExpected, const double sampleRate)
	{
		log::info("audio_custom_source::prepareToPlay");
		unique_lock lock(sl_);
		sample_rate_ = sampleRate;
		samples_per_block_expected_ = samplesPerBlockExpected;
		next_sample_position_ = 0;
		if (channel_num_ < 1)
			channel_num_ = 2; //기본 2채널
		buffer_.setSize(channel_num_, samplesPerBlockExpected);
		low_buffer_ = new float*[channel_num_];
		for (int i = 0; i < channel_num_; ++i)
		{
			low_buffer_[i] = new float[samplesPerBlockExpected + padding * 2];
			low_buffer_[i] += padding;
		}
		speed_smoothed_.reset(sampleRate, 0.5); //0.5초간 적용
	}

	void audio_custom_source::releaseResources()
	{
		log::info("audio_custom_source::releaseResources");
		unique_lock lock(sl_);
		next_sample_position_ = 0;
		sync_data_map_.clear();
		sync_playing_data_set_.clear();
		sync_waiting_data_set_.clear();
		playing_data_set_.clear();
		waiting_data_set_.clear();
		buffer_.setSize(0, 0);
		if (low_buffer_)
		{
			for (int i = 0; i < channel_num_; ++i)
			{
				delete[] (low_buffer_[i] - padding);
			}
			delete[] low_buffer_;
			low_buffer_ = nullptr;
		}
	}

	void audio_custom_source::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill)
	{
		// log::info("audio_custom_source::getNextAudioBlock");
		// auto delay = 0;
		unique_lock lock(sl_);

		auto audio_sample_start_pos = next_sample_position_;
		next_sample_position_ += bufferToFill.numSamples;
		sync_refresh();
		// auto combo_list = play_refresh();

		auto output_buffer = bufferToFill.buffer;
		const uint8_t output_channel_num = output_buffer->getNumChannels();
		auto output_sample_num = bufferToFill.numSamples;

		if (channel_num_ != output_channel_num)
		{
			log::error("channel_num_ != output_channel_num");
			bufferToFill.clearActiveBufferRegion();
			return;
		}
		float* const* const temp_buffer = low_buffer_;

		auto waiting_data_map = map<id_t, queue<shared_ptr<play_data>>>();
		//waiting_data_set_ 정리(waiting_data_map으로 이동)
		while (!waiting_data_set_.empty())
		{
			const auto& w_it = waiting_data_set_.begin();
			auto& w = *w_it;
			if (next_sample_position_ <= w->audio_start_position_)
				break; //같아도 종료(아직 재생할 수 없음)
			const auto& [it, is_inserted] = waiting_data_map.try_emplace(w->id_);
			it->second.push(w);
			waiting_data_set_.erase(w_it);
		}
		// vector<shared_ptr<play_data>> playing_data_vector;

		auto playing_data_set_it = playing_data_set_.begin();
		// bool test_flag = false;
		while (true)
		{
			shared_ptr<play_data> p;
			if (playing_data_set_it != playing_data_set_.end())
			{
				p = *playing_data_set_it;
				++playing_data_set_it;
			}
			else
			{
				if (waiting_data_map.empty())
					break;
				auto it = waiting_data_map.begin();
				p = it->second.front();
				it->second.pop();
				if (it->second.empty())
					waiting_data_map.erase(it);
				// playing_data_set_.insert(p);
				playing_data_set_.insert(p);
				playing_data_set_it = playing_data_set_.end();
				// test_flag = true;
				// cout <<
			}

			auto rate = p->sample_rate_ / sample_rate_ * speed_target_; //출력 대 입력 비율(출력*rate = 입력)
			auto channel_num = min(p->channel_num_, output_channel_num);
			auto input_sample_end_pos = static_cast<double>(p->sample_num_);
			auto output_start_pos = audio_sample_start_pos;
			if (output_start_pos < p->audio_start_position_)
			{
				output_start_pos = p->audio_start_position_;
			}
			auto output_pos = output_start_pos;
			for (; output_pos <= next_sample_position_; ++output_pos)
			{
				//위치 계산
				const double input_sample_pos = (static_cast<double>(output_pos - p->audio_start_position_)
					+ next_sample_position_delay_ - p->audio_start_position_delay_) * rate;
				if (0xffffffffui32<input_sample_pos) //debugging
					// cout << "p->audio_start_position_: " << p->audio_start_position_ << " " << "output_pos: " << output_pos << endl;
					log::warn("input_sample_pos: " + to_string(input_sample_pos));
				// else if (input_sample_pos < 10 || input_sample_pos > 44090)
				// 	cout << "input_sample_pos: " << input_sample_pos << endl;
				// if (test_flag)
				// {
				// 	cout << output_pos - p->audio_start_position_ << endl;
				// }
				auto stop_flag = false;
				while(true)
				{
					if (input_sample_pos < input_sample_end_pos) break;
					if (p->next_que.empty())
					{
						stop_flag = true;
						playing_data_set_it = playing_data_set_.erase(prev(playing_data_set_it));
						// cout << "stop pos:" << output_pos << endl;
						break;
					}
					auto& next = p->next_que.front();
					if (next.exist_) //다음 오디오 데이터가 준비되어 있으면
					{
						auto wdm_it = waiting_data_map.find(next.id_);
						if (wdm_it != waiting_data_map.end())
						{
							p = wdm_it->second.front();
							//TODO: fade 이동
							wdm_it->second.pop();
							if (wdm_it->second.empty())
								waiting_data_map.erase(wdm_it);
							rate = p->sample_rate_ / sample_rate_ * speed_target_;
							channel_num = min(p->channel_num_, output_channel_num);
							input_sample_end_pos = static_cast<double>(p->sample_num_);
							break;
						}
						log::error("예상되지 않은 논리 오류: next.exist_가 true임에도 waiting_data_map에 해당 id가 없음");
					}
					//다음 오디오 데이터가 준비되어 있지 않으면
					//next 데이터를 p 데이터로 이동
					// p->id_ = next.id_;
					p->buffer_ = next.buffer_;
					next.buffer_ = nullptr; //포인트 이동
					p->sample_rate_ = next.sample_rate_;
					p->sample_num_ = next.sample_num_;
					p->channel_num_ = next.channel_num_;
					p->audio_start_position_ = next.audio_start_position_;
					p->audio_start_position_delay_ = next.audio_start_position_delay_;
					rate = p->sample_rate_ / sample_rate_ * speed_target_;
					channel_num = min(p->channel_num_, output_channel_num);
					input_sample_end_pos = static_cast<double>(p->sample_num_);
					p->next_que.pop_front();
				}

				if (stop_flag) break;

				if (next_sample_position_ <= output_pos) break;

				if (input_sample_pos < 0 || input_sample_end_pos <= input_sample_pos)
				{
					continue;
				}

				const float* const* const& input_buffer = p->buffer_;
				const auto tb_p = output_pos - audio_sample_start_pos;

				//TODO: fade_in, fade_out 처리

				//TODO: 선행 IIR 필터 사용

				//리셈플링
				float k;
				const auto t = modf(static_cast<float>(input_sample_pos), &k);
				const int k_i = static_cast<int>(floor(k)) - 1;
				for (auto c_i = 0; c_i < channel_num; ++c_i)
				{
					const float* const& _b_p = input_buffer[c_i];
					temp_buffer[c_i][tb_p] = audio::interpolator::catmull_rom::interpolate(_b_p + k_i, t);
				}

				//TODO: 후행 IIR 필터 사용
			}

			//출력
			for (auto c_i = 0; c_i < channel_num; ++c_i)
			{
				const auto& buffer_channel_data = output_buffer->getWritePointer(c_i);
				const auto& temp_buffer_c = temp_buffer[c_i];
				const auto& i_start = output_start_pos - audio_sample_start_pos;
				const auto& i_max = output_pos - audio_sample_start_pos;
				for (auto i = i_start; i < i_max; ++i)
				{
					buffer_channel_data[i] += temp_buffer_c[i];
				}
				// if (i_max != samples_per_block_expected_)
				// {
				// 	cout << i_max << endl;
				// }
				// if (temp_buffer_c[i_max-1] == 0)
				// {
				// 	cout << "0" << endl;
				// }
			}
		}
	}

	auto audio_custom_source::add_audio(const shared_ptr<audio_data>& data, const add_audio_param& param) -> bool
	{
		if (!data)
			return false;
		const uint8_t channel_num_ = this->channel_num_; //atomic_uint8_t를 값 복사
		sample_position_t sample_start = param.sample.start;
		sample_position_t sample_end = param.sample.end;
		mapping_data md {
			.in{data, sample_start, sample_end, channel_num_},
			.out{nullptr}
		};
		if(const auto& result = chennal_mapping_copy_with_padding(md); !result)
		{
			log::error("채널 매핑 실패");
			return false;
		}
		float** buffer = md.out.buffer;

		const auto sample_length = sample_end - sample_start;
		const auto pd = make_shared<play_data>();
		pd->id_ = param.id;
		pd->buffer_ = buffer;
		pd->sample_rate_ = data->sample_rate_;
		pd->sample_num_ = sample_length;
		pd->channel_num_ = channel_num_;
		pd->time_hint_ = param.sync.target.time_hint;
		pd->fade_in_ = param.fade.in;
		pd->fade_out_list.push_back(param.fade.out);

		//next_que 추가
		// ReSharper disable once CppUseStructuredBinding
		for(const auto pfo: param.fade.out_list)
		{
			md.in.data = pfo.data;
			md.in.sample_start = pfo.sample.start;
			md.in.sample_end = pfo.sample.end;
			if(!chennal_mapping_copy_with_padding(md))
			{
				log::error("채널 매핑 실패");
				return false;
			}
			pd->next_que.push_back({
				.id_ = pfo.id,
				.buffer_ = md.out.buffer,
				.sample_rate_ = pfo.data->sample_rate_,
				.sample_num_ = md.in.sample_end - md.in.sample_start,
				.channel_num_ = channel_num_,
				.time_hint_ = pfo.sync.time_hint
			});
		}
		ranges::sort(pd->next_que, [](const auto& a, const auto& b) { return a.time_hint_ < b.time_hint_; });

		unique_lock lock(sl_);

		shared_ptr<play_data> sync_target_play_data = nullptr;
		bool stop = false;
		sample_position_t pd_asp = 0;
		double pd_aspd = 0;
		map<id_t, shared_ptr<play_data>> sync_data_map;
		for (auto& spd : sync_playing_data_set_)
		{
			sync_data_map.try_emplace(spd->id_, spd); //동일 id가 있으면 무시(즉, 가장 빨리 끝나는 오디오만 남김)
		}
		for (const id_t& st : param.sync.target.list) // | views::reverse
		{
			auto it = sync_data_map.find(st);
			if (it == sync_data_map.end()) continue;
			const auto& _p = it->second;

			// ReSharper disable once CppUseStructuredBinding
			//fade out에 자신의 id가 있다면 즉시 동기화(최우선 순위)
			for (auto& next : _p->next_que)
			{
				if (next.id_ != pd->id_) continue;
				if(!next.exist_) //동기화 대상에 대기중인(동기화로 연결된) 오디오가 없다면
				{
					next.exist_ = true;
					stop = true;
					sync_target_play_data = _p;
					pd_asp = next.audio_start_position_;
					pd_aspd = next.audio_start_position_delay_;
				}
				break;
			}
			if (stop) break;
			if (sync_target_play_data) continue;
			sync_target_play_data = _p; //param.sync.target.list 중 가장 첫번째 오디오 선택
			// pd_asp = sync_target_play_data->audio_start_position_;
			// pd_aspd = sync_target_play_data->audio_start_position_delay_;
		}

		const auto rate = pd->sample_rate_ / sample_rate_ * speed_target_; //출력 대 입력 비율(출력*rate = 입력)

		if (sync_target_play_data)
		{
			const auto time_hint_diff = pd->time_hint_ - sync_target_play_data->time_hint_;
			const auto time_hint_based_comp_pos = sync_target_play_data->audio_start_position_delay_
				+ static_cast<double>(time_hint_diff.count()) * 1e-6 * sample_rate_ / speed_target_;
			const auto pd_based_comp_pos = pd_aspd
				+ static_cast<double>(pd_asp - sync_target_play_data->audio_start_position_);

			if (continuity_tolerance.count() < abs(time_hint_based_comp_pos - pd_based_comp_pos))
			{
				//오차가 허용 범위를 넘어선 경우, time_hint 기반으로 동기화(modf 사용)
				double time_hint_based_comp_pos_i;
				pd_aspd = modf(time_hint_based_comp_pos, &time_hint_based_comp_pos_i);
				pd_asp = sync_target_play_data->audio_start_position_ + static_cast<sample_position_t>(time_hint_based_comp_pos_i);
				//TO?DO: continuity_tolerance보다 작은 오차 보정
			}
		}
		else
		{
			pd_asp = next_sample_position_;
			pd_aspd = 0;
		}

		auto fade_out_length = 0.0;
		if (0us < param.fade.out.end_time_)
		{
			fade_out_length = param.fade.out.end_time_.count() * 1e-6 * sample_rate_ / speed_target_;
		}
		const auto& end_offset = pd_aspd + static_cast<double>(sample_length) / rate;
		const auto& sync_start_pos_floor = floor(end_offset + param.sync.duration.start.count() * 1e-6 * sample_rate_ / speed_target_);
		const auto& sync_start_pos_ceil = ceil(end_offset + param.sync.duration.end.count() * 1e-6 * sample_rate_ / speed_target_);
		pd->audio_start_position_ = pd_asp;
		pd->audio_start_position_delay_ = pd_aspd;
		pd->audio_end_position_ = pd_asp + static_cast<sample_position_t>(ceil(end_offset + fade_out_length));
		pd->sync_start_position_ = pd_asp + static_cast<sample_position_t>(sync_start_pos_floor);
		pd->sync_end_position_ = pd_asp + static_cast<sample_position_t>(sync_start_pos_ceil);

		if (sync_start_pos_floor < 0 && pd_asp < static_cast<sample_position_t>(-sync_start_pos_floor))
			pd->sync_start_position_ = 0;
		if (sync_start_pos_ceil < 0 && pd_asp < static_cast<sample_position_t>(-sync_start_pos_ceil))
			pd->sync_end_position_ = 0;

		waiting_data_set_.insert(pd);
		if (pd->sync_start_position_ < pd->sync_end_position_)
			if (next_sample_position_ < pd->sync_end_position_)
				if (pd->sync_start_position_ <= next_sample_position_)
					sync_playing_data_set_.insert(pd);
				else
					sync_waiting_data_set_.insert(pd);

		if (sync_target_play_data)
		{
			const auto nsp = next_sample_position_;
			const auto sr = sample_rate_;
			const auto st = speed_target_;
			lock.unlock();
			if (pd_asp < next_sample_position_)
			{
				auto t = round(static_cast<double>(nsp - pd_asp) / sr * st * 1e3);
				log::info(format("동기화 정확도: 느림(+{:.0f}ms)", t));
			}
			else if (pd_asp > next_sample_position_)
			{
				auto t = round(static_cast<double>(pd_asp - nsp) / sr * st * 1e3);
				log::info(format("동기화 정확도: 빠름(-{:.0f}ms)", t));
			}
			else
			{
				log::info("동기화 정확도: 정확(0)");
			}
		}
		return true;
	}
}

namespace uniq
{
#pragma endregion audio_custom_source

	audio_player::audio_player() : audio_player(audio_device_manager::create()) {}

	audio_player::audio_player(const std::shared_ptr<audio_device_manager> &device_manager)
	{
		device_manager_ = device_manager;
		player_ = make_unique<AudioSourcePlayer>();
		custom_source_ = make_unique<internal::audio_custom_source>();
		mt_->call_async([this](){
			player_->setSource(custom_source_.get());
			player_->setGain(0.5f);
			device_manager_->get()->addAudioCallback(player_.get());
		});
	}

	audio_player::~audio_player()
	{
		device_manager_->get()->removeAudioCallback(player_.get());
		player_->setSource(nullptr);
		custom_source_.reset();
		player_.reset();
		device_manager_.reset();
	}

	std::shared_ptr<audio_device_manager> audio_player::device_manager_get() const
	{
		return device_manager_;
	}

	auto audio_player::add_audio(const shared_ptr<internal::audio_data> &data, const play_param &param) const -> bool
	{
		return custom_source_->add_audio(data, param);
	}

	audio_cue::audio_cue(const uint64_t cue) : cue_(cue) {}

	void audio_cue::set(uint64_t cue)
	{
		if (cue_ != cue)
		{
			call_callback(*this, callback_mode::change_before);
			cue_ = cue;
			call_callback(*this, callback_mode::change_after);
		}
	}

	bool audio_cue::try_set(uint64_t cue)
	{
		auto possible = call_check_callback(*this, callback_check_mode::change_possible);
		if (!possible)
			return false;
		set(cue);
		return true;
	}

	uint64_t audio_cue::get() const
	{
		return cue_;
	}

	strong_ordering audio_cue::operator<=>(const audio_cue &other) const
	{
		return cue_ <=> other.cue_;
	}

	strong_ordering audio_cue::operator<=>(const uint64_t &other) const
	{
		return cue_ <=> other;
	}

	audio_cue::operator uint64_t() const
	{
		return cue_;
	}

	bool audio_source::audio_cue_compare::operator()(const shared_ptr<audio_cue> &a,
		const shared_ptr<audio_cue> &b) const
	{
		return *a < *b;
	}

	bool audio_source::audio_cue_compare::operator()(const shared_ptr<audio_cue> &a, const uint64_t &b) const
	{
		return *a < b;
	}

	bool audio_source::audio_cue_compare::operator()(const uint64_t &a, const shared_ptr<audio_cue> &b) const
	{
		return a < *b;
	}

	bool audio_source::audio_segment_compare_start_cue::operator()(const shared_ptr<audio_segment> &a,
																   const shared_ptr<audio_segment> &b) const
	{
		const auto& asc = a->start_cue_;
		const auto& bsc = b->start_cue_;
		if (*asc <=> *bsc == 0) return a < b; //start_cue_가 같으면 두 객체의 포인터를 비교
		return *asc < *bsc;
	}

	bool audio_source::audio_segment_compare_start_cue::operator()(const shared_ptr<audio_segment> &a,
		const uint64_t &b) const
	{
		return *a->start_cue_ < b;
	}

	bool audio_source::audio_segment_compare_start_cue::operator()(const uint64_t &a,
		const shared_ptr<audio_segment> &b) const
	{
		return a < *b->start_cue_;
	}

	audio_source::audio_source()
	{

	}

	auto audio_source::audio_load(const string &file_path) -> shared_ptr<audio_source>
	{
		const auto data = uniq::internal::audio_data::load(file_path);
		if (data == nullptr)
		{
			log::println("audio_source::audio_load: data is nullptr");
			return nullptr;
		}
		auto source = create();
		source->data_ = data;
		source->cue_point_list_.insert(audio_cue::create(0));
		source->cue_point_list_.insert(audio_cue::create(data->buffer_.getNumSamples()));
		return source;
	}

	auto audio_source::internal::audio_load(unique_ptr<InputStream> input_stream,
		const string &extension, const string &path, const string &name)
		-> std::shared_ptr<audio_source>
	{
		auto data = uniq::internal::audio_data::load(move(input_stream), extension, path, name);
		if (data == nullptr)
		{
			log::println("audio_source::audio_load: data is nullptr");
			return nullptr;
		}
		auto source = create();
		source->data_ = data;
		source->cue_point_list_.insert(audio_cue::create(0));
		source->cue_point_list_.insert(audio_cue::create(data->buffer_.getNumSamples()));
		return source;
	}

	auto audio_source::play(const shared_ptr<audio_player> &player) -> bool
	{
		if (player == nullptr)
		{
			log::error("player is nullptr");
			return false;
		}
		const audio_player::play_param _play_param {
			.id = 0,
			// .sample = {0, 48000-1},
		};
		return player->add_audio(data_, _play_param);
	}

	auto audio_source::cue_find_lower_bound(cue_point_t cue) -> shared_ptr<audio_cue>
	{
		auto it = cue_point_list_.lower_bound(cue);
		if (it == cue_point_list_.end())
			return nullptr;
		return *it;
	}

	auto audio_source::segment_create(const cue_point_t cue) -> shared_ptr<audio_segment>
	{
		//데이터 길이 검사
		auto data = data_;
		if (data == nullptr)
		{
			log::error("data is nullptr");
			return nullptr;
		}

		{
			shared_lock lock(data->mutex_);
			if (data->buffer_.getNumSamples() <= cue)
			{
				log::error("cue 위치는 항상 오디오 데이터 범위 내에 있어야 합니다.");
				return nullptr;
			}
		}

		// auto cue_point_it = cue_point_list_.lower_bound(cue);
		// if (prev(cue_point_it) == cue_point_list_.end() || cue_point_it == cue_point_list_.end())
		// {
		// 	log::error("논리 오류: cue_point가 nullptr(논리적으로 반드시 존재해야 함)");
		// 	return nullptr;
		// }
		auto cue_point_it = cue_point_list_.upper_bound(cue);
		if (cue_point_it == cue_point_list_.begin() || cue_point_it == cue_point_list_.end())
		{
			log::error("논리 오류: cue_point가 nullptr(논리적으로 반드시 존재해야 함)");
			return nullptr;
		}
		auto cue_end = *cue_point_it;
		auto cue_start = *--cue_point_it;

		auto self = ID_manager::get_shared_ptr_by_ID<audio_source>(ID_get()).value();
		auto segment = audio_segment::create(self, cue_start, cue_end);
		segment_start_set_.emplace(segment);
		return segment;
	}

	audio_segment::audio_segment(const shared_ptr<audio_source> &source,
	                             const shared_ptr<audio_cue> &start_cue, const shared_ptr<audio_cue> &end_cue)
		: source_(source), start_cue_(start_cue), end_cue_(end_cue)
	{
	}

	auto audio_segment::play(const shared_ptr<audio_player> &player) -> bool
	{
		if (player == nullptr)
		{
			log::error("player is nullptr");
			return false;
		}
		if (source_.expired())
		{
			log::info("원본 오디오 소스가 없습니다.");
			return false;
		}
		auto source = source_.lock();
		const audio_player::play_param _play_param {
			.id = ID_get(),
			.sample = {start_cue_->get(), end_cue_->get()},
			.sync = {
				.target = {
					.time_hint = time_hint,
					.list = vector(sync_target_set_.begin(), sync_target_set_.end())
				},
				.duration = {sync_duration_start_, sync_duration_end_}
			},
		};
		return player->add_audio(source->data_, _play_param);
	}

	auto audio_segment::sync_target_add(id_t id) -> bool
	{
		if (id == 0)
		{
			log::error("id는 0이 될 수 없습니다.");
			return false;
		}
		sync_target_set_.emplace(id);
		return true;
	}

	auto audio_segment::sync_target_add(const shared_ptr<audio_segment> &segment) -> bool
	{
		if (segment == nullptr)
		{
			log::error("segment is nullptr");
			return false;
		}
		if (segment.get() == this)
		{
			log::error("자기 자신을 동기화 대상으로 추가할 수 없습니다.");
			return false;
		}
		sync_target_set_.emplace(segment->ID_get());
		return true;
	}

	auto audio_segment::sync_target_remove(id_t id) -> bool
	{
		if (id == 0)
		{
			log::error("id는 0이 될 수 없습니다.");
			return false;
		}
		auto it = sync_target_set_.find(id);
		if (it == sync_target_set_.end())
		{
			log::error("동기화 대상이 없습니다.");
			return false;
		}
		sync_target_set_.erase(it);
		return true;
	}

	auto audio_segment::sync_target_remove(const shared_ptr<audio_segment> &segment) -> bool
	{
		if (segment == nullptr)
		{
			log::error("segment is nullptr");
			return false;
		}
		if (segment.get() == this)
		{
			log::error("자기 자신을 동기화 대상에서 제거할 수 없습니다.");
			return false;
		}
		auto it = sync_target_set_.find(segment->ID_get());
		if (it == sync_target_set_.end())
		{
			log::error("동기화 대상이 없습니다.");
			return false;
		}
		sync_target_set_.erase(it);
		return true;
	}

	auto audio_segment::time_hint_set(chrono::microseconds time_hint) -> bool
	{
		this->time_hint = time_hint;
		return true;
	}

	auto audio_segment::sync_duration_set(const sync_duration_t &start, const sync_duration_t &end) -> bool
	{
		if (start > end)
		{
			log::error("start > end");
			return false;
		}
		sync_duration_start_ = start;
		sync_duration_end_ = end;
		return true;
	}

	void audio_segment::start_cue_change(const shared_ptr<audio_cue> &cue)
	{
		unique_lock lock(sl_);
		if (start_cue_ == cue)
		{
			log::info("동일한 start_cue_로 변경 시도");
			return;
		}
		call_callback(*this, callback_mode::change_before);
		start_cue_ = cue;
		call_callback(*this, callback_mode::change_after);
	}

	void audio_segment::end_cue_change(const shared_ptr<audio_cue> &cue)
	{
		unique_lock lock(sl_);
		if (end_cue_ == cue)
		{
			log::info("동일한 end_cue_로 변경 시도");
			return;
		}
		call_callback(*this, callback_mode::change_before);
		end_cue_ = cue;
		call_callback(*this, callback_mode::change_after);
	}
}
