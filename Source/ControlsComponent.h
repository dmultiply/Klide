/*
  ==============================================================================

    ControlsComponent.h
    Created: 22 Nov 2017 5:48:54pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"
#include "CustomKnob.h"

class ControlsComponent  : public juce::Component
{
public:
    ControlsComponent ();
    ~ControlsComponent();
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void setIntervalSliderVal(int val);
    void setPulseSliderVal(int val);
    void setGainSliderVal(float val);
    void setOffsetSliderVal(int val);
    void setFrequencySliderVal(int val);
    void setResonanceSliderVal(int val);
    
    void setIntervalSliderRange(int min, int max);
    void setPulseSliderRange(int min, int max);
    void setGainSliderRange(double min, double max);
    void setOffsetSliderRange(int min, int max);
    void setFrequencySliderRange(int min, int max);
    void setResonanceSliderRange(float min, float max);

    
    
    int getPulseSliderVal();

        
private:
    //Methods
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    
    juce::ImageButton openButton_;
    juce::ImageButton playButton_;
    juce::ImageButton stopButton_;
    
    /*
    juce::Slider intervalSlider_;
    juce::Slider pulseSlider_;
    
    juce::Slider gainSlider_;
    
    juce::Slider offsetSlider_;
     */
    CustomKnob intervalSlider_;
    CustomKnob pulseSlider_;
    CustomKnob gainSlider_;    
    CustomKnob offsetSlider_;
    //Filter slider
    CustomKnob frequencySlider_;
    CustomKnob resonanceSlider_;
    
    //Labels
    juce::Label intervalLabel_;
    
    
    
    
};



