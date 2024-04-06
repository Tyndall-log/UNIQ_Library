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

	auto fade_low_data::get_gain(const std::chrono::duration<std::int32_t, std::micro> &time) const -> float
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
		//sync_playing_data_set_ 정리
		while (!sync_playing_data_set_.empty())
		{
			auto it = sync_playing_data_set_.begin();
			if ((*it)->sync_end_position_ < next_sample_position_)
			{
				// auto id = (*it)->id_;
				// auto map_it = sync_data_map_.find(id);
				// if (map_it != sync_data_map_.end())
				// 	if (map_it->second.size() <= 1)
				// 		sync_data_map_.erase(map_it);
				// 	else
				// 		map_it->second.erase(map_it->second.begin());
				// else log::error("예상되지 않은 논리 오류: sync_data_map_에 id가 없음");
				sync_playing_data_set_.erase(it);
			}
			else break;
		}

		//sync_waiting_data_set_ 정리
		while (!sync_waiting_data_set_.empty())
		{
			auto it = sync_waiting_data_set_.begin();
			if ((*it)->sync_start_position_ <= next_sample_position_)
			{
				sync_playing_data_set_.insert(*it);
				// sync_data_map_[(*it)->id_].insert(*it);
				sync_waiting_data_set_.erase(it);
			}
			else break;
		}
	}

	auto audio_custom_source::play_refresh()
		-> std::unique_ptr<std::vector<std::vector<std::shared_ptr<play_data>>>>
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
			buffer[i] += padding;
		}

		// const sample_position_t& sample_start = param.sample.start;
		// const sample_position_t& sample_end = param.sample.end == 0xffffffffui32 ? data_buffer.getNumSamples() : param.sample.end;
		const auto& sample_length = sample_end - sample_start;
		if (channel_num_ == data_buffer_channel_num)
		{
			//채널 수가 같으면 그대로 복사
			for (int c_i = 0; c_i < channel_num_; ++c_i)
			{
				const auto& data_buffer_channel_data = data_buffer.getReadPointer(c_i);
				const auto& buffer_c = buffer[c_i];
				for (auto i = sample_start; i < sample_end; ++i)
				{
					// cout << &data_buffer_channel_data[i] << endl;
					buffer_c[i - sample_start] = data_buffer_channel_data[i];
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
				for (auto i = sample_start; i < sample_end; ++i)
				{
					buffer_0[i - sample_start] = (l_p[i] + r_p[i]) * 0.5f;
				}
			}
			else if (channel_num_ == 2 && data_buffer_channel_num == 1)
			{
				//2채널로 매핑(복사)
				const auto& mono_p = data_buffer.getReadPointer(0); // 모노 채널 데이터 포인터
				const auto& buffer_0 = buffer[0];
				const auto& buffer_1 = buffer[1];
				for (auto i = 0; i < sample_length; ++i)
				{
					const float& monoSample = mono_p[i + sample_start];
					buffer_0[i] = monoSample;
					buffer_1[i] = monoSample;
				}
			}
			else
			{
				log::error("채널 매핑 실패(" + to_string(data_buffer_channel_num) + " -> " + to_string(channel_num_) + ")");
				return false;
			}
		}
		md.out.buffer = buffer;
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
		unique_lock lock(sl_);

		auto start_sample_position = next_sample_position_;
		next_sample_position_ += bufferToFill.numSamples;
		sync_refresh();
		auto combo_list = play_refresh();

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
		// vector<shared_ptr<play_data>> playing_data_vector;

		for (auto& combo : *combo_list)
		{
			// auto output_pos = 0;
			//combo는 연속 데이터
			auto p = combo.front();
			auto rate = p->sample_rate_ / sample_rate_ * speed_target_; //출력 대 입력 비율(출력*rate = 입력)
			sample_position_t input_sample_start_pos = p->audio_start_position_ - start_sample_position;
			auto channel_num = min(p->channel_num_, output_channel_num);
			for (auto output_pos = start_sample_position; output_pos < next_sample_position_; ++output_pos)
			{
				const float* const* const& input_buffer = p->buffer_;
				//위치 계산
				double input_sample_pos = (static_cast<double>(output_pos - p->audio_start_position_) + p->audio_start_position_delay_) * rate;
				auto tb_p = output_pos - start_sample_position;

				//TODO: fade_in, fade_out 처리

				//TODO: 선행 IIR 필터 사용

				//리셈플링
				float k;
				const auto t = std::modf(static_cast<float>(input_sample_pos), &k);
				const int k_i = static_cast<int>(std::floor(k)) - 1;
				for (auto c_i = 0; c_i < channel_num; ++c_i)
				{
					const float* const& _b_p = input_buffer[c_i];
					temp_buffer[c_i][tb_p] = audio::interpolator::catmull_rom::interpolate(_b_p + k_i, t);
				}

				//TODO: 후행 IIR 필터 사용

				//다음 오디오 데이터 준비
				while(true)
				{
					input_sample_pos += rate;
					if (input_sample_pos < p->sample_num_) break;
					if (p->next_que.empty()) break;
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
							break;
						}
						log::error("예상되지 않은 논리 오류: next.exist_가 true임에도 waiting_data_map에 해당 id가 없음");
					}
					//다음 오디오 데이터가 준비되어 있지 않으면
					//next 데이터를 p 데이터로 이동
					p->id_ = next.id_;
					p->buffer_ = next.buffer_;
					next.buffer_ = nullptr; //포인트 이동
					p->sample_rate_ = next.sample_rate_;
					p->sample_num_ = next.sample_num_;
					p->channel_num_ = next.channel_num_;
					p->audio_start_position_ = next.audio_start_position_;
					p->audio_start_position_delay_ = next.audio_start_position_delay_;
					rate = p->sample_rate_ / sample_rate_ * speed_target_;
					channel_num = min(p->channel_num_, output_channel_num);
					p->next_que.pop_front();
				}
			}

			//출력
			for (auto c_i = 0; c_i < channel_num; ++c_i)
			{
				const auto& buffer_channel_data = output_buffer->getWritePointer(c_i);
				const auto& temp_buffer_c = temp_buffer[c_i];
				for (auto i = 0; i < output_sample_num; ++i)
				{
					buffer_channel_data[i] += temp_buffer_c[i];
				}
			}
		}
	}

	// void audio_custom_source::getNextAudioBlockLegacy(const AudioSourceChannelInfo &bufferToFill)
	// {
	// 	// log::info("audio_custom_source::getNextAudioBlock");
	//
	// 	unique_lock lock(sl_);
	// 	auto start_sample_position = next_sample_position_;
	// 	next_sample_position_ += bufferToFill.numSamples;
	// 	sync_refresh();
	//
	// 	auto combo_list = play_refresh();
	//
	// 	if (combo_list->empty())
	// 	{
	// 		bufferToFill.clearActiveBufferRegion();
	// 		return;
	// 	}
	//
	// 	const auto output_buffer = bufferToFill.buffer;
	// 	const auto& target_channel_num = output_buffer->getNumChannels();
	// 	const auto& target_sample_num = bufferToFill.numSamples;
	//
	// 	buffer_ready(target_channel_num, target_sample_num);
	//
	// 	for (auto& combo : *combo_list)
	// 	{
	// 		auto output_pos = 0;
	// 		//combo는 연속 데이터
	// 		for (auto& p : combo)
	// 		{
	// 			//채널 수 맞추기
	// 			// shared_lock data_buffer_lock(p->data_->mutex_);
	// 			auto& data_buffer = p->buffer_;
	// 			// chennal_mapping_legacy(p, data_buffer, target_channel_num, target_sample_num);
	// 			const auto ppf = chennal_mapping_copy_with_padding(); //TODO: 새로운 채널 매핑 함수 구현
	//
	// 			//위치 맞추기
	// 			//출력 오디오 1샘플 초 = 1 / sample_rate_ (s)
	// 			//입력 오디오 1샘플 초 = 1 / (p->sample_rate_ * speed_target_) (s)
	// 			//출력 대 입력 비율 = p->sample_rate_ / sample_rate_ * speed_target_
	// 			auto rate = p->sample_rate_ / sample_rate_ * speed_target_;
	// 			//출력 오디오 시작 위치 = start_sample_position
	// 			//출력 오디오에 대한 입력 오디오 시작 위치(offset) = p->audio_start_position_ + audio_start_position_delay_
	//
	// 			//입력 오디오의 첫번째 샘플 위치
	// 			// = (p->audio_start_position_ - start_sample_position + audio_start_position_delay_) * rate
	// 			//pos (input audio sample)
	// 			double pos = (static_cast<double>(p->audio_start_position_ - start_sample_position) + p->audio_start_position_delay_) * rate;
	//
	// 			//입력 오디오의 마지막 샘플 위치
	// 			//input_pos_max (input audio sample)
	// 			double input_pos_max = (p->audio_end_position_ - start_sample_position) * rate;
	//
	// 			for (auto& next : p->next_que)
	// 			{
	// 				if (!next.exist_)
	// 					continue;
	// 				// auto next_pos_s = next.pos_.count() * 1e-6; //us -> s
	// 				// double next_pos = next_pos_s * p->sample_rate_ * speed_target_; //s -> sample
	// 				// input_pos_max = min(input_pos_max, next_pos);
	// 				break;
	// 			}
	//
	//
	// 			//resampling(audio::interpolator::catmull_rom 사용)
	// 			for (auto c_i = 0; c_i < target_channel_num; ++c_i)
	// 			{
	// 				const auto temp_buffer = buffer_.getWritePointer(c_i);
	// 				const auto pf = ppf[c_i];
	//
	// 				//fade_in, fade_out 처리
	// 				//TODO: 구현
	// 				//fade 적용 후 p->last_gain_ 업데이트(add_audio에서 play_data를 지울 경우 다음으로 전달 해야함)
	//
	// 				//선행 IIR 필터 사용
	// 				//TODO: 구현
	//
	//
	// 				float t = pos;
	//
	//
	//
	// 				for (auto i = 0; i < target_sample_num; ++i)
	// 				{
	//
	// 					const auto pointer = pf + (static_cast<int>(std::floor(t)) - 1);
	// 					const auto value = audio::interpolator::catmull_rom::interpolate(pointer, t);
	// 					temp_buffer[i] = value;
	// 					t += static_cast<float>(rate);
	// 				}
	//
	// 				//후행 IIR 필터 사용
	// 				//TODO: 구현
	// 			}
	//
	//
	// 			// auto rate = p->sample_rate_ / sample_rate_ * speed_target_;
	// 			// auto position = static_cast<sample_position_t>(p->audio_start_position_ * rate);
	// 			// auto end_position = static_cast<sample_position_t>(p->audio_end_position_ * rate);
	// 			// auto& data_buffer = p->data_->buffer_;
	// 			// auto& buffer = buffer_;
	// 			// auto& buffer_channel_data = buffer.getWritePointer(0);
	// 			// auto& data_buffer_channel_data = data_buffer.getReadPointer(0);
	// 			// for (int i = 0; i < target_sample_num; ++i)
	// 			// {
	// 			// 	if (position < end_position)
	// 			// 	{
	// 			// 		buffer_channel_data[i] += data_buffer_channel_data[position];
	// 			// 		++position;
	// 			// 	}
	// 			// 	else
	// 			// 	{
	// 			// 		break;
	// 			// 	}
	// 			// }
	// 		}
	// 	}
	//
	// 	// for (auto p : playing_data_set_)
	// 	// {
	// 	// 	shared_lock data_buffer_lock(p->data_->mutex_);
	// 	// 	auto& data_buffer = p->data_->buffer_;
	// 	//
	// 	// 	chennal_mapping(p, data_buffer, target_channel_num, target_sample_num);
	// 	// }
	//
	// 	// AudioBuffer<float>
	// 	//if
	//
	// 	// AudioBuffer<float>
	// }

	auto audio_custom_source::add_audio(const shared_ptr<audio_data>& data, const add_audio_param& param) -> bool
	{
		if (!data)
			return false;

		const uint8_t channel_num_ = this->channel_num_; //atomic_uint8_t를 값 복사
		sample_position_t sample_start = param.sample.start;
		sample_position_t sample_end = param.sample.end;
		const auto sample_length = sample_end - sample_start;
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
					pd->audio_start_position_ = next.audio_start_position_;
					pd->audio_start_position_delay_ = next.audio_start_position_delay_;
				}
				break;
			}
			if (stop) break;
			if (sync_target_play_data) continue;
			sync_target_play_data = _p; //param.sync.target.list 중 가장 첫번째 오디오 선택
		}

		const auto rate = pd->sample_rate_ / sample_rate_ * speed_target_; //출력 대 입력 비율(출력*rate = 입력)
		auto audio_length = static_cast<double>(sample_length) / rate;
		if (0us < param.fade.out.end_time_)
		{
			audio_length += param.fade.out.end_time_.count() * 1e-6 * sample_rate_ / speed_target_;
		}
		if (sync_target_play_data)
		{
			auto time_hint = pd->time_hint_ - sync_target_play_data->time_hint_;
			// if (stop)
			// {
			// 	//위치 동등 검사
			// 	//(pd->audio_start_position_ + pd->audio_start_position_delay_) -
			// 	//(sync_target_play_data->audio_start_position_ + sync_target_play_data->audio_start_position_delay_)
			// }
			// auto standard_position = next_sample_position_;
			// if (time_hint <= 0us)
			// {
			//
			// }
			sample_position_t start_position_i = sync_target_play_data->audio_start_position_ + static_cast<sample_position_t>(ceil(audio_length));
			double start_position_f = audio_length - start_position_i;
			// pd->audio_start_position_ = sync_target_play_data->audio_start_position_
		}
		else
		{
			pd->audio_start_position_ = next_sample_position_;
			pd->audio_start_position_delay_ = 0;
			pd->audio_end_position_ = next_sample_position_ + static_cast<sample_position_t>(ceil(audio_length));
			pd->sync_start_position_ = next_sample_position_ + static_cast<sample_position_t>(param.sync.duration.start.count() * 1e-6 * sample_rate_ / speed_target_);
			pd->sync_end_position_ = next_sample_position_ + static_cast<sample_position_t>(ceil(param.sync.duration.end.count() * 1e-6 * sample_rate_ / speed_target_));


			playing_data_set_.insert(pd);
		}
		return true;
	}
}

