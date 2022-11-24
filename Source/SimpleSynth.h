/*
  ==============================================================================

    SimpleSynth.h
    Created: 17 Nov 2017 1:08:36pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once
#include <string>
#include <map>
#include "../JuceLibraryCode/JuceHeader.h"
#include "DSPSynthesiser.h"
#include "DSPSampler.h"

class SimpleSynth : public DSPSynthesiser {
public:
    SimpleSynth();
    void setup();
    
    void noteOn (const int midiChannel, const int midiNoteNumber, const float velocity);
    
    void setVoiceLevel(float level, int row);
    int getVoiceNumber(int noteNumber);
    void setSample(int noteNumber, juce::String name);
    int getSoundIndex(int noteNumber);
    
    void renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    //void setFilter(juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float> , juce::dsp::IIR::Coefficients <float> >* filter, DSPSynthesiserVoice* voice);
    
private:
    // manager object that finds an appropriate way to decode various audio files.  Used with SampleSound objects.

    juce::AudioFormatManager audioFormatManager_;
    //std::map<int,juce::String> samplenameMap_;
    std::map<int,juce::InputStream*> samplenameMap_;
    
};
