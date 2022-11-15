/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KlideAudioProcessor::KlideAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
sampleRate_ (44100.0),
syncOn_(0), lowPassFilter_(juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, 6000.0f, 0.1f))
#endif
{
    
    
    gainVec_ = {0.6f,0.7f,1.0f,0.8f};
    
    info_ = "nothing";
    
    //Start the synth
    synth_.setup();
    
    //Initialize StepSequencerData
    stepData_ = new StepSequencerData();
    
    for(int kk=0;kk<stepData_->getNumRows();kk++) {
        lowPassFilterVec_.push_back(juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, 6000.0f, 0.1f));
    }
    
}

KlideAudioProcessor::~KlideAudioProcessor()
{
}

//==============================================================================
const juce::String KlideAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KlideAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KlideAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KlideAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KlideAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KlideAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KlideAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KlideAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KlideAudioProcessor::getProgramName (int index)
{
    return {};
}

void KlideAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KlideAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    synth_.setCurrentPlaybackSampleRate(sampleRate);
    
    //Filter
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    lowPassFilter_.prepare(spec);
    lowPassFilter_.reset();
    
    for(int kk=0;kk<lowPassFilterVec_.size();kk++) {
        lowPassFilterVec_[kk].prepare(spec);
        lowPassFilterVec_[kk].reset();
    }
}

void KlideAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KlideAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void KlideAudioProcessor::updateFilter()
{
    *lowPassFilter_.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, stepData_->getFrequencyVec()[0], stepData_->getResonanceVec()[0]);
    
    for(int kk=0;kk<lowPassFilterVec_.size();kk++) {
        *lowPassFilterVec_[kk].state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, stepData_->getFrequencyVec()[kk], stepData_->getResonanceVec()[kk]);
    }
    
    //std::cout<<"freq : "<<stepData_->getFrequencyVec()[0]<<"res : "<<stepData_->getResonanceVec()[0]<<std::endl;
}

void KlideAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    /*
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
     */
    
    if(stepData_ != nullptr && stepData_->isPlaying())
    {
        const double currentTime = juce::Time::getMillisecondCounterHiRes() * 0.001 - stepData_->getStartTime();
        
        
        if(syncOn_)
        {
            juce::AudioPlayHead::CurrentPositionInfo currentPosition;
            getPlayHead()->getCurrentPosition(currentPosition);
            
            double timeInSeconds = currentPosition.timeInSeconds;
            double syncTimeDiff = currentTime - timeInSeconds;
            int bpm = currentPosition.bpm;
            //timeSigDenominator_ = currentPosition_.timeSigDenominator;
            //timeSigNumerator_ = currentPosition_.timeSigNumerator;
            
            //Update the bpm
            stepData_->setBpm(bpm);
            
            for(int row = 0;row<stepData_->getNumRows();row++)
            {
                if(stepData_->getPlay(row) == 1)
                {
                    //Change the duration of the intervals (because bpm may have changed with sync)
                    double duration = stepData_->computeStepDuration(row);
                    stepData_->setStepTime(row, duration);
                    
                    //int absoluteStep = stepData_->computeAbsoluteStep(row);
                    //int currentStep = (absoluteStep)%stepData_->getInterval(row);
                    //float currentTimestamp = stepData_->getTimeStamp()+(float)absoluteStep*stepData_->getStepTime(row);
                    float timeStamp = computeNextBarSyncTime(currentPosition,syncTimeDiff);
                    stepData_->setTimeStamp(timeStamp);
                    stepData_->setGlobalStep(row, -1);
                    stepData_->createNextStep(row, 0,timeStamp);
                }
            }
            
            syncOn_ = 0;
        }
        
        
        
        const int currentSampleNumber =  (int) (currentTime * sampleRate_);
        
        for(int row = 0;row<stepData_->getNumRows();row++)
        {
            if(stepData_->getPlay(row) == 1)
            {
                if(stepData_->getCurrentNoteTime(row)<= currentTime)
                {
                    
                    int noteNumber = stepData_->getCurrentNote(row);
                    
                    if(noteNumber>0) //TODO : assertion < 128 ?
                    {
                        //synth_.noteOn(1, noteNumber, gainVec_[row]);
                        synth_.noteOn(1, noteNumber, stepData_->getGain(row));
                        
                        //Added in 2022
                        //Add midi output
                        auto message = juce::MidiMessage::noteOn (1, noteNumber, juce::uint8(100));
                        midiMessages.addEvent(message, currentSampleNumber);
                        
                        //int voiceNumber = synth_.getVoiceNumber(noteNumber);
                        //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
                        //synth_.getVoice(voiceNumber)->renderNextBlock(buffer, 0, buffer.getNumSamples());
                        
                    }
                    stepData_->next(row);
                }
                //synth_.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
            }
        }
        
        int voiceNumber = synth_.getVoiceNumber(60);
        //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
        voiceNumber = synth_.getVoiceNumber(70);
        //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
        voiceNumber = synth_.getVoiceNumber(80);
        //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
        voiceNumber = synth_.getVoiceNumber(90);
        //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
        
        
        
        updateFilter();
        //synth_.setFilter(&lowPassFilter_, synth_.getVoice(synth_.getVoiceNumber(60)));
        synth_.setFilter(&lowPassFilterVec_[0], synth_.getVoice(synth_.getVoiceNumber(60)));
        synth_.setFilter(&lowPassFilterVec_[1], synth_.getVoice(synth_.getVoiceNumber(70)));
        synth_.setFilter(&lowPassFilterVec_[2], synth_.getVoice(synth_.getVoiceNumber(80)));
        synth_.setFilter(&lowPassFilterVec_[3], synth_.getVoice(synth_.getVoiceNumber(90)));
        
        
        //juce::dsp::AudioBlock<float> block1(buffer);
        
        
        synth_.getVoice(synth_.getVoiceNumber(60))->renderNextBlock(buffer, 0, buffer.getNumSamples());
        //lowPassFilter_.process(juce::dsp::ProcessContextReplacing<float> (block1));
        
        synth_.getVoice(synth_.getVoiceNumber(70))->renderNextBlock(buffer, 0, buffer.getNumSamples());
        synth_.getVoice(synth_.getVoiceNumber(80))->renderNextBlock(buffer, 0, buffer.getNumSamples());
        synth_.getVoice(synth_.getVoiceNumber(90))->renderNextBlock(buffer, 0, buffer.getNumSamples());
        
        
        //synth_.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    }
}

