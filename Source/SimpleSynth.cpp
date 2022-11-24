/*
  ==============================================================================

    SimpleSynth.cpp
    Created: 17 Nov 2017 1:08:36pm
    Author:  Mael Derio

  ==============================================================================
*/


#include "SimpleSynth.h"
#define MAX_VOICES 16

//using namespace juce;

SimpleSynth::SimpleSynth() {
    setNoteStealingEnabled(false);
    
    const char* binaryData = 0;
    int binaryDataSize = 0;
    
    //TODO : better intitialization, with lower note, and centralized init
    int midiNote = 60;
    
    for (int i = 1; i < BinaryData::namedResourceListSize; ++i)
    {
        
        if(i<MAX_VOICES){
            binaryData = BinaryData::getNamedResource(BinaryData::namedResourceList[i], binaryDataSize);
            
            if(binaryData!=nullptr)
            {
                std::string filename = std::string(BinaryData::originalFilenames[i]);
                
                //Get extension
                int position = filename.find_last_of(".");
                std::string result = filename.substr(position+1);
                
                if(result == "wav")
                {
                    auto* inputStream = new MemoryInputStream (binaryData, binaryDataSize, false);
                    samplenameMap_[midiNote] = inputStream;
                    midiNote+=10;
                }
            }
        }
    }

    // add voices to our sampler
    for (int i = 0; i < MAX_VOICES; i++) {
        //addVoice(new CustomSamplerVoice());
        addVoice(new DSPSamplerVoice());
    }
    
    // set up our AudioFormatManager class as detailed in the API docs
    // we can now use WAV and AIFF files!
    
    audioFormatManager_.registerBasicFormats();

}


void SimpleSynth::setup() {
    
    clearSounds();
    
    for ( const auto& samplenamePair : samplenameMap_ )
    {
        //juce::String samplename = samplenamePair.second;
        juce::InputStream* sample = samplenamePair.second;
        int noteNumber = samplenamePair.first;
        
        //juce::File* file = new juce::File(samplename);
        //juce::ScopedPointer<juce::AudioFormatReader> reader = audioFormatManager_.createReaderFor(*file);
        
        juce::ScopedPointer<juce::AudioFormatReader> reader = audioFormatManager_.createReaderFor(std::unique_ptr <juce::InputStream> (sample));
        
        // allow our sound to be played on one note
        juce::BigInteger oneNote;
        oneNote.setRange(noteNumber, 1, true);
        
        // finally, add our sound
        //TODO : get the name and put it instead of anynameForNow
        addSound(new DSPSamplerSound("anynameForNow", *reader, oneNote, noteNumber, 0, 0.2, 2.0));
    }
}


void SimpleSynth::setSample(int noteNumber, juce::String name) {
    
    juce::File* file = new juce::File(name);
    auto* inputStream = new juce::FileInputStream (*file);
    samplenameMap_[noteNumber] = inputStream;
    
    // allow our sound to be played on one note
    juce::BigInteger oneNote;
    oneNote.setRange(noteNumber, 1, true);
    //juce::ScopedPointer<juce::AudioFormatReader> reader = audioFormatManager_.createReaderFor(*file);
    juce::ScopedPointer<juce::AudioFormatReader> reader = audioFormatManager_.createReaderFor(std::unique_ptr <juce::InputStream> (inputStream));
        
    int index = getSoundIndex(noteNumber);
    removeSound(index);
    
    //TODO : put the name
    //addSound(new DSPSamplerSound("anyname", *reader, oneNote, noteNumber, 0, 0.2, 2.0));
    addSound(new DSPSamplerSound("anyname", *reader, oneNote, noteNumber, 0.1f, 10.0f, 10.0f));

    
}

int SimpleSynth::getSoundIndex(int noteNumber)
{
    for (int kk=0;kk<sounds.size();kk++)
    {
        if(sounds[kk]->appliesToNote(noteNumber))
            return kk;
    }
    
    //If the note is not used, index is set to -1
    return -1;
}


void SimpleSynth::noteOn (const int midiChannel,
                          const int midiNoteNumber,
                          const float velocity)
{
    const juce::ScopedLock sl (lock);

    for (auto* sound : sounds)
    {
        if (sound->appliesToNote (midiNoteNumber) && sound->appliesToChannel (midiChannel))
        {
            // If hitting a note that's still ringing, stop it first (it could be
            // still playing because of the sustain or sostenuto pedal).
 
            for (auto* voice : voices)
            {
                if (voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel (midiChannel))
                {
                    stopVoice (voice, 1.0f, false);
                }
                
            }
            
            //auto voice = findFreeVoice (sound, midiChannel, midiNoteNumber, false);
            int voiceNumber = getVoiceNumber(midiNoteNumber);
            auto voice = getVoice(voiceNumber);
            
            startVoice (voice, sound, midiChannel, midiNoteNumber, velocity);
        }
    }
}

void SimpleSynth::renderVoices (AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    for (auto* voice : voices)
        voice->renderNextBlock (buffer, startSample, numSamples);
    
    
    std::cout<<"je suis là"<<std::endl;
}

void SimpleSynth::setVoiceLevel(float level, int row)
{
    //((CustomSamplerVoice*)voices[row])->setLevel(level);
}

//The voice that will play the sample is chosen in noteOn, via getVoiceNumber
int SimpleSynth::getVoiceNumber(int noteNumber)
{
    return (noteNumber - 60)/10;
}

/*
void SimpleSynth::setFilter(juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float> , juce::dsp::IIR::Coefficients <float> >* filter, DSPSynthesiserVoice* voice)
{
    voice->setFilter(filter);
}
 */

