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
        adsr.setParameters (sound->params);
        
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


void DSPSamplerVoice::setFilter(juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float> , juce::dsp::IIR::Coefficients <float> >* filter)
{
    filter_ = filter;
}


//==============================================================================
void DSPSamplerVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
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
        
        
        //AudioBuffer<float> dspBuffer;
        //dspBuffer.clear();
        //dspBuffer.setSize(2,endBuffer);
        AudioBuffer<float> copyBuffer;
        copyBuffer.makeCopyOf(data);
        //dspBuffer.copyFrom(0, 0, data, 0, sourceSamplePosition, endBuffer);
        //dspBuffer.copyFrom(0, 0, data, 1, sourceSamplePosition, endBuffer);
        
        //dspBuffer.copyFrom(0, sourceSamplePosition, inL, endBuffer);
        //dspBuffer.copyFrom(1, sourceSamplePosition, inR, endBuffer);

        AudioBuffer<float> dspBuffer(copyBuffer.getArrayOfWritePointers(), copyBuffer.getNumChannels(), sourceSamplePosition, endBuffer);
        //dspBuffer.clear();
        //dspBuffer.setSize(2, 512);
        //dspBuffer.copyFrom(0, 0, data, 0, sourceSamplePosition, endBuffer);
        //dspBuffer.copyFrom(1, 0, data, 1, sourceSamplePosition, endBuffer);
        
        juce::dsp::AudioBlock<float> block(dspBuffer);
        filter_->process(juce::dsp::ProcessContextReplacing<float> (block));
        
        //data.copyFrom(0, sourceSamplePosition, dspBuffer, 0, 0, endBuffer);
        //data.copyFrom(1, sourceSamplePosition, dspBuffer, 1, 0, endBuffer);
        
        const float* const dspL = copyBuffer.getReadPointer (0);
        const float* const dspR = copyBuffer.getNumChannels() > 1 ? copyBuffer.getReadPointer (1) : nullptr;
        
        int dspBufferSamplePosition = 0;
        
        while (--numSamples >= 0)
        {
            auto pos = (int) sourceSamplePosition;
            auto alpha = (float) (sourceSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;
            
            //auto pos = (int) dspBufferSamplePosition;
            //auto alpha = (float) (dspBufferSamplePosition - pos);
            //auto invAlpha = 1.0f - alpha;
            
            // just using a very simple linear interpolation here..
            float l = (dspL[pos] * invAlpha + dspL[pos + 1] * alpha);
            float r = (dspR != nullptr) ? (dspR[pos] * invAlpha + dspR[pos + 1] * alpha) : l;
            
            //float l = dspL[pos];
            //float r = dspR[pos];
            
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
            
            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (0.0f, false);
                break;
            }
        }
        
    }
}
    

