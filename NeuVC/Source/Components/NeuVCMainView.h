
#ifndef PluginMainView_h
#define PluginMainView_h

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "VisualizationPanel.h"
#include "NeuVCLNF.h"

class NeuVCMainView
    : public juce::Component
    , public juce::Timer
{
public:
    explicit NeuVCMainView(NeuVCAudioProcessor& processor);

    ~NeuVCMainView();

    void resized() override;

    void paint(juce::Graphics& g) override;

    void timerCallback() override;

    void repaintPianoRoll();
     
    VisualizationPanel* getVisualizationPanel();

private:
    void updateEnablements();

    NeuVCAudioProcessor& mProcessor;
    NeuVCLNF mLNF;

    State mPrevState = EmptyAudioAndMidiRegions;

    VisualizationPanel mVisualizationPanel;

    std::unique_ptr<juce::TextButton> mMuteButton;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> mMuteButtonAttachment;

    std::unique_ptr<juce::DrawableButton> mRecordButton;
    std::unique_ptr<juce::DrawableButton> mClearButton;

    std::unique_ptr<juce::DrawableButton> mBackButton;
    std::unique_ptr<juce::DrawableButton> mPlayPauseButton;
    //std::unique_ptr<juce::DrawableButton> mCenterButton;
    std::unique_ptr<juce::DrawableButton> mSettingsButton;


    std::unique_ptr<ComboBox> mKey; // C, C#, D, D# ...
    std::unique_ptr<ComboBox> mMode; // Major, Minor, Chromatic

    int mNumCallbacksStuckInProcessingState = 0;
};

#endif // PluginMainView_h
