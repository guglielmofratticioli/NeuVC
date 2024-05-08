

#ifndef VisualizationPanel_h
#define VisualizationPanel_h

#include <JuceHeader.h>

#include "AudioContainer.h"
#include "MidiFileDrag.h"
#include "PluginProcessor.h"
#include "VisualizationPanel.h"

class VisualizationPanel : public juce::Component
{
public:
    explicit VisualizationPanel(NeuVCAudioProcessor* processor);

    void resized() override;

    void paint(Graphics& g) override;

    void clear();

    void setMidiFileDragComponentVisible();

    void mouseEnter(const juce::MouseEvent& event) override;

    void mouseExit(const juce::MouseEvent& event) override;

    Viewport& getAudioMidiViewport();

    AudioContainer& getAudioContainer();


private:
    NeuVCAudioProcessor* mProcessor;
    //Keyboard mKeyboard;
    juce::Viewport mAudioMidiViewport;
    AudioContainer mAudioContainer;
    MidiFileDrag mMidiFileDrag;

    juce::Slider mAudioGainSlider;
    std::unique_ptr<juce::SliderParameterAttachment> mAudioGainSliderAttachment;

    juce::Slider mMidiGainSlider;
    std::unique_ptr<juce::SliderParameterAttachment> mMidiGainSliderAttachment;

    juce::Rectangle<int> mAudioRegionBounds;

};

#endif // VisualizationPanel_h
