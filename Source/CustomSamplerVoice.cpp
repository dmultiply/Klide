/*
  ==============================================================================

    CustomSamplerVoice.cpp
    Created: 10 Dec 2017 6:51:35pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "CustomSamplerVoice.h"

using namespace juce;

CustomSamplerVoice::CustomSamplerVoice():level_(-20)
{
   
}

CustomSamplerVoice::~CustomSamplerVoice()
{
    
}

void CustomSamplerVoice::renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample,
                     int numSamples)
{
/*
    
    if (auto* playingSound = static_cast<SamplerSound*> (getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->getAudioData();
        const float* const inL = data.getReadPointer (0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer (1) : nullptr;
        
        float* outL = outputBuffer.getWritePointer (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;
        
        while (--numSamples >= 0)
        {
            //auto pos = (int) sourceSamplePosition;
            //auto alpha = (float) (sourceSamplePosition - pos);
            //auto invAlpha = 1.0f - alpha;
            
            // just using a very simple linear interpolation here..
            //float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            //float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
            //: l;
            
            //auto envelopeValue = adsr.getNextSample();
            
            //l *= lgain * envelopeValue;
            //r *= rgain * envelopeValue;
            
            if (outR != nullptr)
            {
                //*outL++ += l;
                //*outR++ += r;
            }
            else
            {
                //*outL++ += (l + r) * 0.5f;
            }
            
            //sourceSamplePosition += pitchRatio;
            
            /*
            if (sourceSamplePosition > playingSound->getLength())
            {
                stopNote (0.0f, false);
                break;
            }
 
        }
    }
    
    //SamplerVoice::renderNextBlock(outputBuffer, startSample, numSamples);
    ////outputBuffer.applyGain(0, outputBuffer.getNumSamples(), juce::Decibels::decibelsToGain(level_));
   */
}

void CustomSamplerVoice::setLevel(float level)
{
    level_ = level;
}

