// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace uniq
{
	namespace internal
	{
		class audio_custom_source : public juce::AudioSource
		{

		public:
			void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
			void releaseResources() override;
			void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
		};
	}

}