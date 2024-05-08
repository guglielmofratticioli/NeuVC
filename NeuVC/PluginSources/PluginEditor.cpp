#include "PluginProcessor.h"
#include "PluginEditor.h"

NeuVCEditor::NeuVCEditor(NeuVCAudioProcessor& p)
    : AudioProcessorEditor(&p)
{
    mMainView = std::make_unique<NeuVCMainView>(p);

    addAndMakeVisible(*mMainView);
    setSize(800, 150);

    getLookAndFeel().setDefaultSansSerifTypeface(MONTSERRAT_REGULAR);

    mMainView->setLookAndFeel(&mNeuVCLnF);
}

NeuVCEditor::~NeuVCEditor()
{
    mMainView->setLookAndFeel(nullptr);
}

void NeuVCEditor::paint(juce::Graphics& g)
{
}

void NeuVCEditor::resized()
{
    mMainView->setBounds(getLocalBounds());
}
