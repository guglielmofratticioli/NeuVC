

#include "AudioRegion.h"
#include "AudioContainer.h"

AudioRegion::AudioRegion(NeuVCAudioProcessor* processor, double inNumPixelsPerSecond)
    : mProcessor(processor)
    , mPlayhead(processor, inNumPixelsPerSecond)
    , mNumPixelsPerSecond(inNumPixelsPerSecond)
{
    addAndMakeVisible(mPlayhead);
}

void AudioRegion::resized()
{
    mPlayhead.setSize(getWidth(), getHeight());
}

void AudioRegion::paint(Graphics& g)
{
    auto num_samples_available = mProcessor->getSourceAudioManager()->getNumSamplesDownAcquired();

    auto* thumbnail = mProcessor->getSourceAudioManager()->getAudioThumbnail();

    if (num_samples_available > 0 && thumbnail->isFullyLoaded()) {
        g.setColour(WAVEFORM_BG_COLOR);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 2.0f);

        auto thumbnail_area = getLocalBounds();
        thumbnail_area.setWidth(mThumbnailWidth);

        g.setColour(WAVEFORM_COLOR);

        thumbnail->drawChannel(g,
                               thumbnail_area,
                               0.0,
                               num_samples_available / 48000,
                               0,
                               0.95f / std::max(thumbnail->getApproximatePeak(), 0.1f));
    } else if (mProcessor->getState() == Processing) {
        g.setColour(WAVEFORM_BG_COLOR);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    } else {
        if (mIsFileOver)
            g.setColour(WAVEFORM_BG_COLOR);
        else {
            uint8 red = 255;
            uint8 green = 255;
            uint8 blue = 255;
            uint8 alpha = 10;
            g.setColour(Colour(red, green, blue, alpha));
        }
            

        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

        g.setColour(Colour(177,55,217));
        g.setFont(LARGE_FONT);

        if (mIsFileOver)
            g.drawText("YUMMY!", getLocalBounds(), juce::Justification::centred);
        else
            g.drawText("LOAD OR DROP AN AUDIO FILE", getLocalBounds(), juce::Justification::centred);
    }
}

void AudioRegion::setIsFileOver(bool inIsFileOver)
{
    mIsFileOver = inIsFileOver;
}

void AudioRegion::setThumbnailWidth(int inThumbnailWidth)
{
    mThumbnailWidth = inThumbnailWidth;
}

void AudioRegion::mouseDown(const juce::MouseEvent& e)
{
    getPlayhead()->repaint();
    
    if (mProcessor->getState() == EmptyAudioAndMidiRegions) {
        mFileChooser = std::make_shared<juce::FileChooser>(
            "Select Audio File", juce::File {}, "*.wav;*.aiff;*.flac;*.mp3;*.ogg", true, false, this);

        mFileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this](const juce::FileChooser& fc) {
                                      if (fc.getResults().isEmpty())
                                          return;
                                      auto* parent = dynamic_cast<AudioContainer*>(getParentComponent());
                                      if (parent) {
                                          parent->filesDropped(StringArray(fc.getResult().getFullPathName()), 1, 1);
                                      }
                                  });
    } else if (mProcessor->getState() == PopulatedAudioAndMidiRegions) {
        mPlayhead.setPlayheadTime(_pixelToTime((float) e.x));
    }
}

float AudioRegion::_pixelToTime(float inPixel) const
{
    return inPixel / static_cast<float>(mNumPixelsPerSecond);
}
