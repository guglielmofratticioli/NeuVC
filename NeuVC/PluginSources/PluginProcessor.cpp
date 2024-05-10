#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <stdlib.h>

NeuVCAudioProcessor::NeuVCAudioProcessor()
    : mTree(*this, nullptr, "PARAMETERS", createParameterLayout())
    , mThreadPool(1)
{
    mJobLambda = [this]() { _runModel(); };

    mSourceAudioManager = std::make_unique<SourceAudioManager>(this);
    mPlayer = std::make_unique<Player>(this);
}

AudioProcessorValueTreeState::ParameterLayout NeuVCAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto mute = std::make_unique<juce::AudioParameterBool>(juce::ParameterID {"MUTE", 1}, "Mute", true);
    params.push_back(std::move(mute));

    auto audio_level_db = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"AUDIO_LEVEL_DB", 1}, "Audio Level dB", NormalisableRange<float>(-36.f, 6.0f, 1.0f), 0.0f);
    params.push_back(std::move(audio_level_db));
    
    auto midi_level_db = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"MIDI_LEVEL_DB", 1}, "Midi Level dB", NormalisableRange<float>(-36.f, 6.0f, 1.0f), 0.0f);
    params.push_back(std::move(midi_level_db));
     

    return {params.begin(), params.end()};
}

void NeuVCAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mSourceAudioManager->prepareToPlay(sampleRate, samplesPerBlock);
    mPlayer->prepareToPlay(sampleRate, samplesPerBlock);
}

void NeuVCAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    // Get tempo and time signature for UI.
    
    auto playhead_info = getPlayHead()->getPosition();
    if (playhead_info.hasValue()) {
        if (playhead_info->getBpm().hasValue())
            mCurrentTempo = *playhead_info->getBpm();
        if (playhead_info->getTimeSignature().hasValue()) {
            mCurrentTimeSignatureNum = playhead_info->getTimeSignature()->numerator;
            mCurrentTimeSignatureDenom = playhead_info->getTimeSignature()->denominator;
        }
    }
     

    mSourceAudioManager->processBlock(buffer);
    
    
    if (mState.load() == Recording) {
        if (!mWasRecording) {
            mWasRecording = true;
            mPlayheadInfoStartRecord = getPlayHead()->getPosition();
        }
    } else {
        // If we were previously recording but not anymore (user clicked record button to stop it).
        if (mWasRecording) {
            mWasRecording = false;
        }
    }
    
    auto isMute = mTree.getRawParameterValue("MUTE")->load() > 0.5;

    if (isMute)
        buffer.clear();

    mPlayer->processBlock(buffer);
}

juce::AudioProcessorEditor* NeuVCAudioProcessor::createEditor()
{
    return new NeuVCEditor(*this);
}

void NeuVCAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void NeuVCAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

void NeuVCAudioProcessor::clear()
{
    mPlayheadInfoStartRecord = juce::Optional<juce::AudioPlayHead::PositionInfo>();

    mCurrentTempo = -1;

    mMidiFileTempo = 120.0;

    mWasRecording = false;

    mPlayer->reset();
    mSourceAudioManager->clear();

    mState.store(EmptyAudioAndMidiRegions);
}

void NeuVCAudioProcessor::launchTranscribeJob()
{
    jassert(mState.load() == Processing);

    // Have at least one second to transcribe
    if (getSourceAudioManager()->getNumSamplesDownAcquired() >= 1 * 48000) {
        mThreadPool.addJob(mJobLambda);
    } else {
        clear();
    }
}


NeuVCAudioProcessor::Parameters* NeuVCAudioProcessor::getCustomParameters()
{
    return &mParameters;
}


const juce::Optional<juce::AudioPlayHead::PositionInfo>& NeuVCAudioProcessor::getPlayheadInfoOnRecordStart()
{
    return mPlayheadInfoStartRecord;
}
 

