

#ifndef MidiFileDrag_h
#define MidiFileDrag_h

#include <JuceHeader.h>

//#include "MidiFileWriter.h"
#include "PluginProcessor.h"
#include "UIDefines.h"

class MidiFileDrag : public Component
{
public:
    explicit MidiFileDrag(NeuVCAudioProcessor* processor);

    ~MidiFileDrag() override;

    void resized() override;

    void paint(Graphics& g) override;

    void mouseDown(const MouseEvent& event) override;

    void mouseEnter(const MouseEvent& event) override;

    void mouseExit(const MouseEvent& event) override;

private:
    NeuVCAudioProcessor* mProcessor;

    juce::File mTempDirectory = juce::File::getSpecialLocation(juce::File::tempDirectory);

    //MidiFileWriter mMidiFileWriter;
};

#endif // MidiFileDrag_h
