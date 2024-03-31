// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "audio.h"


using namespace std;
using namespace juce;

namespace uniq
{
	namespace internal
	{

		shared_ptr<AudioFormatManager> audio_format_manager::get()
		{
			if (!format_manager_)
			{
				format_manager_ = make_shared<AudioFormatManager>();
				format_manager_ -> registerBasicFormats();
			}
			return format_manager_;
		}

#pragma region audio_data

		shared_ptr<audio_data> audio_data::load(const string &path)
		{
			const auto format_manager = audio_format_manager::get();
			const auto reader = format_manager->createReaderFor(File(path));
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

		audio_custom_source::play_data::~play_data()
		{
			for (int i = 0; i < channel_num_; ++i)
			{
				delete[] buffer_[i];
			}
			delete[] buffer_;
		}

		void audio_custom_source::sync_refresh()
		{
			//sync_playing_data_set_ 정리
			while (!sync_playing_data_set_.empty())
			{
				auto it = sync_playing_data_set_.begin();
				if ((*it)->sync_end_position_ < next_sample_position_)
				{
					auto id = (*it)->id_;
					auto map_it = sync_data_map_.find(id);
					if (map_it != sync_data_map_.end())
						if (map_it->second.size() <= 1)
							sync_data_map_.erase(map_it);
						else
							map_it->second.erase(map_it->second.begin());
					else log::error("예상되지 않은 논리 오류: sync_data_map_에 id가 없음");
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
					sync_data_map_[(*it)->id_].insert(*it);
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
				for (const auto& n : p->next_list)
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
				// auto rate = p->sample_rate_ / sample_rate_ * target_speed_;
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


		void audio_custom_source::buffer_ready(const int &target_channel_num, const int &target_sample_num)
		{
			sample_position_t buffer_needed_size = 0;
			const sample_position_t padding = 10;
			for (auto p : playing_data_set_)
			{
				auto rate = p->sample_rate_ / sample_rate_ * target_speed_;
				buffer_needed_size = max(static_cast<sample_position_t>(target_sample_num * rate + padding),
										 buffer_needed_size);
			}
			if (buffer_.getNumChannels() != target_channel_num
				|| buffer_.getNumSamples() < buffer_needed_size)
			{
				buffer_.setSize(target_channel_num, buffer_needed_size);
			}
		}

		void audio_custom_source::chennal_mapping_legacy(shared_ptr<play_data>& pd, AudioBuffer<float> &data_buffer, const int &target_channel_num, const int &target_sample_num)
		{
			if (target_channel_num == data_buffer.getNumChannels())
				return;

			//채널 수 맞추기(현재 1, 2 채널만 지원)
			if (target_channel_num < data_buffer.getNumChannels())
			{
				//target_channel_num: 1, data_buffer.getNumChannels(): 2
				//1채널로 매핑(평균)
				const auto& channelDataLeft = data_buffer.getReadPointer(0);
				const auto& channelDataRight = data_buffer.getReadPointer(1);
				auto* channelData = data_buffer.getWritePointer(0);
				for (int i = 0; i < data_buffer.getNumSamples(); ++i)
				{
					channelData[i] = (channelDataLeft[i] + channelDataRight[i]) * 0.5f;
				}
			}
			else
			{
				//target_channel_num: 2, data_buffer.getNumChannels(): 1
				//2채널로 매핑(복사)
			}
		}

		const float **audio_custom_source::chennal_mapping()
		{
			//padding값 자동 채움
			return nullptr;
		}

		void audio_custom_source::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
		{
			// log::info("audio_custom_source::prepareToPlay");
			unique_lock lock(sl_);
			sample_rate_ = sampleRate;
			next_sample_position_ = 0;
			buffer_.setSize(2, samplesPerBlockExpected);
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
		}

		void audio_custom_source::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill)
		{

		}

		void audio_custom_source::getNextAudioBlockLegacy(const AudioSourceChannelInfo &bufferToFill)
		{
			// log::info("audio_custom_source::getNextAudioBlock");

			unique_lock lock(sl_);
			auto start_sample_position = next_sample_position_;
			next_sample_position_ += bufferToFill.numSamples;
			sync_refresh();

			auto combo_list = play_refresh();

			if (combo_list->empty())
			{
				bufferToFill.clearActiveBufferRegion();
				return;
			}

			const auto output_buffer = bufferToFill.buffer;
			const auto& target_channel_num = output_buffer->getNumChannels();
			const auto& target_sample_num = bufferToFill.numSamples;

			buffer_ready(target_channel_num, target_sample_num);

			for (auto& combo : *combo_list)
			{
				auto output_pos = 0;
				//combo는 연속 데이터
				for (auto& p : combo)
				{
					//채널 수 맞추기
					// shared_lock data_buffer_lock(p->data_->mutex_);
					auto& data_buffer = p->buffer_;
					// chennal_mapping_legacy(p, data_buffer, target_channel_num, target_sample_num);
					const auto ppf = chennal_mapping(); //TODO: 새로운 채널 매핑 함수 구현

					//위치 맞추기
					//출력 오디오 1샘플 초 = 1 / sample_rate_ (s)
					//입력 오디오 1샘플 초 = 1 / (p->sample_rate_ * target_speed_) (s)
					//출력 대 입력 비율 = p->sample_rate_ / sample_rate_ * target_speed_
					auto rate = p->sample_rate_ / sample_rate_ * target_speed_;
					//출력 오디오 시작 위치 = start_sample_position
					//출력 오디오에 대한 입력 오디오 시작 위치(offset) = p->audio_start_position_ + audio_start_position_delay_

					//입력 오디오의 첫번째 샘플 위치
					// = (p->audio_start_position_ - start_sample_position + audio_start_position_delay_) * rate
					//pos (input audio sample)
					double pos = (static_cast<double>(p->audio_start_position_ - start_sample_position) + p->audio_start_position_delay_) * rate;

					//입력 오디오의 마지막 샘플 위치
					//input_pos_max (input audio sample)
					double input_pos_max = (p->audio_end_position_ - start_sample_position) * rate;

					for (auto& next : p->next_list)
					{
						if (!next.exist_)
							continue;
						auto next_pos_s = next.pos_.count() * 1e-6; //us -> s
						double next_pos = next_pos_s * p->sample_rate_ * target_speed_; //s -> sample
						input_pos_max = min(input_pos_max, next_pos);
						break;
					}


					//resampling(audio::interpolator::catmull_rom 사용)
					for (auto c_i = 0; c_i < target_channel_num; ++c_i)
					{
						const auto temp_buffer = buffer_.getWritePointer(c_i);
						const auto pf = ppf[c_i];

						//fade_in, fade_out 처리
						//TODO: 구현
						//fade 적용 후 p->last_gain_ 업데이트(add_audio에서 play_data를 지울 경우 다음으로 전달 해야함)

						//선행 IIR 필터 사용
						//TODO: 구현


						float t = pos;



						for (auto i = 0; i < target_sample_num; ++i)
						{

							const auto pointer = pf + (static_cast<int>(std::floor(t)) - 1);
							const auto value = audio::interpolator::catmull_rom::interpolate(pointer, t);
							temp_buffer[i] = value;
							t += static_cast<float>(rate);
						}

						//후행 IIR 필터 사용
						//TODO: 구현
					}


					// auto rate = p->sample_rate_ / sample_rate_ * target_speed_;
					// auto position = static_cast<sample_position_t>(p->audio_start_position_ * rate);
					// auto end_position = static_cast<sample_position_t>(p->audio_end_position_ * rate);
					// auto& data_buffer = p->data_->buffer_;
					// auto& buffer = buffer_;
					// auto& buffer_channel_data = buffer.getWritePointer(0);
					// auto& data_buffer_channel_data = data_buffer.getReadPointer(0);
					// for (int i = 0; i < target_sample_num; ++i)
					// {
					// 	if (position < end_position)
					// 	{
					// 		buffer_channel_data[i] += data_buffer_channel_data[position];
					// 		++position;
					// 	}
					// 	else
					// 	{
					// 		break;
					// 	}
					// }
				}
			}

			// for (auto p : playing_data_set_)
			// {
			// 	shared_lock data_buffer_lock(p->data_->mutex_);
			// 	auto& data_buffer = p->data_->buffer_;
			//
			// 	chennal_mapping(p, data_buffer, target_channel_num, target_sample_num);
			// }

			// AudioBuffer<float>
			//if

			// AudioBuffer<float>
		}

		auto audio_custom_source::add_audio(const shared_ptr<audio_data>& data, id_t id, add_audio_param param) -> bool
		{
			if (!data)
				return false;

			unique_lock lock(sl_);
			shared_lock data_buffer_lock(data->mutex_);
			auto& data_buffer = data->buffer_;
			if (2 < data_buffer.getNumChannels())
			{
				log::error("2 채널 이하만 지원합니다.(채널 수: " + to_string(data_buffer.getNumChannels()) + ")");
				//TODO: 2.1채널 이상의 오디오 지원
				return false;
			}

			id_t sync_target_id;
			//TODO: 구현
			for (auto target_id : param.sync.target.list)
			{
				// if (sync_data_list_.find(target_id) != sync_data_list_.end())
				// {
				// 	sync_target_id = target_id;
				// 	break;
				// }
			}
			auto pd = make_unique<play_data>();

			return true;
		}
#pragma endregion audio_custom_source
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

	auto audio_source::cue_lower_bound(std::uint64_t cue) -> std::shared_ptr<audio_cue>
	{
		auto it = cue_point_list_.lower_bound(cue);
		if (it == cue_point_list_.end())
			return nullptr;
		return *it;
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
