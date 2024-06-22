//
//  audioFileWriter.hpp
//  NeuVCPlugin
//
//  Created by Guglielmo Fratticioli on 09/05/24.
//

#ifndef AudioFileWriter_h
#define AudioFileWriter_h

#include <JuceHeader.h>

#include "BasicPitchConstants.h"
#include "Notes.h"

class MidiFileWriter
{
public:
    bool writeAudioFile(const juce::AudioBuffer<float>& buffer, const juce::File& fileToSave) const;

private:
    static double _BPMToMicrosecondsPerQuarterNote(double inTempoBPM);

    const int mTicksPerQuarterNote = 960;
};

#endif /* audioFileWriter_hpp */
