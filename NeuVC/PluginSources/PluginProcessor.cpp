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
    RVC rvc;
    

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

void NeuVCAudioProcessor::launchConversionJob()
{
    jassert(mState.load() == Processing);
    mThreadPool.addJob(mJobLambda);
    /* Have at least one second to transcribe
    if (getSourceAudioManager()->getNumSamplesDownAcquired() >= 1 * 48000) {
     mThreadPool.addJob(mJobLambda);
    } else {
        clear();
    }
     */
}


NeuVCAudioProcessor::Parameters* NeuVCAudioProcessor::getCustomParameters()
{
    return &mParameters;
}


const juce::Optional<juce::AudioPlayHead::PositionInfo>& NeuVCAudioProcessor::getPlayheadInfoOnRecordStart()
{
    return mPlayheadInfoStartRecord;
}
 
// int setenv(const char *name, const char *value, int overwrite)
// {
//     int errcode = 0;
//     if(!overwrite) {
//         size_t envsize = 0;
//         errcode = getenv_s(&envsize, NULL, 0, name);
//         if(errcode || envsize) return errcode;
//     }
//     return _putenv_s(name, value);
// }

void NeuVCAudioProcessor::_runModel()
{
    if(mProcessMode =="python"){

        //"python infer_cli.py --input_path --f0method --opt_path --model_name --index_rate --device"
        setenv("PATH", "/usr/local/bin/", 1);
        
        // -> AUTO PATHS
        juce::String workDir = getSourceAudioManager()->getRecordedFile().getParentDirectory().getFullPathName();
        juce::String command ="cd ";
        command += workDir;
        command +=" && ";
        // - - - - - - - - - - - -
        
        // -> MANUAL PATH
        //juce::String command = "cd /Users/guglielmofratticioli/Documents/Lib/Retrieval-based-Voice-Conversion-WebUI && ";
        // - - - - - - - - - - - -
        
        command+="/Users/guglielmofratticioli/opt/miniconda3/bin/python ";
        
        // -> MANUAL PATH
        //command+=mRVCPath;
        // - - - - - - - - - - - -
        
        // -> AUTO PATHS
        command+= workDir;
        command+="/infer_cli.py ";
        // - - - - - - - - - - - -
        
        command+=" --input_path ";
        command+=getSourceAudioManager()->getRecordedFile().getFullPathName();
        command+=" --opt_path ";
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
    else if (mProcessMode == "c++"){
        //TODO C++ Write Here libtorch processing
        auto file = getSourceAudioManager()->getRecordedFile();
        std::vector<float> audio = loadAudioFile(file);
        
        std::vector<float> normalized_audio;
        normalized_audio = normalizAudio(audio);
       
        std::vector<float> resampled_audio;
        resampled_audio = resampleAudio(normalized_audio, 48000, 16000);
        
        recordedAudio = resampled_audio;
        convertedAudio = rvc.voiceConversion(resampled_audio);
        
        juce::Logger::writeToLog("Audio Converted");
        
        juce::File directory = file.getParentDirectory();
        juce::File converted_file = directory.getChildFile("audio_converted.wav");
        
        saveAudioToFile(convertedAudio,  16000, 1, converted_file);
        
        converted_file.copyFileTo(file);
        
        juce::Logger::writeToLog("File Saved");
        
        getSourceAudioManager()->updateSourceAudio();
        mState.store(PopulatedAudioAndMidiRegions);
    }
}

// TODO: This function is faiing
void NeuVCAudioProcessor::saveAudioToFile(const std::vector<float>& audioData, int sampleRate, int numChannels, const juce::File& file)
{
    // Number of channels and length of audio data
       const int numSamples = static_cast<int>(audioData.size());
       
       // Create an AudioBuffer and copy the data into it
       juce::AudioBuffer<float> buffer(numChannels, numSamples);
       buffer.copyFrom(0, 0, audioData.data(), numSamples);

       // Prepare an output stream to the file
       std::unique_ptr<juce::FileOutputStream> fileStream(file.createOutputStream());

       // Check if the file stream was created successfully
       if (fileStream == nullptr || !fileStream->openedOk())
       {
           DBG("Failed to create output stream for file: " + file.getFullPathName());
           return;
       }

       // Create a WavAudioFormat object to handle writing to the file
       juce::WavAudioFormat wavFormat;

       // Create an output stream to write the WAV file
       std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(fileStream.get(), sampleRate, numChannels, 16, {}, 0));

       // Check if the writer was created successfully
       if (writer == nullptr)
       {
           DBG("Failed to create WAV writer for file: " + file.getFullPathName());
           return;
       }

       // Write the audio data to the file
       writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);

       // Manually release the FileOutputStream to ensure it's properly closed after writing
       fileStream.release();
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


std::vector<float> NeuVCAudioProcessor::loadAudioFile(const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
        throw std::runtime_error("Failed to create reader for audio file");

    juce::AudioBuffer<float> buffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

    std::vector<float> audioData;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        audioData.insert(audioData.end(), buffer.getReadPointer(channel), buffer.getReadPointer(channel) + buffer.getNumSamples());
    }

    return audioData;
}

std::vector<float> NeuVCAudioProcessor::normalizAudio(const std::vector<float>& audioData, float targetLevel )
{
    // Find the maximum absolute value in the audio data
    float maxVal = *std::max_element(audioData.begin(), audioData.end(),
                                     [](float a, float b) { return std::abs(a) < std::abs(b); });

    // Avoid division by zero
    if (maxVal == 0.0f)
        return audioData;

    // Calculate the normalization factor
    float normalizationFactor = targetLevel / maxVal;

    // Create a new vector to hold normalized audio data
    std::vector<float> normalizedAudio;
    normalizedAudio.reserve(audioData.size());

    // Apply normalization
    std::transform(audioData.begin(), audioData.end(), std::back_inserter(normalizedAudio),
                   [normalizationFactor](float sample) { return sample * normalizationFactor; });

    return normalizedAudio;
}

std::vector<float> NeuVCAudioProcessor::resampleAudio(const std::vector<float>& input, double inputSampleRate, double outputSampleRate)
{
    // Ensure the input sample rate and output sample rate are valid
    if (inputSampleRate <= 0 || outputSampleRate <= 0)
        throw std::invalid_argument("Sample rates must be positive");

    // Calculate the resampling ratio
    double resampleRatio = inputSampleRate / outputSampleRate;

    // Create the resampler
    juce::LagrangeInterpolator resampler;

    // Prepare the output vector
    int numOutputSamples = static_cast<int>(input.size() / resampleRatio);
    std::vector<float> output(numOutputSamples);

    // Perform the resampling
    resampler.process(resampleRatio, input.data(), output.data(), numOutputSamples);

    return output;
}
