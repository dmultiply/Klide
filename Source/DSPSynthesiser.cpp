/*
  ==============================================================================

    DSPSynthesiser.cpp
    Created: 14 Nov 2022 12:23:06pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "DSPSynthesiser.h"


    
    DSPSynthesiserSound::DSPSynthesiserSound() {}
    DSPSynthesiserSound::~DSPSynthesiserSound() {}
    
    //==============================================================================
    DSPSynthesiserVoice::DSPSynthesiserVoice() {}
    DSPSynthesiserVoice::~DSPSynthesiserVoice() {}
    
    bool DSPSynthesiserVoice::isPlayingChannel (const int midiChannel) const
    {
        return currentPlayingMidiChannel == midiChannel;
    }
    
    void DSPSynthesiserVoice::setCurrentPlaybackSampleRate (const double newRate)
    {
        currentSampleRate = newRate;
    }
    
    bool DSPSynthesiserVoice::isVoiceActive() const
    {
        return getCurrentlyPlayingNote() >= 0;
    }
    
    void DSPSynthesiserVoice::clearCurrentNote()
    {
        currentlyPlayingNote = -1;
        currentlyPlayingSound = nullptr;
        currentPlayingMidiChannel = 0;
    }
    
    void DSPSynthesiserVoice::aftertouchChanged (int) {}
    void DSPSynthesiserVoice::channelPressureChanged (int) {}
    
    bool DSPSynthesiserVoice::wasStartedBefore (const DSPSynthesiserVoice& other) const noexcept
    {
        return noteOnTime < other.noteOnTime;
    }
    
    void DSPSynthesiserVoice::renderNextBlock (AudioBuffer<double>& outputBuffer,
                                            int startSample, int numSamples)
    {
        AudioBuffer<double> subBuffer (outputBuffer.getArrayOfWritePointers(),
                                       outputBuffer.getNumChannels(),
                                       startSample, numSamples);
        
        tempBuffer.makeCopyOf (subBuffer, true);
        renderNextBlock (tempBuffer, 0, numSamples);
        subBuffer.makeCopyOf (tempBuffer, true);
    }
    
    //==============================================================================
    DSPSynthesiser::DSPSynthesiser()
    {
        for (int i = 0; i < numElementsInArray (lastPitchWheelValues); ++i)
            lastPitchWheelValues[i] = 0x2000;
    }
    
    DSPSynthesiser::~DSPSynthesiser()
    {
    }
    
    //==============================================================================
    DSPSynthesiserVoice* DSPSynthesiser::getVoice (const int index) const
    {
        const ScopedLock sl (lock);
        return voices [index];
    }
    
    void DSPSynthesiser::clearVoices()
    {
        const ScopedLock sl (lock);
        voices.clear();
    }
    
    DSPSynthesiserVoice* DSPSynthesiser::addVoice (DSPSynthesiserVoice* const newVoice)
    {
        const ScopedLock sl (lock);
        newVoice->setCurrentPlaybackSampleRate (sampleRate);
        return voices.add (newVoice);
    }
    
    void DSPSynthesiser::removeVoice (const int index)
    {
        const ScopedLock sl (lock);
        voices.remove (index);
    }
    
    void DSPSynthesiser::clearSounds()
    {
        const ScopedLock sl (lock);
        sounds.clear();
    }
    
    DSPSynthesiserSound* DSPSynthesiser::addSound (const DSPSynthesiserSound::Ptr& newSound)
    {
        const ScopedLock sl (lock);
        return sounds.add (newSound);
    }
    
    void DSPSynthesiser::removeSound (const int index)
    {
        const ScopedLock sl (lock);
        sounds.remove (index);
    }
    
    void DSPSynthesiser::setNoteStealingEnabled (const bool shouldSteal)
    {
        shouldStealNotes = shouldSteal;
    }
    
    void DSPSynthesiser::setMinimumRenderingSubdivisionSize (int numSamples, bool shouldBeStrict) noexcept
    {
        jassert (numSamples > 0); // it wouldn't make much sense for this to be less than 1
        minimumSubBlockSize = numSamples;
        subBlockSubdivisionIsStrict = shouldBeStrict;
    }
    
    //==============================================================================
    void DSPSynthesiser::setCurrentPlaybackSampleRate (const double newRate)
    {
        if (sampleRate != newRate)
        {
            const ScopedLock sl (lock);
            allNotesOff (0, false);
            sampleRate = newRate;
            
            for (auto* voice : voices)
                voice->setCurrentPlaybackSampleRate (newRate);
        }
    }
    
    template <typename floatType>
    void DSPSynthesiser::processNextBlock (AudioBuffer<floatType>& outputAudio,
                                        const MidiBuffer& midiData,
                                        int startSample,
                                        int numSamples)
    {
        // must set the sample rate before using this!
        jassert (sampleRate != 0);
        const int targetChannels = outputAudio.getNumChannels();
        
        auto midiIterator = midiData.findNextSamplePosition (startSample);
        
        bool firstEvent = true;
        
        const ScopedLock sl (lock);
        
        for (; numSamples > 0; ++midiIterator)
        {
            if (midiIterator == midiData.cend())
            {
                if (targetChannels > 0)
                    renderVoices (outputAudio, startSample, numSamples);
                
                return;
            }
            
            const auto metadata = *midiIterator;
            const int samplesToNextMidiMessage = metadata.samplePosition - startSample;
            
            if (samplesToNextMidiMessage >= numSamples)
            {
                if (targetChannels > 0)
                    renderVoices (outputAudio, startSample, numSamples);
                
                handleMidiEvent (metadata.getMessage());
                break;
            }
            
            if (samplesToNextMidiMessage < ((firstEvent && ! subBlockSubdivisionIsStrict) ? 1 : minimumSubBlockSize))
            {
                handleMidiEvent (metadata.getMessage());
                continue;
            }
            
            firstEvent = false;
            
            if (targetChannels > 0)
                renderVoices (outputAudio, startSample, samplesToNextMidiMessage);
            
            handleMidiEvent (metadata.getMessage());
            startSample += samplesToNextMidiMessage;
            numSamples  -= samplesToNextMidiMessage;
        }
        
        std::for_each (midiIterator,
                       midiData.cend(),
                       [&] (const MidiMessageMetadata& meta) { handleMidiEvent (meta.getMessage()); });
    }
    
    // explicit template instantiation
    template void DSPSynthesiser::processNextBlock<float>  (AudioBuffer<float>&,  const MidiBuffer&, int, int);
    template void DSPSynthesiser::processNextBlock<double> (AudioBuffer<double>&, const MidiBuffer&, int, int);
    
    void DSPSynthesiser::renderNextBlock (AudioBuffer<float>& outputAudio, const MidiBuffer& inputMidi,
                                       int startSample, int numSamples)
    {
        processNextBlock (outputAudio, inputMidi, startSample, numSamples);
    }
    
    void DSPSynthesiser::renderNextBlock (AudioBuffer<double>& outputAudio, const MidiBuffer& inputMidi,
                                       int startSample, int numSamples)
    {
        processNextBlock (outputAudio, inputMidi, startSample, numSamples);
    }
    
    void DSPSynthesiser::renderVoices (AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        for (auto* voice : voices)
            voice->renderNextBlock (buffer, startSample, numSamples);
    }
    
    void DSPSynthesiser::renderVoices (AudioBuffer<double>& buffer, int startSample, int numSamples)
    {
        for (auto* voice : voices)
            voice->renderNextBlock (buffer, startSample, numSamples);
    }
    
    void DSPSynthesiser::handleMidiEvent (const MidiMessage& m)
    {
        const int channel = m.getChannel();
        
        if (m.isNoteOn())
        {
            noteOn (channel, m.getNoteNumber(), m.getFloatVelocity());
        }
        else if (m.isNoteOff())
        {
            noteOff (channel, m.getNoteNumber(), m.getFloatVelocity(), true);
        }
        else if (m.isAllNotesOff() || m.isAllSoundOff())
        {
            allNotesOff (channel, true);
        }
        else if (m.isPitchWheel())
        {
            const int wheelPos = m.getPitchWheelValue();
            lastPitchWheelValues [channel - 1] = wheelPos;
            handlePitchWheel (channel, wheelPos);
        }
        else if (m.isAftertouch())
        {
            handleAftertouch (channel, m.getNoteNumber(), m.getAfterTouchValue());
        }
        else if (m.isChannelPressure())
        {
            handleChannelPressure (channel, m.getChannelPressureValue());
        }
        else if (m.isController())
        {
            handleController (channel, m.getControllerNumber(), m.getControllerValue());
        }
        else if (m.isProgramChange())
        {
            handleProgramChange (channel, m.getProgramChangeNumber());
        }
    }
    
    //==============================================================================
    void DSPSynthesiser::noteOn (const int midiChannel,
                              const int midiNoteNumber,
                              const float velocity)
    {
        const ScopedLock sl (lock);
        
        for (auto* sound : sounds)
        {
            if (sound->appliesToNote (midiNoteNumber) && sound->appliesToChannel (midiChannel))
            {
                // If hitting a note that's still ringing, stop it first (it could be
                // still playing because of the sustain or sostenuto pedal).
                for (auto* voice : voices)
                    if (voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel (midiChannel))
                        stopVoice (voice, 1.0f, true);
                
                startVoice (findFreeVoice (sound, midiChannel, midiNoteNumber, shouldStealNotes),
                            sound, midiChannel, midiNoteNumber, velocity);
            }
        }
    }
    
    void DSPSynthesiser::startVoice (DSPSynthesiserVoice* const voice,
                                  DSPSynthesiserSound* const sound,
                                  const int midiChannel,
                                  const int midiNoteNumber,
                                  const float velocity)
    {
        if (voice != nullptr && sound != nullptr)
        {
            if (voice->currentlyPlayingSound != nullptr)
                voice->stopNote (0.0f, false);
            
            voice->currentlyPlayingNote = midiNoteNumber;
            voice->currentPlayingMidiChannel = midiChannel;
            voice->noteOnTime = ++lastNoteOnCounter;
            voice->currentlyPlayingSound = sound;
            voice->setKeyDown (true);
            voice->setSostenutoPedalDown (false);
            voice->setSustainPedalDown (sustainPedalsDown[midiChannel]);
            
            voice->startNote (midiNoteNumber, velocity, sound,
                              lastPitchWheelValues [midiChannel - 1]);
        }
    }
    
    void DSPSynthesiser::stopVoice (DSPSynthesiserVoice* voice, float velocity, const bool allowTailOff)
    {
        jassert (voice != nullptr);
        
        voice->stopNote (velocity, allowTailOff);
        
        // the subclass MUST call clearCurrentNote() if it's not tailing off! RTFM for stopNote()!
        jassert (allowTailOff || (voice->getCurrentlyPlayingNote() < 0 && voice->getCurrentlyPlayingSound() == nullptr));
    }
    
    void DSPSynthesiser::noteOff (const int midiChannel,
                               const int midiNoteNumber,
                               const float velocity,
                               const bool allowTailOff)
    {
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
        {
            if (voice->getCurrentlyPlayingNote() == midiNoteNumber
                && voice->isPlayingChannel (midiChannel))
            {
                if (auto sound = voice->getCurrentlyPlayingSound())
                {
                    if (sound->appliesToNote (midiNoteNumber)
                        && sound->appliesToChannel (midiChannel))
                    {
                        jassert (! voice->keyIsDown || voice->isSustainPedalDown() == sustainPedalsDown [midiChannel]);
                        
                        voice->setKeyDown (false);
                        
                        if (! (voice->isSustainPedalDown() || voice->isSostenutoPedalDown()))
                            stopVoice (voice, velocity, allowTailOff);
                    }
                }
            }
        }
    }
    
    void DSPSynthesiser::allNotesOff (const int midiChannel, const bool allowTailOff)
    {
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
            if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
                voice->stopNote (1.0f, allowTailOff);
        
        sustainPedalsDown.clear();
    }
    
    void DSPSynthesiser::handlePitchWheel (const int midiChannel, const int wheelValue)
    {
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
            if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
                voice->pitchWheelMoved (wheelValue);
    }
    
    void DSPSynthesiser::handleController (const int midiChannel,
                                        const int controllerNumber,
                                        const int controllerValue)
    {
        switch (controllerNumber)
        {
            case 0x40:  handleSustainPedal   (midiChannel, controllerValue >= 64); break;
            case 0x42:  handleSostenutoPedal (midiChannel, controllerValue >= 64); break;
            case 0x43:  handleSoftPedal      (midiChannel, controllerValue >= 64); break;
            default:    break;
        }
        
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
            if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
                voice->controllerMoved (controllerNumber, controllerValue);
    }
    
    void DSPSynthesiser::handleAftertouch (int midiChannel, int midiNoteNumber, int aftertouchValue)
    {
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
            if (voice->getCurrentlyPlayingNote() == midiNoteNumber
                && (midiChannel <= 0 || voice->isPlayingChannel (midiChannel)))
                voice->aftertouchChanged (aftertouchValue);
    }
    
    void DSPSynthesiser::handleChannelPressure (int midiChannel, int channelPressureValue)
    {
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
            if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
                voice->channelPressureChanged (channelPressureValue);
    }
    
    void DSPSynthesiser::handleSustainPedal (int midiChannel, bool isDown)
    {
        jassert (midiChannel > 0 && midiChannel <= 16);
        const ScopedLock sl (lock);
        
        if (isDown)
        {
            sustainPedalsDown.setBit (midiChannel);
            
            for (auto* voice : voices)
                if (voice->isPlayingChannel (midiChannel) && voice->isKeyDown())
                    voice->setSustainPedalDown (true);
        }
        else
        {
            for (auto* voice : voices)
            {
                if (voice->isPlayingChannel (midiChannel))
                {
                    voice->setSustainPedalDown (false);
                    
                    if (! (voice->isKeyDown() || voice->isSostenutoPedalDown()))
                        stopVoice (voice, 1.0f, true);
                }
            }
            
            sustainPedalsDown.clearBit (midiChannel);
        }
    }
    
    void DSPSynthesiser::handleSostenutoPedal (int midiChannel, bool isDown)
    {
        jassert (midiChannel > 0 && midiChannel <= 16);
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
        {
            if (voice->isPlayingChannel (midiChannel))
            {
                if (isDown)
                    voice->setSostenutoPedalDown (true);
                else if (voice->isSostenutoPedalDown())
                    stopVoice (voice, 1.0f, true);
            }
        }
    }
    
    void DSPSynthesiser::handleSoftPedal (int midiChannel, bool /*isDown*/)
    {
        ignoreUnused (midiChannel);
        jassert (midiChannel > 0 && midiChannel <= 16);
    }
    
    void DSPSynthesiser::handleProgramChange (int midiChannel, int programNumber)
    {
        ignoreUnused (midiChannel, programNumber);
        jassert (midiChannel > 0 && midiChannel <= 16);
    }
    
    //==============================================================================
    DSPSynthesiserVoice* DSPSynthesiser::findFreeVoice (DSPSynthesiserSound* soundToPlay,
                                                  int midiChannel, int midiNoteNumber,
                                                  const bool stealIfNoneAvailable) const
    {
        const ScopedLock sl (lock);
        
        for (auto* voice : voices)
            if ((! voice->isVoiceActive()) && voice->canPlaySound (soundToPlay))
                return voice;
        
        if (stealIfNoneAvailable)
            return findVoiceToSteal (soundToPlay, midiChannel, midiNoteNumber);
        
        return nullptr;
    }
    
    DSPSynthesiserVoice* DSPSynthesiser::findVoiceToSteal (DSPSynthesiserSound* soundToPlay,
                                                     int /*midiChannel*/, int midiNoteNumber) const
    {
        // This voice-stealing algorithm applies the following heuristics:
        // - Re-use the oldest notes first
        // - Protect the lowest & topmost notes, even if sustained, but not if they've been released.
        
        // apparently you are trying to render audio without having any voices...
        jassert (! voices.isEmpty());
        
        // These are the voices we want to protect (ie: only steal if unavoidable)
        DSPSynthesiserVoice* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
        DSPSynthesiserVoice* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase
        
        // this is a list of voices we can steal, sorted by how long they've been running
        Array<DSPSynthesiserVoice*> usableVoices;
        usableVoices.ensureStorageAllocated (voices.size());
        
        for (auto* voice : voices)
        {
            if (voice->canPlaySound (soundToPlay))
            {
                jassert (voice->isVoiceActive()); // We wouldn't be here otherwise
                
                usableVoices.add (voice);
                
                // NB: Using a functor rather than a lambda here due to scare-stories about
                // compilers generating code containing heap allocations..
                struct Sorter
                {
                    bool operator() (const DSPSynthesiserVoice* a, const DSPSynthesiserVoice* b) const noexcept { return a->wasStartedBefore (*b); }
                };
                
                std::sort (usableVoices.begin(), usableVoices.end(), Sorter());
                
                if (! voice->isPlayingButReleased()) // Don't protect released notes
                {
                    auto note = voice->getCurrentlyPlayingNote();
                    
                    if (low == nullptr || note < low->getCurrentlyPlayingNote())
                        low = voice;
                    
                    if (top == nullptr || note > top->getCurrentlyPlayingNote())
                        top = voice;
                }
            }
        }
        
        // Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
        if (top == low)
            top = nullptr;
        
        // The oldest note that's playing with the target pitch is ideal..
        for (auto* voice : usableVoices)
            if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
                return voice;
        
        // Oldest voice that has been released (no finger on it and not held by sustain pedal)
        for (auto* voice : usableVoices)
            if (voice != low && voice != top && voice->isPlayingButReleased())
                return voice;
        
        // Oldest voice that doesn't have a finger on it:
        for (auto* voice : usableVoices)
            if (voice != low && voice != top && ! voice->isKeyDown())
                return voice;
        
        // Oldest voice that isn't protected
        for (auto* voice : usableVoices)
            if (voice != low && voice != top)
                return voice;
        
        // We've only got "protected" voices now: lowest note takes priority
        jassert (low != nullptr);
        
        // Duophonic synth: give priority to the bass note:
        if (top != nullptr)
            return top;
        
        return low;
    }

    
