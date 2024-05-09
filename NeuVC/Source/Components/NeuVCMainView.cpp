
#include "NeuVCMainView.h"

NeuVCMainView::NeuVCMainView(NeuVCAudioProcessor& processor)
    : mProcessor(processor)
    , mVisualizationPanel(&processor)
{
    mRecordButton = std::make_unique<DrawableButton>("RecordButton", DrawableButton::ButtonStyle::ImageRaw);
    mRecordButton->setClickingTogglesState(true);
    mRecordButton->setColour(DrawableButton::ColourIds::backgroundColourId, Colours::transparentBlack);
    mRecordButton->setColour(DrawableButton::ColourIds::backgroundOnColourId, Colours::transparentBlack);

    auto record_off_drawable =
        Drawable::createFromImageData(BinaryData::recordingoff_svg, BinaryData::recordingoff_svgSize);
    auto record_on_drawable =
        Drawable::createFromImageData(BinaryData::recordingon_svg, BinaryData::recordingon_svgSize);

    mRecordButton->setImages(
        record_off_drawable.get(), nullptr, nullptr, nullptr, record_on_drawable.get(), nullptr, nullptr);

    mRecordButton->onClick = [this]() {
        bool is_on = mRecordButton->getToggleState();

        // Recording started
        if (is_on) {
            mProcessor.getSourceAudioManager()->startRecording();
        } else {
            // Recording has ended, set processor state to processing
            mProcessor.getSourceAudioManager()->stopRecording();
        }

        updateEnablements();
    };

    mRecordButton->setToggleState(mProcessor.getState() == Recording, juce::NotificationType::dontSendNotification);

    addAndMakeVisible(*mRecordButton);

    mClearButton = std::make_unique<DrawableButton>("ClearButton", DrawableButton::ButtonStyle::ImageRaw);
    mClearButton->setClickingTogglesState(false);
    mClearButton->setColour(DrawableButton::ColourIds::backgroundColourId, Colours::transparentBlack);
    mClearButton->setColour(DrawableButton::ColourIds::backgroundOnColourId, Colours::transparentBlack);

    auto bin_drawable = Drawable::createFromImageData(BinaryData::deleteicon_svg, BinaryData::deleteicon_svgSize);
    mClearButton->setImages(bin_drawable.get());

    mClearButton->onClick = [this]() {
        mProcessor.clear();
        mVisualizationPanel.clear();
        updateEnablements();
    };
    addAndMakeVisible(*mClearButton);

    mBackButton = std::make_unique<DrawableButton>("BackButton", DrawableButton::ButtonStyle::ImageRaw);
    mBackButton->setClickingTogglesState(false);
    mBackButton->setColour(DrawableButton::ColourIds::backgroundColourId, Colours::transparentBlack);
    mBackButton->setColour(DrawableButton::ColourIds::backgroundOnColourId, Colours::transparentBlack);
    auto back_icon_drawable = Drawable::createFromImageData(BinaryData::back_svg, BinaryData::back_svgSize);
    mBackButton->setImages(back_icon_drawable.get());
    mBackButton->onClick = [this]() {
        mProcessor.getPlayer()->reset();
        mPlayPauseButton->setToggleState(false, juce::sendNotification);
        mVisualizationPanel.getAudioMidiViewport().setViewPositionProportionately(0, 0);
    };

    addAndMakeVisible(*mBackButton);

    mPlayPauseButton = std::make_unique<DrawableButton>("PlayPauseButton", DrawableButton::ButtonStyle::ImageRaw);
    mPlayPauseButton->setClickingTogglesState(true);
    mPlayPauseButton->setColour(DrawableButton::ColourIds::backgroundColourId, Colours::transparentBlack);
    mPlayPauseButton->setColour(DrawableButton::ColourIds::backgroundOnColourId, Colours::transparentBlack);
    auto play_icon_drawable = Drawable::createFromImageData(BinaryData::play_svg, BinaryData::play_svgSize);
    auto pause_icon_drawable = Drawable::createFromImageData(BinaryData::pause_svg, BinaryData::pause_svgSize);
    mPlayPauseButton->setImages(
        play_icon_drawable.get(), nullptr, nullptr, nullptr, pause_icon_drawable.get(), nullptr, nullptr, nullptr);

    mPlayPauseButton->onClick = [this]() {
        if (mProcessor.getState() == PopulatedAudioAndMidiRegions) {
            mProcessor.getPlayer()->setPlayingState(mPlayPauseButton->getToggleState());
        } else {
            mPlayPauseButton->setToggleState(false, sendNotification);
        }
    };

    addAndMakeVisible(*mPlayPauseButton);
    
    
    mCenterButton = std::make_unique<DrawableButton>("PlayPauseButton", DrawableButton::ButtonStyle::ImageRaw);
    mCenterButton->setClickingTogglesState(true);
    mCenterButton->setColour(DrawableButton::ColourIds::backgroundColourId, Colours::transparentBlack);
    mCenterButton->setColour(DrawableButton::ColourIds::backgroundOnColourId, Colours::transparentBlack);
    auto center_icon_drawable_off =
        Drawable::createFromImageData(BinaryData::center_off_svg, BinaryData::center_off_svgSize);
    auto center_icon_drawable_on =
        Drawable::createFromImageData(BinaryData::center_on_svg, BinaryData::center_on_svgSize);
    mCenterButton->setImages(center_icon_drawable_off.get(),
                             nullptr,
                             nullptr,
                             nullptr,
                             center_icon_drawable_on.get(),
                             nullptr,
                             nullptr,
                             nullptr);
    mCenterButton->onClick = [this]() {
        mVisualizationPanel.getAudioContainer().setCenterView(mCenterButton->getToggleState());
    };

    addAndMakeVisible(*mCenterButton);
    

    mMuteButton = std::make_unique<juce::TextButton>("MuteButton");
    mMuteButton->setButtonText("");
    mMuteButton->setClickingTogglesState(true);

    mMuteButton->setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.5f));
    mMuteButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    mMuteButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white); // Set the text color when button is on
    mMuteButton->setColour(juce::TextButton::buttonOnColourId, Colour(177,55,217));
    
    mMuteButtonAttachment =
        std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(mProcessor.mTree, "MUTE", *mMuteButton);
    addAndMakeVisible(*mMuteButton);
    
    mModelChooserButton = std::make_unique<juce::TextButton>("ModelChooser");
    mModelChooserButton->setClickingTogglesState(false);
    mModelChooserButton->setButtonText(mProcessor.getModelPath());
    mModelChooserButton->setColour(juce::TextButton::ColourIds::textColourOnId, Colour(177,55,217));
    mModelChooserButton->setColour(juce::TextButton::ColourIds::buttonColourId, Colour::fromRGBA(0, 0, 0, 0));
    mModelChooserButton->setColour(juce::TextButton::ColourIds::buttonOnColourId, Colour::fromRGBA(0, 0, 0, 0));
    mModelChooserButton->setLookAndFeel(&modelChooseLNF);
    mModelChooserButton->onClick = [this]() {
        modelChoose();
    };
    
    addAndMakeVisible(*mModelChooserButton);
    
    addAndMakeVisible(mVisualizationPanel);

    startTimerHz(60);

    updateEnablements();
}

