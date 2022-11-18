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

#include "adsr_editor.h"

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
    
    void timerCallback() override;
    void buttonClicked (juce::Button* button) override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    

private:
    
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    //KlideAudioProcessor& audioProcessor;
    KlideAudioProcessor& audioProcessor_;
    
    void initComponents();
    
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
    int numrows_ {4};
    
    juce::OwnedArray<ControlsComponent> controlsArray_;
    juce::OwnedArray<StepSequencerComponent> stepSequencerArray_;
    
    //Buttons
    juce::TextButton syncButton_;
    juce::TextEditor testBox_;

    //Row choice
    juce::Slider rowChoiceSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rowChoiceSliderAttachment_;
    
    //ADSR sliders
    juce::Slider attackSlider_;
    juce::Slider decaySlider_;
    juce::Slider sustainSlider_;
    juce::Slider releaseSlider_;
    
    //Saving vectors
    std::vector<juce::ADSR::Parameters> adsrParamsVec_;
    std::vector<float> panVec_;
    
    //Pan slider
    juce::Slider panSlider_;

    
    //Sliders Labels
    juce::Label rowChoiceLabel_;
    juce::Label attackLabel_;
    juce::Label decayLabel_;
    juce::Label sustainLabel_;
    juce::Label releaseLabel_;
    juce::Label panLabel_;

    
    //Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackSliderAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decaySliderAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainSliderAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseSliderAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panSliderAttachment_;
    
    //CHEAT : Restart sequence if changed directly on the sequencer
    //Using fake slider, that will be "listened" to after the other sliders
    //that put a new sequence otherwise
    juce::Slider fakeSeqSlider_;
    std::vector< std::vector<bool> > restartArray_;
    bool isNotRestarted_ {true};

    
    
    
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
