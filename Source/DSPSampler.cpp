/*
  ==============================================================================

    DSPDSPSampler.cpp
    Created: 14 Nov 2022 1:05:14pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "DSPSampler.h"


    
DSPSamplerSound::DSPSamplerSound (const String& soundName,
                            AudioFormatReader& source,
                            const BigInteger& notes,
                            int midiNoteForNormalPitch,
                            double attackTimeSecs,
                            double releaseTimeSecs,
                            double maxSampleLengthSeconds)
: name (soundName),
sourceSamplerate (source.sampleRate),
midiNotes (notes),
midiRootNote (midiNoteForNormalPitch)
{
    if (sourceSamplerate > 0 && source.lengthInSamples > 0)
    {
        length = jmin ((int) source.lengthInSamples,
                       (int) (maxSampleLengthSeconds * sourceSamplerate));
        
        data.reset (new AudioBuffer<float> (jmin (2, (int) source.numChannels), length + 4));
        
        source.read (data.get(), 0, length + 4, 0, true, true);
        
        params.attack  = static_cast<float> (attackTimeSecs);
        params.release = static_cast<float> (releaseTimeSecs);
    }
}

DSPSamplerSound::~DSPSamplerSound()
{
}

bool DSPSamplerSound::appliesToNote (int midiNoteNumber)
{
    return midiNotes[midiNoteNumber];
}

bool DSPSamplerSound::appliesToChannel (int /*midiChannel*/)
{
    return true;
}

//==============================================================================
DSPSamplerVoice::DSPSamplerVoice() {}
DSPSamplerVoice::~DSPSamplerVoice() {}

bool DSPSamplerVoice::canPlaySound (DSPSynthesiserSound* sound)
{
    return dynamic_cast<const DSPSamplerSound*> (sound) != nullptr;
}

void DSPSamplerVoice::startNote (int midiNoteNumber, float velocity, DSPSynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    if (auto* sound = dynamic_cast<const DSPSamplerSound*> (s))
    {
        pitchRatio = std::pow (2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
        * sound->sourceSamplerate / getSampleRate();
        
        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;
        
        adsr.setSampleRate (sound->sourceSamplerate);
        //adsr.setParameters (sound->params);
        adsr.setParameters (adsrParams);
        
        adsr.noteOn();
    }
    else
    {
        jassertfalse; // this object can only play DSPSamplerSounds!
    }
}

void DSPSamplerVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void DSPSamplerVoice::pitchWheelMoved (int /*newValue*/) {}
void DSPSamplerVoice::controllerMoved (int /*controllerNumber*/, int /*newValue*/) {}

void DSPSamplerVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels)
{
    
    //SampleRate
    sampleRate_ = sampleRate;
    
    //ADSR
    adsr.setSampleRate(sampleRate);
    
    //Filter
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = outputChannels;
    
    lowPassFilter_.prepare(spec);
    lowPassFilter_.reset();
    
    dspBuffer_.setSize(outputChannels, samplesPerBlock);
    dspBuffer_.clear();
    
    //Bool to check if the prepareToPlay was called
    isPrepared = true;
}

void DSPSamplerVoice::setADSRParams(float attack, float decay, float sustain, float release)
{
    adsrParams.attack = attack;
    adsrParams.decay = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;

}

juce::ADSR::Parameters DSPSamplerVoice::getADSRParams()
{
    return adsrParams;
}


void DSPSamplerVoice::updateFilter(int frequency, float resonance)
{
    *lowPassFilter_.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, frequency, resonance);
}

void DSPSamplerVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    jassert(isPrepared);
    
    int numSamplesMem = numSamples;
    
    if (auto* playingSound = static_cast<DSPSamplerSound*> (getCurrentlyPlayingSound().get()))
    {

        
        auto& data = *playingSound->data;
        
        const float* const inL = data.getReadPointer (0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer (1) : nullptr;
        
        float* outL = outputBuffer.getWritePointer (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;
        
        //Compute length of buffer
        int endBuffer = 0;
        
        if((sourceSamplePosition + numSamples) <= playingSound->length)
            endBuffer = numSamples;
        else
            endBuffer = data.getNumSamples()-sourceSamplePosition;
        
       
        //AudioBuffer<float> copyBuffer;
        if(dspBuffer_.getNumChannels() != data.getNumChannels() || dspBuffer_.getNumSamples() != endBuffer){
            dspBuffer_.setSize(data.getNumChannels(),endBuffer);
            dspBuffer_.clear();
        }
        
        for (int i = 0; i<data.getNumChannels();i++) {
            dspBuffer_.copyFrom (i, 0, data, i, sourceSamplePosition, endBuffer);
        }
        
        juce::dsp::AudioBlock<float> block(dspBuffer_);
        lowPassFilter_.process(juce::dsp::ProcessContextReplacing<float> (block));
        
        const float* const dspL = dspBuffer_.getReadPointer (0);
        const float* const dspR = dspBuffer_.getNumChannels() > 1 ? dspBuffer_.getReadPointer (1) : nullptr;
        
        int dspBufferSamplePosition = 0;
        
        while (--numSamples >= 0)
        {
            //auto pos = (int) sourceSamplePosition;
            auto pos = (int) dspBufferSamplePosition;
            auto alpha = (float) (dspBufferSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;
            
            // just using a very simple linear interpolation here..
            float l = (dspL[pos] * invAlpha + dspL[pos + 1] * alpha);
            float r = (dspR != nullptr) ? (dspR[pos] * invAlpha + dspR[pos + 1] * alpha) : l;
            
            auto envelopeValue = adsr.getNextSample();
            
            l *= lgain * envelopeValue;
            r *= rgain * envelopeValue;
            
            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }
            
            sourceSamplePosition += pitchRatio;
            dspBufferSamplePosition += pitchRatio;
            
            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (0.0f, false);
                break;
            }
        }
        
    }//end if
}
    