double KlideAudioProcessor::computeNextBarSyncTime(juce::AudioPlayHead::CurrentPositionInfo currentPosition, double syncTimeDiff)
{
    int bpm = currentPosition.bpm;
    int timeSigDenominator = currentPosition.timeSigDenominator;
    int timeSigNumerator = currentPosition.timeSigNumerator;
    double ppqPositionOfLastBarStart = currentPosition.ppqPositionOfLastBarStart;
    double ppqPosition = currentPosition.ppqPosition;
    double editOriginTime = currentPosition.editOriginTime;
    double timeInSeconds = currentPosition.timeInSeconds;
    
    //bpm = number of quarter notes per 60s
    //for 1s :
    float numberQuarterNotesPerSec = bpm/60;
    //number of seconds per quarter note
    float numberSecondsPerQuarterNote = 1/numberQuarterNotesPerSec;
    
    double numberOfQuarterNotes = ((double)timeSigNumerator/(double)timeSigDenominator)*4.0;
    double ppqOfNextBar = ppqPositionOfLastBarStart + numberOfQuarterNotes;
    double nextBarHostTimeSeconds = (60.0/(double)bpm)*ppqOfNextBar;
    double nextBarLocalTimeSeconds = nextBarHostTimeSeconds + syncTimeDiff;
    
    return nextBarLocalTimeSeconds;
    
}

//==============================================================================
bool KlideAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KlideAudioProcessor::createEditor()
{
    return new KlideAudioProcessorEditor (*this);
}

//==============================================================================
void KlideAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void KlideAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KlideAudioProcessor();
}

SimpleSynth* KlideAudioProcessor::getSynth() {
    return &synth_;
}

//Getters/Setters
void KlideAudioProcessor::setGain(float level, int row)
{
    if(gainVec_.size()>=row+1)
        gainVec_[row] = level;
}

void KlideAudioProcessor::setSyncOn(bool syncOn)
{
    syncOn_ = syncOn;
}

juce::String KlideAudioProcessor::getInfo()
{
    return info_;
}

//void PoLyrplugAudioProcessor::setStepData(StepSequencerData *stepData)
//{
//    stepData_ = stepData;
//}

StepSequencerData* KlideAudioProcessor::getStepData()
{
    return stepData_;
}





