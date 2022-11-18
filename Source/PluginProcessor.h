/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SimpleSynth.h"
#include "StepSequencerData.h"

//==============================================================================
/**
*/
class KlideAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    KlideAudioProcessor();
    ~KlideAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    const juce::String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect () const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    //=== Made by Mael =============================================================
    SimpleSynth* getSynth();
    
    //Getters/Setters
    //void setStepData(StepSequencerData *stepData);
    StepSequencerData* getStepData();
    void setSyncOn(bool syncOn);
    void setGain(float level, int row);
    
    juce::String getInfo();
    
    void updateADSRParams();
    juce::ADSR::Parameters getADSRParams(int row);
    
    void updatePan();
    
    juce::AudioProcessorValueTreeState tree_;
    
private:
    
    //In case of using a tree
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    //==============================================================================
    
    double computeNextBarSyncTime(juce::AudioPlayHead::CurrentPositionInfo currentPosition, double syncTimeDiff);
    
    
    SimpleSynth synth_;
    StepSequencerData* stepData_ = NULL;
    
    double sampleRate_;
    
    bool syncOn_;
    
    // ======================
    
    std::vector<float> gainVec_;
    
    juce::String info_;
    
    //Update Filter parameters in the sampler
    void updateFilter();
    
    
    
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KlideAudioProcessor)
};
