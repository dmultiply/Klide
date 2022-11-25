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
                       ), tree_(*this, nullptr, "Parameters", createParameters()),

sampleRate_ (44100.0),
syncOn_(0)
#endif
{
    
    
    gainVec_ = {0.6f,0.7f,1.0f,0.8f};
    
    info_ = "nothing";
    
    //Start the synth
    synth_.setup();
    
    //Initialize StepSequencerData
    stepData_ = new StepSequencerData();
       
}

juce::AudioProcessorValueTreeState::ParameterLayout KlideAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterInt>("ROWCHOICE", "Rowchoice", 0,3,0));
    
    for(int row = 0;row<4;row++)
    {
        //ADSR
        params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK"+std::to_string(row), "Attack"+std::to_string(row), 0.0f,1.0f,0.01f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("DECAY"+std::to_string(row), "Decay"+std::to_string(row), 0.0f,2.0f,0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN"+std::to_string(row), "Sustain"+std::to_string(row), 0.0f,2.5f,1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE"+std::to_string(row), "Release"+std::to_string(row), 0.0f,8.0f,0.5f));
        
        //Pan slider can only take positive values, so we put values from 0 to 2 and we remove 1 to the value at update
        params.push_back(std::make_unique<juce::AudioParameterFloat>("PAN"+std::to_string(row), "Pan"+std::to_string(row), 0.0f,2.0f,1.0f));
        
        //Euclidean Parameters
        params.push_back(std::make_unique<juce::AudioParameterInt>("INTERVAL"+std::to_string(row), "Interval"+std::to_string(row), 4,16,8));
        params.push_back(std::make_unique<juce::AudioParameterInt>("PULSE"+std::to_string(row), "Pulse"+std::to_string(row), 0,8,4-row));
        params.push_back(std::make_unique<juce::AudioParameterInt>("OFFSET"+std::to_string(row), "Offset"+std::to_string(row), 0,4,3-row));
        
        //Gain
        params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN"+std::to_string(row), "Gain"+std::to_string(row), 0.0f,1.0f,0.5f));
        
        //Filter
        params.push_back(std::make_unique<juce::AudioParameterInt>("FREQUENCY"+std::to_string(row), "Frequency"+std::to_string(row), 20,20000,6000));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("RESONANCE"+std::to_string(row), "Resonance"+std::to_string(row), 0.0f,2.0f,0.5f));
    }
    
    return {params.begin(),params.end()};
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
    
    //ADSR Params
    
    /*
    auto rowChoice = tree_.getRawParameterValue("ROWCHOICE");
    
    auto attack = tree_.getRawParameterValue("ATTACK"+std::to_string(int(rowChoice->load())));
    auto decay = tree_.getRawParameterValue("DECAY"+std::to_string(int(rowChoice->load())));
    auto sustain = tree_.getRawParameterValue("SUSTAIN"+std::to_string(int(rowChoice->load())));
    auto release = tree_.getRawParameterValue("RELEASE"+std::to_string(int(rowChoice->load())));
     */
    
    
    
    //Prepare the filtering in each voice of the synth
    //Set ADSR and pan
    for (int i = 0; i<stepData_->getNumRows();i++) {
        
        auto attack = tree_.getRawParameterValue("ATTACK"+std::to_string(i));
        auto decay = tree_.getRawParameterValue("DECAY"+std::to_string(i));
        auto sustain = tree_.getRawParameterValue("SUSTAIN"+std::to_string(i));
        auto release = tree_.getRawParameterValue("RELEASE"+std::to_string(i));
        
        //Pan
        auto pan = tree_.getRawParameterValue("PAN"+std::to_string(i));
        
        if(auto voice = dynamic_cast<DSPSamplerVoice*>(synth_.getVoice(i)))
        {
            voice->prepareToPlay(sampleRate,samplesPerBlock,getTotalNumOutputChannels());
            voice->setADSRParams(attack->load(),decay->load(),sustain->load(),release->load());
            //Pan slider can only take positive values, so we remove 1 to the value at update
            voice->updatePan(pan->load()-1);
        }
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

void KlideAudioProcessor::updateADSRParams()
{
    
    
    
    for (int i = 0; i<stepData_->getNumRows();i++) {
        
        auto attack = tree_.getRawParameterValue("ATTACK"+std::to_string(i));
        auto decay = tree_.getRawParameterValue("DECAY"+std::to_string(i));
        auto sustain = tree_.getRawParameterValue("SUSTAIN"+std::to_string(i));
        auto release = tree_.getRawParameterValue("RELEASE"+std::to_string(i));
        
        if(auto voice = dynamic_cast<DSPSamplerVoice*>(synth_.getVoice(i)))
        {
            voice->setADSRParams(attack->load(),decay->load(),sustain->load(),release->load());
        }
    }
}

juce::ADSR::Parameters KlideAudioProcessor::getADSRParams(int row)
{
    juce::ADSR::Parameters adsrParams;
    if(auto voice = dynamic_cast<DSPSamplerVoice*>(synth_.getVoice(row))){
        adsrParams = voice->getADSRParams();
    }
    
    return adsrParams;
        
        
}


void KlideAudioProcessor::updateFilter()
{
    
    for (int i = 0; i<stepData_->getNumRows();i++) {
        if(auto voice = dynamic_cast<DSPSamplerVoice*>(synth_.getVoice(i)))
        {
            voice->updateFilter(stepData_->getFrequencyVec()[i], stepData_->getResonanceVec()[i]);
        }
    }
    
}

void KlideAudioProcessor::updatePan()
{
    //Pan slider can only take positive values, so we remove 1 to the value at update
    for (int i = 0; i<stepData_->getNumRows();i++) {
        auto pan = tree_.getRawParameterValue("PAN"+std::to_string(i));
        
        if(auto voice = dynamic_cast<DSPSamplerVoice*>(synth_.getVoice(i)))
        {
            voice->updatePan(pan->load()-1);
        }
    }    
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
        
        
        //
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
                        
                    }
                    stepData_->next(row);
                }
            }
        }
        
        
    }//end if "is Playing"
    
    int voiceNumber = synth_.getVoiceNumber(60);
    //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
    voiceNumber = synth_.getVoiceNumber(70);
    //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
    voiceNumber = synth_.getVoiceNumber(80);
    //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
    voiceNumber = synth_.getVoiceNumber(90);
    //synth_.setVoiceLevel(gainVec_[voiceNumber], voiceNumber);
    
    
    
    updateFilter();
    updateADSRParams();
    updatePan();
    
    synth_.getVoice(synth_.getVoiceNumber(60))->renderNextBlock(buffer, 0, buffer.getNumSamples());
    synth_.getVoice(synth_.getVoiceNumber(70))->renderNextBlock(buffer, 0, buffer.getNumSamples());
    synth_.getVoice(synth_.getVoiceNumber(80))->renderNextBlock(buffer, 0, buffer.getNumSamples());
    synth_.getVoice(synth_.getVoiceNumber(90))->renderNextBlock(buffer, 0, buffer.getNumSamples());
    
    //synth_.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
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

/* //In case of using a tree
juce::AudioProcessorValueTreeState::ParameterLayout KlideAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter> > params;
    params.push_back(std::make_unique<juce::AudioParameterInt>("NUMROWS","Numrows",4,8,4));
    for(int numrow=0;numrow<4;numrow++){
        params.push_back(std::make_unique<juce::AudioParameterBool>("PLAY"+std::to_string(numrow),"Play"+std::to_string(numrow),false));
    }
    
    return {params.begin(), params.end()};
}
 */



