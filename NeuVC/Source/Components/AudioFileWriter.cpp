//
//  audioFileWriter.cpp
//  NeuVCPlugin
//
//  Created by Guglielmo Fratticioli on 09/05/24.
//

#include "AudioFileWriter.h"

bool AudioFileWriter::writeAudioFile(const juce::AudioBuffer<float>& buffer, const juce::File& fileToSave)
{
    juce::FileOutputStream outputStream(fileToSave);

    if (!outputStream.openedOk())
    {
        juce::Logger::writeToLog("Error opening file for writing!");
        return;
    }

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const int numBytesPerSample = 2; // 16-bit audio
    const int numBytesPerChannel = numSamples * numBytesPerSample;

    // Write WAV header
    outputStream.writeInt("RIFF");
    outputStream.writeInt(36 + numChannels * numBytesPerChannel);
    outputStream.writeInt("WAVE");
    outputStream.writeInt("fmt ");
    outputStream.writeInt(16);
    outputStream.writeShort(1); // PCM format
    outputStream.writeShort(numChannels);
    outputStream.writeInt(buffer.getSampleRate());
    outputStream.writeInt(buffer.getSampleRate() * numChannels * numBytesPerSample);
    outputStream.writeShort(numChannels * numBytesPerSample);
    outputStream.writeShort(16); // 16 bits per sample
    outputStream.writeInt("data");
    outputStream.writeInt(numChannels * numSamples * numBytesPerSample);

    // Write audio data
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            int sampleData = static_cast<int>(channelData[sample] * 32767.0f);
            outputStream.writeShort(sampleData);
        }
    }
}
