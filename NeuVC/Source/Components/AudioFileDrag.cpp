

#include "AudioFileDrag.h"

AudioFileDrag::AudioFileDrag(NeuVCAudioProcessor* processor)
    : mProcessor(processor)
{
}

AudioFileDrag::~AudioFileDrag()
{
    /*
    if (mTempDirectory.isDirectory()) {
        mTempDirectory.deleteRecursively();
    }
     */
}

void AudioFileDrag::resized()
{
}

void AudioFileDrag::paint(Graphics& g)
{
    g.setColour(Colour::fromRGBA(177, 55, 215, 100));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    g.setColour(Colour(28,28,28));
    g.setFont(LARGE_FONT);
    g.drawText(">", getLocalBounds(), juce::Justification::centred);
}

void AudioFileDrag::mouseDown(const MouseEvent& event)
{

    auto file = mProcessor->getSourceAudioManager()->getRecordedFile();
    
    String originalFileName = file.getFileNameWithoutExtension();
    String originalFileExtension = file.getFileExtension();

    // Create the exports directory within the parent directory of the original file
    File exportsDirectory = file.getParentDirectory().getChildFile("exports");
    if (!exportsDirectory.exists()) {
        exportsDirectory.createDirectory();
    }

    // Generate a unique filename in the exports directory
    int index = 0;
    File copiedFile;
    do {
        String newFileName = originalFileName + "_" + String(++index);
        copiedFile = exportsDirectory.getChildFile(newFileName).withFileExtension(originalFileExtension);
    } while (copiedFile.exists()); // Ensure the filename is unique

    file.copyFileTo(copiedFile);
    DragAndDropContainer::performExternalDragDropOfFiles(copiedFile.getFullPathName(), false, this);
}

void AudioFileDrag::mouseEnter(const MouseEvent& event)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

void AudioFileDrag::mouseExit(const MouseEvent& event)
{
    setMouseCursor(juce::MouseCursor::ParentCursor);
}
