#pragma once

#include "atomic"
#include <JuceHeader.h>

//#include "Resampler.h"
#include "ProcessorBase.h"
#include "Player.h"
#include "SourceAudioManager.h"
#include "rvc.h"
#include <vector>


enum State { EmptyAudioAndMidiRegions = 0, Recording, Processing, PopulatedAudioAndMidiRegions };

class NeuVCAudioProcessor : public PluginHelpers::ProcessorBase
{
public:
    struct Parameters {
        std::atomic<float> noteSensibility = 0.7;
        std::atomic<float> splitSensibility = 0.5;
        std::atomic<float> minNoteDurationMs = 125;
    };

    NeuVCAudioProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    State getState() const { return mState.load(); }

    void setStateToRecording() { mState.store(Recording); }

    void setStateToProcessing() { mState.store(Processing); }
    
    void setStateToPopulated()  { mState.store(PopulatedAudioAndMidiRegions); }

    void clear();

    // TODO: function to put in a new class TranscriptionManager
    void launchConversionJob();

    bool isJobRunningOrQueued() const;


    Parameters* getCustomParameters();

    const juce::Optional<juce::AudioPlayHead::PositionInfo>&
        getPlayheadInfoOnRecordStart(); // TODO: Add to timeQuantizeManager

    // Value tree state to pass automatable parameters from UI
    juce::AudioProcessorValueTreeState mTree;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // TODO: TimeQuantizeManager
    
     std::string getTempoStr() const;

     std::string getTimeSignatureStr() const;

    void setMidiFileTempo(double inMidiFileTempo);

    double getMidiFileTempo() const;

    SourceAudioManager* getSourceAudioManager();

    Player* getPlayer();
    
    juce::String getModelPath() const { return mModelPath; };
    void setModelPath(juce::String path) { mModelPath = path; };
    
    void toggleProcessMode(){
        if(mProcessMode == "python") 
            mProcessMode = "c++";
        else 
            mProcessMode ="python";
    };
    
    juce::String getProcessMode() const {return mProcessMode;};
    
private:
    
    void _runModel(); // Add to TranscriptionManager
    
    std::atomic<State> mState = EmptyAudioAndMidiRegions;
    
    
    std::vector<float> loadAudioFile(const juce::File& file);
    
    std::vector<float> normalizAudio(const std::vector<float>& audioData, float targetLevel = 1.0f);
    
    std::vector<float> resampleAudio(const std::vector<float>& input, double inputSampleRate, double outputSampleRate);
    
    void saveAudioToFile(const std::vector<float>& audioData, int sampleRate, int numChannels, const juce::File& file);
    

    std::unique_ptr<SourceAudioManager> mSourceAudioManager;
    std::unique_ptr<Player> mPlayer;
    RVC rvc;
    
    Parameters mParameters;
    bool mWasRecording = false;

    std::atomic<double> mCurrentTempo = -1.0;
    std::atomic<int> mCurrentTimeSignatureNum = -1;
    std::atomic<int> mCurrentTimeSignatureDenom = -1;
    
    std::vector<float> recordedAudio;
    std::vector<float> convertedAudio;
    
    juce::String mProcessMode = "python";
    juce::String mModelPath = "select RVC model file path ->";
    juce::String mRVCPath = "/Users/guglielmofratticioli/Documents/Lib/Retrieval-based-Voice-Conversion-WebUI/infer_cli.py";


    double mMidiFileTempo = 120.0;

    // To quantize manager
    juce::Optional<juce::AudioPlayHead::PositionInfo> mPlayheadInfoStartRecord;

    // Thread pool to run ML in background thread. To transcription manager
    juce::ThreadPool mThreadPool;
    std::function<void()> mJobLambda;
};
