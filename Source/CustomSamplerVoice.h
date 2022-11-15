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
    
    void prepareToPlay();
    
    void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample,
                                       int numSamples);
    
    void setFilter(juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float> , juce::dsp::IIR::Coefficients <float> >* filter);
    
    //Getters/Setters
    void setLevel(float level);
    
private:
    float level_;
    
    void UpdateVoice();
    
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float> , juce::dsp::IIR::Coefficients <float> >* filter_ = NULL;
    
};
