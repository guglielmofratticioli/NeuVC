
#ifndef PluginMainView_h
#define PluginMainView_h

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "VisualizationPanel.h"
#include "NeuVCLNF.h"

class TextBtnLNF : public LookAndFeel_V4
{
public:
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override
    {
        if(isMouseOverButton)
            g.fillAll (Colour::fromRGBA(255,255,255,10)); // Default color
        if (isButtonDown)
            g.fillAll (Colour::fromRGBA(0,0,0,0)); // Change color when button is pressed
        else
            g.fillAll (Colour::fromRGBA(0,0,0,0)); // Default color
    }
    void drawButtonText (Graphics& g, TextButton& button, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
        {
            g.setColour (Colour::fromRGB(177,55,217)); // Change text color here
            g.setFont (LABEL_FONT);
            g.drawFittedText (button.getButtonText(), button.getLocalBounds(), Justification::centred, 1);
        }
};

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
    void modelChoose();
    
    State mPrevState = EmptyAudioAndMidiRegions;
    
    NeuVCAudioProcessor& mProcessor;
    
    NeuVCLNF mLNF;
    TextBtnLNF textBtnLNF;
    
    VisualizationPanel mVisualizationPanel;

    std::unique_ptr<juce::TextButton> mProcessButton;
    std::unique_ptr<juce::TextButton> mModeButton;
    //std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> mProcessButtonAttachment;
    std::unique_ptr<juce::DrawableButton> mRecordButton;
    std::unique_ptr<juce::DrawableButton> mClearButton;
    std::unique_ptr<juce::DrawableButton> mBackButton;
    std::unique_ptr<juce::DrawableButton> mPlayPauseButton;
    std::unique_ptr<juce::DrawableButton> mCenterButton;
    std::unique_ptr<juce::DrawableButton> mSettingsButton;
    std::unique_ptr<juce::TextButton> mModelChooserButton;
    
    std::shared_ptr<juce::FileChooser> mFileChooser;
    
    juce::String rvcPath = "select RVC folder path ->";
    
    std::unique_ptr<ComboBox> mKey; // C, C#, D, D# ...
    std::unique_ptr<ComboBox> mMode; // Major, Minor, Chromatic

    int mNumCallbacksStuckInProcessingState = 0;
};

#endif // PluginMainView_h
