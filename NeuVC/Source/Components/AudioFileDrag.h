

#ifndef AudioFileDrag_h
#define AudioFileDrag_h

#include <JuceHeader.h>

//#include "MidiFileWriter.h"
#include "PluginProcessor.h"
#include "UIDefines.h"

class AudioFileDrag : public Component
{
public:
    explicit AudioFileDrag(NeuVCAudioProcessor* processor);

    ~AudioFileDrag() override;

    void resized() override;

    void paint(Graphics& g) override;

    void mouseDown(const MouseEvent& event) override;

    void mouseEnter(const MouseEvent& event) override;

    void mouseExit(const MouseEvent& event) override;

private:
    NeuVCAudioProcessor* mProcessor;

    //juce::File mTempDirectory = juce::File::getSpecialLocation(juce::File::tempDirectory);

    //MidiFileWriter mMidiFileWriter;
};

#endif // AudioFileDrag_h
