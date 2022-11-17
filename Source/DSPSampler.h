/*
  ==============================================================================

    DSPSampler.h
    Created: 14 Nov 2022 1:05:14pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include "DSPSynthesiser.h"

    
    //==============================================================================
    /**
     A subclass of DSPSynthesiserSound that represents a sampled audio clip.
     
     This is a pretty basic DSPSampler, and just attempts to load the whole audio stream
     into memory.
     
     To use it, create a DSPSynthesiser, add some DSPSamplerVoice objects to it, then
     give it some SampledSound objects to play.
     
     @see DSPSamplerVoice, DSPSynthesiser, DSPSynthesiserSound
     
     @tags{Audio}
     */


using namespace juce;

class DSPSamplerSound    : public DSPSynthesiserSound
{
public:
    //==============================================================================
    /** Creates a sampled sound from an audio reader.
     
     This will attempt to load the audio from the source into memory and store
     it in this object.
     
     @param name         a name for the sample
     @param source       the audio to load. This object can be safely deleted by the
     caller after this constructor returns
     @param midiNotes    the set of midi keys that this sound should be played on. This
     is used by the DSPSynthesiserSound::appliesToNote() method
     @param midiNoteForNormalPitch   the midi note at which the sample should be played
     with its natural rate. All other notes will be pitched
     up or down relative to this one
     @param attackTimeSecs   the attack (fade-in) time, in seconds
     @param releaseTimeSecs  the decay (fade-out) time, in seconds
     @param maxSampleLengthSeconds   a maximum length of audio to read from the audio
     source, in seconds
     */
    DSPSamplerSound (const String& name,
                  AudioFormatReader& source,
                  const BigInteger& midiNotes,
                  int midiNoteForNormalPitch,
                  double attackTimeSecs,
                  double releaseTimeSecs,
                  double maxSampleLengthSeconds);
    
    /** Destructor. */
    ~DSPSamplerSound() override;
    
    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const noexcept                  { return name; }
    
    /** Returns the audio sample data.
     This could return nullptr if there was a problem loading the data.
     */
    AudioBuffer<float>* getAudioData() const noexcept       { return data.get(); }
    
    //==============================================================================
    /** Changes the parameters of the ADSR envelope which will be applied to the sample. */
    void setEnvelopeParameters (ADSR::Parameters parametersToUse)    { params = parametersToUse; }
    
    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;
    
private:
    //==============================================================================
    friend class DSPSamplerVoice;
    
    String name;
    std::unique_ptr<AudioBuffer<float>> data;
    double sourceSamplerate;
    BigInteger midiNotes;
    int length = 0, midiRootNote = 0;
    
    ADSR::Parameters params;
    
    JUCE_LEAK_DETECTOR (DSPSamplerSound)
};


//==============================================================================
/**
 A subclass of DSPSynthesiserVoice that can play a DSPSamplerSound.
 
 To use it, create a DSPSynthesiser, add some DSPSamplerVoice objects to it, then
 give it some SampledSound objects to play.
 
 @see DSPSamplerSound, DSPSynthesiser, DSPSynthesiserVoice
 
 @tags{Audio}
 */
class DSPSamplerVoice    : public DSPSynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a DSPSamplerVoice. */
    DSPSamplerVoice();
    
    /** Destructor. */
    ~DSPSamplerVoice() override;
    
    //==============================================================================
    bool canPlaySound (DSPSynthesiserSound*) override;
    
    void startNote (int midiNoteNumber, float velocity, DSPSynthesiserSound*, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    
    void renderNextBlock (AudioBuffer<float>&, int startSample, int numSamples) override;
    
    void updateFilter(int frequency, float resonance);
    void setADSRParams(float attack, float sustain, float decay, float release);
    
    using DSPSynthesiserVoice::renderNextBlock;
    
private:
    //==============================================================================
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;
    
    ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    
    //Filter
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float> , juce::dsp::IIR::Coefficients <float> > lowPassFilter_;
    
    AudioBuffer<float> dspBuffer_;
    
    bool isPrepared {false};
    
    int sampleRate_ {44100};
        
    JUCE_LEAK_DETECTOR (DSPSamplerVoice)
};

