/*
  ==============================================================================

    StepSequencerComponent.cpp
    Created: 22 Nov 2017 4:08:36pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "StepSequencerComponent.h"

StepSequencerComponent::StepSequencerComponent ()
: numberOfSteps_(8), numberOfPulses_(5), currentStep_(0)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    //setSize (80, 100);
    
    //Initialize the step states to 0
    for(int i=0;i<numberOfSteps_;i++)
        stepStates_.push_back(0);
}

StepSequencerComponent::~StepSequencerComponent()
{
}

//==============================================================================

void StepSequencerComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (juce::Colour((juce::uint8)44,(juce::uint8)53,(juce::uint8)57,1.0f));
    
    //g.setColour (juce::Colour((juce::uint8)34,(juce::uint8)43,(juce::uint8)47,1.0f));
    g.setFont (15.0f);
    
    float scaleSize = 1, scaleTop = 1; //Change the size of the rectangle if current step
    
    float stepWidth = (getWidth())/numberOfSteps_;
        
    for(int i=0;i<numberOfSteps_;i++)
    {
        
        if(stepStates_[i]==1)
            g.setColour(juce::Colours::slategrey);
        else
            g.setColour (juce::Colour((juce::uint8)34,(juce::uint8)43,(juce::uint8)47,1.0f));
        
        //Change the size of the rectangle
        //paint is asynchronous, so step() adds one to current step before it shows
        //we compensate for this by starting the number of steps at -1

        if(currentStep_ == i)
        {
            scaleSize = 0.95;
            scaleTop = 0.025;
        }
        else
        {
            scaleSize = 0.9;
            scaleTop = 0.05;
        }
        
        g.fillRoundedRectangle(stepWidth*i, 10, stepWidth*.9*scaleSize, 40*scaleSize, 5.f);
    }
}


void StepSequencerComponent::step()
{
    
    if(currentStep_<numberOfSteps_-1) {
        currentStep_+=1;
    }
    else
        currentStep_=0;
    
    repaint();

}

void StepSequencerComponent::mouseDown(const juce::MouseEvent& event)
{
    int x = event.getMouseDownX();
    float stepWidth = getWidth()/numberOfSteps_;
    for(int i=0;i<numberOfSteps_;i++)
    {
        if(x>i*stepWidth && x<i*stepWidth+stepWidth) {
            if(stepStates_[i]==1) {
                stepStates_[i] = 0;
            }
            else {
                stepStates_[i] = 1;
            }
        }
        
        repaint();
    }
    
}


void StepSequencerComponent::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
}


//Getters/Setters
int StepSequencerComponent::getNumberOfSteps()
{
    return numberOfSteps_;
}

int StepSequencerComponent::getNumberOfPulses()
{
    return numberOfPulses_;
}

int StepSequencerComponent::getCurrentStep()
{
    return currentStep_;
}

void StepSequencerComponent::setNumberOfSteps(int num)
{
    numberOfSteps_ = num;
}

void StepSequencerComponent::setNumberOfPulses(int num)
{
    numberOfPulses_ = num;
}

void StepSequencerComponent::setOffset(int num)
{
    offset_ = num;
}

void StepSequencerComponent::setCurrentStep(int num)
{
    currentStep_ = num;
}

void StepSequencerComponent::setStepStates(std::vector<bool> states)
{
    stepStates_.clear();
    for(int i=0;i<states.size();i++)
        stepStates_.push_back(states[i]);
    
    repaint();
}

int StepSequencerComponent::getCurrentStepState()
{
    jassert(currentStep_<stepStates_.size());
    
    return stepStates_[currentStep_];
}

std::vector<bool> StepSequencerComponent::getStepStates()
{
    return stepStates_;
}