NeuVCMainView::~NeuVCMainView()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    mModelChooserButton->setLookAndFeel(nullptr);
}

void NeuVCMainView::resized()
{
    mRecordButton->setBounds(0, 115, 35, 35);
    mClearButton->setBounds(40, 115, 35, 35);
    mCenterButton->setBounds(80, 115, 35, 35);
    mBackButton->setBounds(120, 115, 35, 35);
    mPlayPauseButton->setBounds(160, 115, 35, 35);
    mMuteButton->setBounds(200, 115, 35, 35);
    mModelChooserButton->setBounds(400, 115, 400, 35);
    mVisualizationPanel.setBounds(0, 0, 800, 115);
}

void NeuVCMainView::paint(Graphics& g)
{
    auto background_image = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    g.drawImage(background_image, getLocalBounds().toFloat());
    g.setFont(LABEL_FONT);
}

void NeuVCMainView::timerCallback()
{
    auto processor_state = mProcessor.getState();
    if (mRecordButton->getToggleState() && processor_state != Recording) {
        mRecordButton->setToggleState(false, juce::sendNotification);
        updateEnablements();
    }

    if (mPlayPauseButton->getToggleState() != mProcessor.getPlayer()->isPlaying()) {
        mPlayPauseButton->setToggleState(mProcessor.getPlayer()->isPlaying(), sendNotification);
    }

    if (mPrevState != processor_state) {
        mPrevState = processor_state;
        updateEnablements();
    }

    // To avoid getting stuck in processing mode if processBlock is not called anymore and recording is over (can happen in some DAWs).
    if (mProcessor.getState() == Processing && !mProcessor.isJobRunningOrQueued()) {
        mNumCallbacksStuckInProcessingState += 1;
        if (mNumCallbacksStuckInProcessingState >= 10) {
            //mProcessor.launchTranscribeJob();
        }
    } else {
        mNumCallbacksStuckInProcessingState = 0;
    }
    mVisualizationPanel.getAudioContainer().getAudioRegion()->getPlayhead()->timerCallback();
    mVisualizationPanel.getAudioContainer().getAudioRegion()->getPlayhead()->repaint();
    mVisualizationPanel.getAudioContainer().timerCallback();
}

void NeuVCMainView::modelChoose()
{
    mFileChooser = std::make_shared<juce::FileChooser>(
        "Select Model File", juce::File {}, "*.pth;", true, false, this);

    mFileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this](const juce::FileChooser& fc) {
                                  if (fc.getResults().isEmpty())
                                      return;
                                  //auto* parent = dynamic_cast<AudioContainer*>(getParentComponent());
                                  //if (parent) {
                                      //parent->filesDropped(StringArray(fc.getResult().getFullPathName()), 1, 1);
                                  //}
                                  mProcessor.setModelPath(String(fc.getResult().getFullPathName()));
                                  mModelChooserButton->setButtonText(mProcessor.getModelPath());
                              });
}

void NeuVCMainView::updateEnablements()
{
    auto current_state = mProcessor.getState();
    mPrevState = current_state;

    if (current_state == EmptyAudioAndMidiRegions) {
        mRecordButton->setEnabled(true);
        mClearButton->setEnabled(false);
        mPlayPauseButton->setEnabled(false);
        mBackButton->setEnabled(false);
        mCenterButton->setEnabled(false);
    } else if (current_state == Recording) {
        mRecordButton->setEnabled(true);
        mClearButton->setEnabled(false);
        mPlayPauseButton->setEnabled(false);
        mBackButton->setEnabled(false);
        mCenterButton->setEnabled(false);
    } else if (current_state == Processing) {
        mRecordButton->setEnabled(false);
        // TODO: activate clear button to be able to cancel processing.
        mClearButton->setEnabled(false);
        mPlayPauseButton->setEnabled(false);
        mBackButton->setEnabled(false);
        mCenterButton->setEnabled(false);
    } else if (current_state == PopulatedAudioAndMidiRegions) {
        mRecordButton->setEnabled(false);
        mClearButton->setEnabled(true);
        mPlayPauseButton->setEnabled(true);
        mBackButton->setEnabled(true);
        mCenterButton->setEnabled(true);
        mVisualizationPanel.setMidiFileDragComponentVisible();
    }

    repaint();
}
