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

#include "Gui/PresetPanel.h"


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
    
    Gui::PresetPanel presetPanel;
    
    void initComponents();
    void initRowControls();
    
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
    
    //ADSR sliders Vec
    juce::OwnedArray<juce::Slider> attackSliderVec_;
    juce::OwnedArray<juce::Slider> decaySliderVec_;
    juce::OwnedArray<juce::Slider> sustainSliderVec_;
    juce::OwnedArray<juce::Slider> releaseSliderVec_;
    
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > attackAttachmentVec_;
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > decayAttachmentVec_;
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > sustainAttachmentVec_;
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > releaseAttachmentVec_;
    
    //Pan slider Vec
    juce::OwnedArray<juce::Slider> panSliderVec_;
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > panAttachmentVec_;
    
    //Sliders Labels
    juce::Label rowChoiceLabel_;
    juce::Label attackLabel_;
    juce::Label decayLabel_;
    juce::Label sustainLabel_;
    juce::Label releaseLabel_;
    juce::Label panLabel_;
    
    // === Control Components
    //Sequencer
    juce::OwnedArray<CustomKnob> intervalSliderVec_;
    juce::OwnedArray<CustomKnob> pulseSliderVec_;
    juce::OwnedArray<CustomKnob> offsetSliderVec_;
    
    //Gain
    juce::OwnedArray<CustomKnob> gainSliderVec_;
    
    //Filter
    juce::OwnedArray<CustomKnob> frequencySliderVec_;
    juce::OwnedArray<CustomKnob> resonanceSliderVec_;
    
    //Attachments
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > intervalAttachmentVec_;
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > pulseAttachmentVec_;
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > offsetAttachmentVec_;
    
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > gainAttachmentVec_;
    
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > frequencyAttachmentVec_;
    std::vector< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> > resonanceAttachmentVec_;
    
    //Control Components Labels
    juce::Label intervalLabel_;
    juce::Label pulseLabel_;
    juce::Label offsetLabel_;
    
    juce::Label gainLabel_;

    juce::Label frequencyLabel_;
    juce::Label resonanceLabel_;

    // === CHEAT : Restart sequence if changed directly on the sequencer
    //Using fake slider, that will be "listened" to after the other sliders
    //that put a new sequence otherwise
    juce::Slider fakeSeqSlider_;
    std::vector< std::vector<bool> > restartArray_;
    bool isNotRestarted_ {true};
    
    
    
    transportState state_;
    
    //=== Step Sequencer
    juce::String name_;
    juce::Colour colour_;
    
    float startTime_;
    
    SequenceGenerator seqGenerator_;
    
    std::vector<int> playVec_;
    std::vector<int> notesVec_;
    std::vector<int> globalCurrentStepVec_;  
    
    //Sync parameters
    int bpm_;
    std::vector<double> stepTimeVec_;
    bool noRowOn_;
    
    //Time signature
    int localTimeSigNumerator_;
    int localTimeSignDenominator_;
    
    StepSequencerData* stepData_ = NULL;
    
    juce::Image backgroundImage_;
    
    //Check the comboBox in the panel to load audio
    OwnedArray<juce::ComboBox> loadBoxVec_;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KlideAudioProcessorEditor)
};