void NeuVCAudioProcessor::_runModel()
{
    //"python infer_cli.py --input_path --f0method --opt_path --model_name --index_rate --device"
    setenv("PATH", "/usr/local/bin/", 1);
    
    juce::String command = "cd /Users/guglielmofratticioli/Documents/Lib/Retrieval-based-Voice-Conversion-WebUI && ";
    command+="/Users/guglielmofratticioli/opt/miniconda3/bin/python ";
    command+=mRVCPath;
    command+=" --input_path ";
    command+=getSourceAudioManager()->getRecordedFile().getFullPathName();
    //command+="/Users/guglielmofratticioli/Downloads/output.wav";
    command+=" --opt_path ";
    //command+="/Users/guglielmofratticioli/Downloads/output2.wav";
    command+=getSourceAudioManager()->getRecordedFile().getFullPathName();
    command+=" --model_name ";
    command+=mModelPath;
    command+=" --index_rate ";
    command+="0 ";
    command+="--device ";
    command+="cpu ";
    
    //juce::ChildProcess process;
    //success = process.start(command);
    DBG(command);
    int result = std::system(command.toStdString().c_str());
        if (result != 0)
        {
            DBG("error");
        }
    
    getSourceAudioManager()->updateSourceAudio();
    mState.store(PopulatedAudioAndMidiRegions);
    
    
}

/*
void NeuVCAudioProcessor::updatePostProcessing()
{

    jassert(mState == PopulatedAudioAndMidiRegions);

    if (mState == PopulatedAudioAndMidiRegions) {
        mNoteOptions.setParameters(NoteUtils::RootNote(mParameters.keyRootNote.load()),
                                   NoteUtils::ScaleType(mParameters.keyType.load()),
                                   NoteUtils::SnapMode(mParameters.keySnapMode.load()),
                                   mParameters.minMidiNote.load(),
                                   mParameters.maxMidiNote.load());

        auto post_processed_notes = mNoteOptions.process(mBasicPitch.getNoteEvents());

        mRhythmOptions.setParameters(RhythmUtils::TimeDivisions(mParameters.rhythmTimeDivision.load()),
                                     mParameters.rhythmQuantizationForce.load());

        mPostProcessedNotes = mRhythmOptions.quantize(post_processed_notes);

        Notes::dropOverlappingPitchBends(mPostProcessedNotes);
        Notes::mergeOverlappingNotesWithSamePitch(mPostProcessedNotes);

        // For the synth
        auto single_events = SynthController::buildMidiEventsVector(mPostProcessedNotes);
        mPlayer->getSynthController()->setNewMidiEventsVectorToUse(single_events);
    }
     
}
 */


std::string NeuVCAudioProcessor::getTempoStr() const
{
    
    if (mPlayheadInfoStartRecord.hasValue() && mPlayheadInfoStartRecord->getBpm().hasValue())
        return std::to_string(static_cast<int>(std::round(*mPlayheadInfoStartRecord->getBpm())));
    else if (mCurrentTempo > 0)
        return std::to_string(static_cast<int>(std::round(mCurrentTempo.load())));
    else
        return "-";
     
}
 


std::string NeuVCAudioProcessor::getTimeSignatureStr() const
{
    
    if (mPlayheadInfoStartRecord.hasValue() && mPlayheadInfoStartRecord->getTimeSignature().hasValue()) {
        int num = mPlayheadInfoStartRecord->getTimeSignature()->numerator;
        int denom = mPlayheadInfoStartRecord->getTimeSignature()->denominator;
        return std::to_string(num) + " / " + std::to_string(denom);
    } else if (mCurrentTimeSignatureNum > 0 && mCurrentTimeSignatureDenom > 0)
        return std::to_string(mCurrentTimeSignatureNum.load()) + " / "
               + std::to_string(mCurrentTimeSignatureDenom.load());
    else
        return "- / -";
     
}
 

void NeuVCAudioProcessor::setMidiFileTempo(double inMidiFileTempo)
{
    mMidiFileTempo = inMidiFileTempo;
}
 


double NeuVCAudioProcessor::getMidiFileTempo() const
{
    return mMidiFileTempo;
}

bool NeuVCAudioProcessor::isJobRunningOrQueued() const
{
    return mThreadPool.getNumJobs() > 0;
}

Player* NeuVCAudioProcessor::getPlayer()
{
    return mPlayer.get();
}

SourceAudioManager* NeuVCAudioProcessor::getSourceAudioManager()
{
    return mSourceAudioManager.get();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeuVCAudioProcessor();
}
