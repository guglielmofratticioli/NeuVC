

#include "Player.h"
#include "PluginProcessor.h"

Player::Player(NeuVCAudioProcessor* inProcessor)
    : mProcessor(inProcessor)
{
}

void Player::prepareToPlay(double inSampleRate, int inSamplesPerBlock)
{
    mSampleRate = inSampleRate;
    mInternalBuffer.setSize(2, inSamplesPerBlock);
}

void Player::processBlock(AudioBuffer<float>& inAudioBuffer)
{
    auto old_audio_gain = mGainSourceAudio;
    auto old_synth_gain = mGainSynth;

    int playhead_index = (int) std::round(mPlayheadTime * mSampleRate);

    _setGains(mProcessor->mTree.getRawParameterValue("AUDIO_LEVEL_DB")->load(),
              mProcessor->mTree.getRawParameterValue("MIDI_LEVEL_DB")->load());

    bool is_playing = mIsPlaying.load();
    mInternalBuffer.clear();

    int num_out_channels = mProcessor->getTotalNumOutputChannels();
    jassert(num_out_channels > 0 && num_out_channels <= 2);
    
    mInternalBuffer.applyGainRamp(0, 0, inAudioBuffer.getNumSamples(), old_synth_gain, mGainSynth);

    for (int ch = 1; ch < num_out_channels; ch++) {
        mInternalBuffer.copyFrom(ch, 0, mInternalBuffer, 0, 0, inAudioBuffer.getNumSamples());
    }

    if (is_playing && mProcessor->getState() == PopulatedAudioAndMidiRegions) {
        const auto& source_buffer = mProcessor->getSourceAudioManager()->getSourceAudio();
        int num_samples = std::min(inAudioBuffer.getNumSamples(), source_buffer.getNumSamples() - playhead_index);

        int num_source_channel = source_buffer.getNumChannels();

        for (int ch = 0; ch < num_out_channels; ch++) {
            int source_channel = std::min(ch, num_source_channel - 1);
            mInternalBuffer.addFromWithRamp(ch,
                                            0,
                                            source_buffer.getReadPointer(source_channel) + playhead_index,
                                            num_samples,
                                            old_audio_gain,
                                            mGainSourceAudio);
        }

        playhead_index += num_samples;

        if (playhead_index >= source_buffer.getNumSamples()) {
            playhead_index = 0;
        }

        mPlayheadTime = (double) playhead_index / mSampleRate;
    }

    for (int ch = 0; ch < num_out_channels; ch++) {
        inAudioBuffer.addFrom(ch, 0, mInternalBuffer, ch, 0, inAudioBuffer.getNumSamples());
    }
}

bool Player::isPlaying() const
{
    return mIsPlaying.load();
}

void Player::setPlayingState(bool inIsPlaying)
{
    mIsPlaying.store(inIsPlaying);
}

void Player::reset()
{
    setPlayingState(false);
    mPlayheadTime = 0;
}

double Player::getPlayheadPositionSeconds() const
{
    return mPlayheadTime;
}

void Player::setPlayheadPositionSeconds(double inNewPosition)
{
   
    if (inNewPosition >= 0 && inNewPosition < mProcessor->getSourceAudioManager()->getAudioSampleDuration()) {
        mPlayheadTime = inNewPosition;
    }
}

void Player::_setGains(float inGainAudioSourceDB, float inGainSynthDB)
{
    mGainSourceAudio = Decibels::decibelsToGain(inGainAudioSourceDB, -36.0f);
}
