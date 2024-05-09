
#ifndef PluginMainView_h
#define PluginMainView_h

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "VisualizationPanel.h"
#include "NeuVCLNF.h"

class ModelChooserLNF : public LookAndFeel_V4
{
public:
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override
    {
        if (isButtonDown)
            g.fillAll (Colour::fromRGBA(0,0,0,0)); // Change color when button is pressed
        else if (isMouseOverButton) {
            Rectangle<float> bounds = button.getLocalBounds().toFloat();
            //g.fillAll (Colour::fromRGBA(177,55,217,20)); // Change color when mouse hovers over button
            ColourGradient gradient(Colour::fromRGB(28,28,28), bounds.getX(), bounds.getCentreY(),
                                    Colour::fromRGB(36,12,44), 2*bounds.getRight(), bounds.getCentreY(), false);
                
                g.setGradientFill(gradient);
                g.fillAll();
        }
            
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
    ModelChooserLNF modelChooseLNF;
    
    VisualizationPanel mVisualizationPanel;

    std::unique_ptr<juce::TextButton> mMuteButton;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> mMuteButtonAttachment;
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
