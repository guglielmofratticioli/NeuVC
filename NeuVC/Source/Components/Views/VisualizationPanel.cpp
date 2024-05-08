
#include "VisualizationPanel.h"

VisualizationPanel::VisualizationPanel(NeuVCAudioProcessor* processor)
    : mProcessor(processor)
    , mAudioContainer(processor)
    , mMidiFileDrag(processor)
{
    mAudioMidiViewport.setViewedComponent(&mAudioContainer);
    addAndMakeVisible(mAudioMidiViewport);
    mAudioContainer.setViewportPtr(&mAudioMidiViewport);
    mAudioMidiViewport.setScrollBarsShown(false, true, false, false);
    addChildComponent(mMidiFileDrag);
    /*
    mFileTempo = std::make_unique<juce::TextEditor>();
    mFileTempo->setInputRestrictions(6, "0123456789.");
    mFileTempo->setMultiLine(false, false);
    mFileTempo->setReadOnly(false);

    mFileTempo->setFont(LABEL_FONT);
    mFileTempo->setJustification(juce::Justification::centred);

    mFileTempo->setColour(TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    mFileTempo->setColour(TextEditor::textColourId, BLACK);
    mFileTempo->setColour(TextEditor::outlineColourId, juce::Colours::lightgrey);
    mFileTempo->setColour(TextEditor::focusedOutlineColourId, juce::Colours::grey);
    mFileTempo->onReturnKey = [this]() { mFileTempo->giveAwayKeyboardFocus(); };
    mFileTempo->onEscapeKey = [this]() { mFileTempo->giveAwayKeyboardFocus(); };
    mFileTempo->onFocusLost = [this]() {
        double tempo = jlimit(5.0, 900.0, mFileTempo->getText().getDoubleValue());
        String correct_tempo_str = String(tempo);
        correct_tempo_str = correct_tempo_str.substring(0, jmin(correct_tempo_str.length(), 6));
        mFileTempo->setText(correct_tempo_str);
        mProcessor->setMidiFileTempo(tempo);
    };
    mFileTempo->onTextChange = [this]() {
        double tempo = jlimit(5.0, 900.0, mFileTempo->getText().getDoubleValue());
        mProcessor->setMidiFileTempo(tempo);
    };

    mFileTempo->setText(String(mProcessor->getMidiFileTempo()));
    addChildComponent(*mFileTempo);
     */

    mAudioGainSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    mAudioGainSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxLeft, true, 40, 20);
    mAudioGainSlider.setTextValueSuffix(" dB");
    mAudioGainSlider.setColour(Slider::ColourIds::textBoxTextColourId, BLACK);
    mAudioGainSlider.setColour(Slider::ColourIds::textBoxOutlineColourId, Colours::transparentWhite);
    // To also receive mouseExit callback from this slider
    mAudioGainSlider.addMouseListener(this, true);
    mAudioGainSliderAttachment = std::make_unique<SliderParameterAttachment>(
        *mProcessor->mTree.getParameter("AUDIO_LEVEL_DB"), mAudioGainSlider);

    addChildComponent(mAudioGainSlider);

    mMidiGainSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    mMidiGainSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxLeft, true, 40, 20);
    mMidiGainSlider.setTextValueSuffix(" dB");
    mMidiGainSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colour(177,55,217));
    mMidiGainSlider.setColour(Slider::ColourIds::textBoxOutlineColourId, Colours::transparentWhite);
    // To also receive mouseExit callback from this slider
    mMidiGainSlider.addMouseListener(this, true);

    mMidiGainSliderAttachment =
        std::make_unique<SliderParameterAttachment>(*mProcessor->mTree.getParameter("MIDI_LEVEL_DB"), mMidiGainSlider);

    addChildComponent(mMidiGainSlider);

    // Add this as mouse listener of audio region and pianoroll to control visibility of gain sliders
    mAudioContainer.getAudioRegion()->addMouseListener(this, true);
}

void VisualizationPanel::resized()
{
    
    mAudioMidiViewport.setBounds(0, 0, getWidth() , getHeight());

    mAudioContainer.setBaseWidth(getWidth() );
    mAudioContainer.setBounds(0, 0, getWidth(), getHeight());
    mAudioContainer.changeListenerCallback(mProcessor->getSourceAudioManager()->getAudioThumbnail());

    mMidiFileDrag.setBounds(770, 0, 30, 115);

    mAudioGainSlider.setBounds(getWidth() - 205, 3, 200, 20);
    mMidiGainSlider.setBounds(getWidth() - 205, mAudioContainer.mAudioRegionHeight + 3, 200, 20);

    mAudioRegionBounds = {0, 0, getWidth() , mAudioContainer.mAudioRegionHeight};
    }

void VisualizationPanel::paint(Graphics& g)
{
    /*
    if (mMidiFileDrag.isVisible()) {
        g.setColour(WHITE_TRANSPARENT);
        g.fillRoundedRectangle(
            Rectangle<int>(0, 0, 50, mAudioContainer.mAudioRegionHeight).toFloat(), 4);

        g.setColour(BLACK);
        g.setFont(LABEL_FONT);
        g.drawFittedText(
            "MIDI\nFILE\nTEMPO", Rectangle<int>(0, 0, 50, 55), juce::Justification::centred, 3);
    }
     */
}

void VisualizationPanel::clear()
{
    mAudioContainer.setSize(getWidth() - 50, getHeight());
    mMidiFileDrag.setVisible(false);
}


void VisualizationPanel::setMidiFileDragComponentVisible()
{
    mMidiFileDrag.setVisible(true);
}

void VisualizationPanel::mouseEnter(const MouseEvent& event)
{
    Component::mouseEnter(event);

    if (mProcessor->getState() == PopulatedAudioAndMidiRegions) {
        if (event.originalComponent == mAudioContainer.getAudioRegion()) {
            mAudioGainSlider.setVisible(true);
        }
    }
}

void VisualizationPanel::mouseExit(const MouseEvent& event)
{
    Component::mouseExit(event);

    if (mAudioGainSlider.isVisible()) {
        if (!mAudioRegionBounds.contains(getMouseXYRelative()))
            mAudioGainSlider.setVisible(false);
    }
}

Viewport& VisualizationPanel::getAudioMidiViewport()
{
    return mAudioMidiViewport;
}

AudioContainer& VisualizationPanel::getAudioContainer()
{
    return mAudioContainer;
}
