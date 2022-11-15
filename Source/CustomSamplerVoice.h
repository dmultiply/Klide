/*
  ==============================================================================

    CustomSamplerVoice.h
    Created: 10 Dec 2017 6:51:35pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class CustomSamplerVoice : public juce::SamplerVoice {
public:
    CustomSamplerVoice();
    ~CustomSamplerVoice();
    
    void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample,
                                       int numSamples);
        
    //Getters/Setters
    void setLevel(float level);
    
private:
    float level_;
    
    void UpdateVoice();
};
