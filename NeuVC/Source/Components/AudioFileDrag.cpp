

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
    /*
    if (!mTempDirectory.isDirectory()) {
        auto result = mTempDirectory.createDirectory();
        if (result.failed()) {
            NativeMessageBox::showMessageBoxAsync(
                juce::MessageBoxIconType::NoIcon, "Error", "Temporary directory for midi file failed.");
        }
    }

    std::string filename = mProcessor->getSourceAudioManager()->getDroppedFilename();

    if (filename.empty())
        filename = "NNTranscription.mid";
    else
        filename += "_NNTranscription.mid";
     */
    /*
    auto success_midi_file_creation = mMidiFileWriter.writeMidiFile(
        mProcessor->getNoteEventVector(),
        out_file,
        mProcessor->getPlayheadInfoOnRecordStart(),
        mProcessor->getMidiFileTempo(),
        static_cast<PitchBendModes>(mProcessor->getCustomParameters()->pitchBendMode.load()));
     
    if (!success_midi_file_creation) {
        NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::NoIcon, "Error", "Could not create the midi file.");
    }
    */
    //StringArray out_files = {out_file.getFullPathName()};
    auto file = mProcessor->getSourceAudioManager()->getRecordedFile();
    
    String originalFileName = file.getFileNameWithoutExtension();
        String originalFileExtension = file.getFileExtension();

        // Generate a unique filename
        int index = 0;
        File copiedFile;
        do {
            String newFileName = originalFileName + "_" + String(++index);
            copiedFile = file.getParentDirectory().getChildFile(newFileName).withFileExtension(originalFileExtension);
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
