#pragma once

#include "PluginProcessor.h"
#include "NeuVCMainView.h"
#include "NeuVCLNF.h"

class NeuVCEditor : public juce::AudioProcessorEditor
{
public:
    explicit NeuVCEditor(NeuVCAudioProcessor&);

    ~NeuVCEditor();

    void paint(juce::Graphics&) override;

    void resized() override;

private:
    std::unique_ptr<NeuVCMainView> mMainView;

    NeuVCLNF mNeuVCLnF;
};
