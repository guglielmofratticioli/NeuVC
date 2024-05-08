
#ifndef NeuVCLNF_h
#define NeuVCLNF_h

#include "JuceHeader.h"
#include "UIDefines.h"

class NeuVCLNF : public juce::LookAndFeel_V4
{
public:
    NeuVCLNF();

    Font getComboBoxFont(ComboBox& /*box*/) override { return LABEL_FONT; }

    Font getPopupMenuFont() override { return LABEL_FONT; }

    Font getTextButtonFont(TextButton&, int buttonHeight) override { return LARGE_FONT; };

    Font getLabelFont(juce::Label&) override { return DROPDOWN_FONT; };

    void drawRotarySlider(Graphics&,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          Slider&) override;
};

#endif // NeuVCLNF_h