namespace uniq
{
#pragma endregion audio_custom_source

	audio_player::audio_player()
	{
		device_manager_ = internal::audio_device_manager::create();
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

	auto audio_player::add_audio(const std::shared_ptr<internal::audio_data> &data, const play_param &param) const -> bool
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

	bool audio_cue::try_set(std::uint64_t cue)
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

	bool audio_source::audio_cue_compare::operator()(const std::shared_ptr<audio_cue> &a, const std::uint64_t &b) const
	{
		return *a < b;
	}

	bool audio_source::audio_cue_compare::operator()(const std::uint64_t &a, const std::shared_ptr<audio_cue> &b) const
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
		const auto data = internal::audio_data::load(file_path);
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

	auto audio_source::play(const std::shared_ptr<audio_player> &player) -> bool
	{
		if (player == nullptr)
		{
			log::error("player is nullptr");
			return false;
		}
		const audio_player::play_param _play_param {
			.id = 0,
			.sample = {0, 48000-1},
		};
		return player->add_audio(data_, _play_param);
	}

	auto audio_source::cue_find_lower_bound(std::uint64_t cue) -> std::shared_ptr<audio_cue>
	{
		auto it = cue_point_list_.lower_bound(cue);
		if (it == cue_point_list_.end())
			return nullptr;
		return *it;
	}

	auto audio_source::segment_create(std::uint64_t cue) -> std::shared_ptr<audio_segment>
	{
		//cue_find_lower_bound
		auto cue_point = cue_find_lower_bound(cue);
		if (cue_point == nullptr)
		{
			log::error("논리 오류: cue_point가 nullptr(논리적으로 반드시 존재해야 함)");
			return nullptr;
		}
	}

	audio_segment::audio_segment(const std::shared_ptr<audio_source> &source,
		const std::shared_ptr<audio_cue> &start_cue, const std::shared_ptr<audio_cue> &end_cue)
		: source_(source), start_cue_(start_cue), end_cue_(end_cue)
	{
	}

	void audio_segment::start_cue_change(const std::shared_ptr<audio_cue> &cue)
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

	void audio_segment::end_cue_change(const std::shared_ptr<audio_cue> &cue)
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
