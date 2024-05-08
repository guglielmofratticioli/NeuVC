

#ifndef Playhead_h
#define Playhead_h

#include "PluginProcessor.h"
#include <JuceHeader.h>

class Playhead : public Component
{
public:
    Playhead(NeuVCAudioProcessor* inProcessor, double inNumPixelsPerSecond);

    void resized() override;

    void paint(juce::Graphics& g) override;

    void setPlayheadTime(double inNewTime);

    static double computePlayheadPositionPixel(double inPlayheadPositionSeconds,
                                               double inSampleDuration,
                                               double inNumPixelPerSecond,
                                               int inWidth);

    void timerCallback();
    
private:
    
    NeuVCAudioProcessor* mProcessor;
    VBlankAttachment mVBlankAttachment;

    double mCurrentPlayerPlayheadTime = 10.0;
    double mAudioSampleDuration = 0;
    const double mNumPixelsPerSecond;
    static constexpr float mTriangleSide = 8.0f;
    static constexpr float mTriangleHeight = 0.86602540378 * mTriangleSide; // Sqrt(3) / 2
};

#endif // Playhead_h
