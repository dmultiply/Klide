/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "ControlsComponent.h"
#include "StepSequencerComponent.h"
#include "SequenceGenerator.h"
#include "StepSequencerData.h"

#include "CustomKnob.h"

//==============================================================================
/**
*/
class KlideAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::TextButton::Listener, public juce::Timer, public juce::Slider::Listener
{
public:
    KlideAudioProcessorEditor (KlideAudioProcessor&);
    ~KlideAudioProcessorEditor() override;

    void init();
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback();
    void buttonClicked (juce::Button* button);
    
    void sliderValueChanged (juce::Slider *slider);
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    //KlideAudioProcessor& audioProcessor;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KlideAudioProcessor& processor_;
    
    // =========== Types
    enum transportState
    {
        stopped,
        starting,
        playing,
        stopping
    };
    
    // =========== Methods
    void openButtonClicked(int row);
    void playButtonClicked(int row);
    void stopButtonClicked(int row);
    
    void intervalValueChanged(juce::Slider *slider, int row);
    void pulseValueChanged(juce::Slider *slider, int row);
    void gainValueChanged(juce::Slider *slider, int row);
    void offsetValueChanged(juce::Slider *slider, int row);
    //Filter
    void frequencyValueChanged(juce::Slider *slider, int row);
    void resonanceValueChanged(juce::Slider *slider, int row);
    
    void syncButtonClicked();
    
    std::vector< std::vector<bool> > getStatesArray();
    
    // =========== Variables
    int numrows_;
    
    juce::OwnedArray<ControlsComponent> controlsArray_;
    juce::OwnedArray<StepSequencerComponent> stepSequencerArray_;
    
    //Buttons
    juce::TextButton syncButton_;
    juce::TextEditor testBox_;
    
    
    transportState state_;
    
    //Step Sequencer
    juce::String name_;
    juce::Colour colour_;
    
    float startTime_;
    
    SequenceGenerator seqGenerator_;
    
    std::vector<int> playVec_;
    
    std::vector<int> globalCurrentStepVec_;
    
    std::vector<int> intervalVec_;
    std::vector<int> pulseVec_;
    std::vector<float> gainVec_;
    std::vector<int> offsetVec_;
    std::vector<int> frequencyVec_;
    std::vector<float> resonanceVec_;
    
    std::vector< std::vector<int> > intervalSliderRangeVec_;
    std::vector< std::vector<int> > pulseSliderRangeVec_;
    std::vector< std::vector<float> > gainSliderRangeVec_;
    std::vector< std::vector<int> > offsetSliderRangeVec_;
    std::vector< std::vector<int> > frequencySliderRangeVec_;
    std::vector< std::vector<float> > resonanceSliderRangeVec_;
    
    
    std::vector<int> notesVec_;
    
    //Sync parameters
    int bpm_;
    std::vector<double> stepTimeVec_;
    bool noRowOn_;
    
    //Time signature
    int localTimeSigNumerator_;
    int localTimeSignDenominator_;
    
    StepSequencerData* stepData_ = NULL;
    
    juce::Image backgroundImage_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KlideAudioProcessorEditor)
};
