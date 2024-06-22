

#ifndef AudioRegion_h
#define AudioRegion_h

#include <JuceHeader.h>

#include "AudioUtils.h"
#include "PluginProcessor.h"
#include "UIDefines.h"
#include "Playhead.h"

class AudioContainer;

class AudioRegion : public Component, public juce::Timer
{
public:
    AudioRegion(NeuVCAudioProcessor* processor, double inNumPixelsPerSecond);

    void resized() override;

    void paint(Graphics& g) override;

    void setIsFileOver(bool inIsFileOver);

    void setThumbnailWidth(int inThumbnailWidth);

    void mouseDown(const juce::MouseEvent& e) override;
    
    Playhead* getPlayhead() { return &mPlayhead;};
    

private:
    void drawSpinningIcon(juce::Graphics& g);
    
    void timerCallback() override;
    
    float _pixelToTime(float inPixel) const;
    
    NeuVCAudioProcessor* mProcessor;
    Playhead mPlayhead;

    std::shared_ptr<juce::FileChooser> mFileChooser;

    const double mNumPixelsPerSecond;
    
    //juce::Image mIcon; // Image object to hold the icon
    juce::ImageButton mIcon;
    int mThumbnailWidth = 0;
    bool mIsFileOver = false;
    float mAngle; // Variable to store the angle for the spinning animation
    
};

#endif // AudioRegion_h
