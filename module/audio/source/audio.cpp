// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "audio.h"

#include <shared_mutex>

using namespace std;
using namespace juce;

namespace uniq
{
	namespace internal
	{
		void audio_custom_source::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
		{
			log::println("audio_custom_source::prepareToPlay");
		}

		void audio_custom_source::releaseResources()
		{
			log::println("audio_custom_source::releaseResources");
		}

		void audio_custom_source::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
		{
			log::println("audio_custom_source::getNextAudioBlock");
			// AudioBuffer<float>
		}
	}

	class audio : public ID<audio>
	{
	private:

	};

	class audio_source : public ID<audio_source>
	{

	};


}
