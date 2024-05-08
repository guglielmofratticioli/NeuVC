

#ifndef AudioContainer_h
#define AudioContainer_h

#include <JuceHeader.h>

#include "AudioRegion.h"
#include "PluginProcessor.h"

class AudioContainer
    : public Component
    , public FileDragAndDropTarget
    , public ChangeListener
{
public:
    AudioContainer(NeuVCAudioProcessor* processor);

    ~AudioContainer() override;

    void setViewportPtr(juce::Viewport* inViewportPtr);

    void resized() override;

    void paint(Graphics& g) override;

    bool isInterestedInFileDrag(const StringArray& files) override;

    void filesDropped(const StringArray& files, int x, int y) override;

    void fileDragEnter(const StringArray& files, int x, int y) override;

    void fileDragExit(const StringArray& files) override;

    void setBaseWidth(int inWidth);

    void resizeAccordingToNumSamplesAvailable();

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void setCenterView(bool inShouldCenterView);

    AudioRegion* getAudioRegion();


    double mNumPixelsPerSecond = 100.0;

    const int mAudioRegionHeight = 115;
    const int mHeightBetweenAudioMidi = 0;
    void timerCallback();
    
    //void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);
    
private:

    void _centerViewOnPlayhead();

    NeuVCAudioProcessor* mProcessor;

    juce::Viewport* mViewportPtr = nullptr;
    //juce::VBlankAttachment mVBlankAttachment;

    bool mShouldCenterView = true;

    int mBaseWidth = 0;

    AudioRegion mAudioRegion;
};

#endif // AudioContainer_h
