/*
  ==============================================================================

    StepSequencerComponent.h
    Created: 22 Nov 2017 4:08:36pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class StepSequencerComponent  : public juce::Component
{
public:
    StepSequencerComponent ();
    ~StepSequencerComponent();
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void step();
    
    void mouseDown(const juce::MouseEvent& event) override;
    
    //Getters/Setters
    int getNumberOfSteps();
    int getNumberOfPulses();
    int getCurrentStep();
    int getCurrentStepState();
    std::vector<bool> getStepStates();
    
    void setNumberOfSteps(int num);
    void setNumberOfPulses(int num);
    void setOffset(int num);
    void setCurrentStep(int num);
    void setStepStates(std::vector<bool> states);
    
private:
    
    //Step Sequencer
    juce::String name_;
    juce::Colour colour_;
    
    int numberOfSteps_;
    int numberOfPulses_;
    int offset_;
    int currentStep_;
    std::vector<bool> stepStates_;
        
};


