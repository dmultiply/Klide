/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KlideAudioProcessorEditor::KlideAudioProcessorEditor (KlideAudioProcessor& p)
: AudioProcessorEditor (&p), processor_ (p),state_ (stopped), noRowOn_(true)
{
    //Initialize StepSequencerData
    //stepData_ = new StepSequencerData();
    stepData_ = p.getStepData();
    
    //If a first initialization of stepData has been made initiate with data from stepDataSequencer
    if(stepData_ != nullptr && stepData_->getInitialized() == true)
        init();
}

void KlideAudioProcessorEditor::init() {
    
    numrows_ = stepData_->getNumRows();
    
    bpm_ = stepData_->getBpm();
    
    //Initialize the steps
    globalCurrentStepVec_ = stepData_->getGlobalCurrentStepVec();
    
    //Initialize time signature (4/4)
    localTimeSigNumerator_ = stepData_->getlocalTimeSigNumerator();
    localTimeSignDenominator_ = stepData_->getlocalTimeSigDenominator();
    
    //Initialize parameters
    playVec_ = stepData_->getPlayVec();
    pulseVec_ = stepData_->getPulseVec();
    intervalVec_ = stepData_->getIntervalVec();
    gainVec_ = stepData_->getGainVec();
    offsetVec_ = stepData_->getOffsetVec();
    frequencyVec_ = stepData_->getFrequencyVec();
    resonanceVec_ = stepData_->getResonanceVec();
    
    notesVec_ = stepData_->getNotesVec();
    
    //Sync Button
    syncButton_.setButtonText("Sync");
    syncButton_.addListener(this);
    addAndMakeVisible(&syncButton_);
    
    //Text box for tests, to remove later
    addAndMakeVisible(&testBox_);
    testBox_.setMultiLine (true);
    testBox_.setReturnKeyStartsNewLine (true);
    testBox_.setReadOnly (true);
    testBox_.setScrollbarsShown (true);
    testBox_.setCaretVisible (false);
    testBox_.setPopupMenuEnabled (true);
    testBox_.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x32ffffff));
    testBox_.setColour (juce::TextEditor::outlineColourId, juce::Colour (0x1c000000));
    testBox_.setColour (juce::TextEditor::shadowColourId, juce::Colour (0x16000000));
    
    //int currentStep = 0;
    std::vector< std::vector<int> > slideRange = stepData_->getIntervalSliderRangeVec_();
    std::vector< std::vector<int> > pulseRange = stepData_->getPulseSliderRangeVec_();
    std::vector< std::vector<float> > gainRange = stepData_->getGainSliderRangeVec_();
    std::vector< std::vector<int> > offsetRange = stepData_->getOffsetSliderRangeVec_();
    std::vector< std::vector<int> > frequencyRange = stepData_->getFrequencySliderRangeVec_();
    std::vector< std::vector<float> > resonanceRange = stepData_->getResonanceSliderRangeVec_();
    
    
    
    for(int kk=0;kk<numrows_;kk++)
    {
        int currentStep = stepData_->getGlobalStep(kk);
        //Compute a first rhytmic sequence
        std::vector<bool> stepStates;
        seqGenerator_.compute_bitmap (intervalVec_[kk], pulseVec_[kk]);
        std::vector<bool> sequence = seqGenerator_.getSequence();
        
        for(int i=0;i<sequence.size();i++)
            stepStates.push_back(sequence[i]);
        
        //Sequencers
        StepSequencerComponent* s = new StepSequencerComponent();
        s->setNumberOfSteps(intervalVec_[kk]);
        s->setNumberOfPulses(pulseVec_[kk]);
        s->setOffset(offsetVec_[kk]);
        s->setCurrentStep(currentStep);
        s->setStepStates(stepStates);
        addAndMakeVisible(s);
        stepSequencerArray_.add(s);
        
        //Controls
        ControlsComponent* c = new ControlsComponent();
        
        juce::TextButton* openButton = (juce::TextButton*)c->getChildComponent(0);
        openButton->addListener(this);
        
        juce::TextButton* startButton = (juce::TextButton*)c->getChildComponent(1);
        startButton->addListener(this);
        
        juce::TextButton* stopButton = (juce::TextButton*)c->getChildComponent(2);
        stopButton->addListener(this);
        
        juce::Slider* intervalSlider = (juce::Slider*)c->getChildComponent(3);
        intervalSlider->addListener(this);
        
        juce::Slider* pulseSlider = (juce::Slider*)c->getChildComponent(4);
        pulseSlider->addListener(this);
        
        juce::Slider* gainSlider = (juce::Slider*)c->getChildComponent(5);
        gainSlider->addListener(this);
        
        juce::Slider* offsetSlider = (juce::Slider*)c->getChildComponent(6);
        offsetSlider->addListener(this);
        
        juce::Slider* frequencySlider = (juce::Slider*)c->getChildComponent(7);
        frequencySlider->addListener(this);
        
        juce::Slider* resonanceSlider = (juce::Slider*)c->getChildComponent(8);
        resonanceSlider->addListener(this);
        
        c->setIntervalSliderRange(slideRange[kk][0], slideRange[kk][1]);
        c->setPulseSliderRange(pulseRange[kk][0], pulseRange[kk][1]);
        c->setGainSliderRange(gainRange[kk][0], gainRange[kk][1]);
        c->setOffsetSliderRange(offsetRange[kk][0], offsetRange[kk][1]);
        c->setFrequencySliderRange(frequencyRange[kk][0], frequencyRange[kk][1]);
        c->setResonanceSliderRange(resonanceRange[kk][0], resonanceRange[kk][1]);
        
        c->setIntervalSliderVal(intervalVec_[kk]);
        c->setPulseSliderVal(pulseVec_[kk]);
        c->setGainSliderVal(gainVec_[kk]);
        c->setOffsetSliderVal(offsetVec_[kk]);
        c->setFrequencySliderVal(frequencyVec_[kk]);
        c->setResonanceSliderVal(resonanceVec_[kk]);
        
        addAndMakeVisible(c);
        controlsArray_.add(c);
    }
    
    //Give stepData pointer to the processor
    //processor_.setStepData(stepData_);
    stepData_->setNumRows(numrows_);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (700, 400);
    
    //Start the timer
    startTime_ = juce::Time::getMillisecondCounterHiRes() * 0.001;
    startTimer(10);
}

KlideAudioProcessorEditor::~KlideAudioProcessorEditor()
{
    stopTimer();
    
}

void KlideAudioProcessorEditor::timerCallback()
{
    //Get the states array
    stepData_->setPlayVec(playVec_);
    std::vector< std::vector<bool> > statesArray = getStatesArray();
    
    //The states activated may be changed by the user, the processor needs to be notified, via the StepSequencerData object
    stepData_->setStatesArray(statesArray);
    
    //Get the audio step in stepData_ and update it in the widget
    for(int row=0;row<numrows_;row++)
    {
        int currentStep = stepData_->getGlobalStep(row);
        
        if(currentStep>globalCurrentStepVec_[row])
        {
            
            globalCurrentStepVec_[row] = currentStep;
            
            if(playVec_[row] == 1)
                stepSequencerArray_[row]->step();
        }
        else if(currentStep == 0 && globalCurrentStepVec_[row] == intervalVec_[row]-1)
        {
            globalCurrentStepVec_[row] = currentStep;
            
            if(playVec_[row] == 1)
            {
                stepSequencerArray_[row]->step();
            }
        }
    }
    
    /*
     In case we need to test the sync with the host in the testBox
     
     String m = "info : " + processor_.getInfo();
     
     testBox_.moveCaretToTop(1);
     testBox_.insertTextAtCaret (m + newLine);
     
     */
    
}

//==============================================================================
void KlideAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (juce::Colour((juce::uint8)44,(juce::uint8)53,(juce::uint8)57,1.0f));
    
    //g.setColour (juce::Colour((juce::uint8)34,(juce::uint8)43,(juce::uint8)47,1.0f));
    
    backgroundImage_ = juce::ImageCache::getFromMemory (BinaryData::fractal_png, BinaryData::fractal_pngSize);
    g.drawImage(backgroundImage_, getLocalBounds ().toFloat ());
    
    g.setFont (15.0f);
}

void KlideAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> synthArea = area.removeFromTop(area.getHeight()*4/5);
    juce::Rectangle<int> controlsArea = synthArea.removeFromLeft(area.getWidth()*3/5);
    
    const int controlsHeight = synthArea.getHeight()/5;
    
    for(int kk=0;kk<numrows_;kk++) {
        controlsArray_[kk]->setBounds(controlsArea.removeFromTop(controlsHeight));
        stepSequencerArray_[kk]->setBounds(synthArea.removeFromTop(controlsHeight).reduced(controlsHeight/5));
    }
    
    juce::Rectangle<int> moreControlsArea = area.removeFromTop(area.getHeight());
    
    syncButton_.setBounds(moreControlsArea.removeFromRight(100).reduced(10));
    //testBox_.setBounds(area.removeFromTop(controlsHeight));
}

void KlideAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if(button == &syncButton_)
        KlideAudioProcessorEditor::syncButtonClicked();
    
    for(int kk=0;kk<controlsArray_.size();kk++)
    {
        
        if((juce::Button*)controlsArray_[kk]->getChildren()[0] == button)
            KlideAudioProcessorEditor::openButtonClicked(kk);
        if((juce::Button*)controlsArray_[kk]->getChildren()[1] == button)
            KlideAudioProcessorEditor::playButtonClicked(kk);
        if((juce::Button*)controlsArray_[kk]->getChildren()[2] == button)
            KlideAudioProcessorEditor::stopButtonClicked(kk);
    }
    
}

void KlideAudioProcessorEditor::openButtonClicked(int row)
{
    
    juce::FileChooser chooser ("Select a Wave file to play...",
                               juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                               "*.wav");
    if (chooser.browseForFileToOpen())
    {
        juce::File file (chooser.getResult());
        
        processor_.getSynth()->setSample(stepData_->getNote(row), file.getFullPathName());
    }
    
    
}

void KlideAudioProcessorEditor::playButtonClicked(int row)
{
    jassert(row<intervalVec_.size());
    jassert(row<notesVec_.size());
    jassert(row<playVec_.size());
    jassert(row<globalCurrentStepVec_.size());
    jassert(row<stepSequencerArray_.size());
    
    
    if(noRowOn_ == true) //No row is started yet
    {
        
        //Get the states array, gives the state of each step, on or off
        std::vector< std::vector<bool> > statesArray = getStatesArray();
        
        jassert(row<statesArray.size());
        stepData_->setStates(row,statesArray[row]);
        
        //timeStamp is the time when the first row start
        //in reference to startTime_
        float timeStamp = juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime_;
        stepData_->setTimeStamp(timeStamp);
        stepData_->setStartTime(startTime_);
        
        //Setting of the number of intervals and their durations
        stepData_->setInterval(row, intervalVec_[row]);
        double duration = stepData_->computeStepDuration(row);
        stepData_->setStepTime(row, duration);
        
        //Setting of the note for this row, used by the audio processor via stepData
        stepData_->setNote(row,notesVec_[row]);
        
        //Set the status play on or off for the row
        playVec_[row] = 1;
        stepData_->setPlay(row,playVec_[row]);
        
        //Set the step counters, for the audio and the widget
        stepData_->setGlobalStep(row, -1); //Audio step
        globalCurrentStepVec_[row] = -1;//Used in the timer callback to increment the widget step
        stepSequencerArray_[row]->setCurrentStep(globalCurrentStepVec_[row]);//Widget step
        
        //Create the first step. If play is on, the playing starts on this row
        stepData_->createNextStep(row, 0, timeStamp);
        noRowOn_ = false;
        
    }
    else
    {
        //Get the states array, gives the state of each step, on or off
        std::vector< std::vector<bool> > statesArray = getStatesArray();
        stepData_->setStates(row,statesArray[row]);
        
        //Setting of the note for this row, used by the audio processor via stepData
        stepData_->setNote(row,notesVec_[row]);
        
        //Setting of the number of intervals and their durations
        stepData_->setInterval(row, intervalVec_[row]);
        double duration = stepData_->computeStepDuration(row);
        stepData_->setStepTime(row, duration);
        
        //A first row is already started. We compute what is it's current step
        //and we start the new row at this step and timestamp
        //the rows may have different intervals durations and number of steps
        int absoluteStep = stepData_->computeAbsoluteStep(row);
        int currentStep = (absoluteStep)%stepData_->getInterval(row);
        float currentTimestamp = stepData_->getTimeStamp()+(float)absoluteStep*stepData_->getStepTime(row);
        
        //Set the step counters, for the audio and the widget
        stepData_->setGlobalStep(row, currentStep-1); //Audio stepping
        globalCurrentStepVec_[row] = currentStep-1; //Used in the timer callback to increment the widget step
        stepSequencerArray_[row]->setCurrentStep(globalCurrentStepVec_[row]);//Widget step
        
        //Create the first step. If play is on, the playing starts on this row
        stepData_->createNextStep(row, currentStep,currentTimestamp);
        
        //Set the status play on or off for the row
        playVec_[row] = 1;
        stepData_->setPlay(row,playVec_[row]);
    }
    
}

void KlideAudioProcessorEditor::stopButtonClicked(int row)
{
    jassert(row<playVec_.size());
    jassert(row<globalCurrentStepVec_.size());
    
    playVec_[row] = 0;
    
    //Put the stepper back to 0 if all rows are stopped
    if(std::find(playVec_.begin(), playVec_.end(), 1) != playVec_.end()) {
        //Do nothing if there is still a row playing
    } else {
        globalCurrentStepVec_[row] = -1;
        noRowOn_ = true;
    }
}

void KlideAudioProcessorEditor::sliderValueChanged (juce::Slider *slider)
{
    for(int kk=0;kk<controlsArray_.size();kk++)
    {
        
        if((juce::Slider*)controlsArray_[kk]->getChildren()[3] == slider)
            KlideAudioProcessorEditor::intervalValueChanged(slider, kk);
        if((juce::Slider*)controlsArray_[kk]->getChildren()[4] == slider)
            KlideAudioProcessorEditor::pulseValueChanged(slider, kk);
        if((juce::Slider*)controlsArray_[kk]->getChildren()[5] == slider)
            KlideAudioProcessorEditor::gainValueChanged(slider, kk);
        if((juce::Slider*)controlsArray_[kk]->getChildren()[6] == slider)
            KlideAudioProcessorEditor::offsetValueChanged(slider, kk);
        if((juce::Slider*)controlsArray_[kk]->getChildren()[7] == slider)
            KlideAudioProcessorEditor::frequencyValueChanged(slider, kk);
        if((juce::Slider*)controlsArray_[kk]->getChildren()[8] == slider)
            KlideAudioProcessorEditor::resonanceValueChanged(slider, kk);
    }
    
    
}

void KlideAudioProcessorEditor::intervalValueChanged(juce::Slider *slider, int row)
{
    //Fix this jassert
    //jassert(row<playVec_.size());
    //jassert(row<intervalVec_.size());
    jassert(row<controlsArray_.size());
    jassert(row<pulseVec_.size());
    jassert(row<globalCurrentStepVec_.size());
    jassert(row<stepSequencerArray_.size());
    
    //If the row is playing, the row is stopped during processing of the new interval
    int playOn = playVec_[row];
    playVec_[row] = 0;
    stepData_->setPlay(row,playVec_[row]);
    
    //Get the new number of steps for the row
    intervalVec_[row] = slider->getValue();
    
    //Change the number of pulses if superior to the number of steps
    if(controlsArray_[row]->getPulseSliderVal() > intervalVec_[row])
    {
        controlsArray_[row]->setPulseSliderVal(intervalVec_[row]/2.0);
        pulseVec_[row] = intervalVec_[row]/2.0;
    }
    
    controlsArray_[row]->setPulseSliderRange(0, intervalVec_[row]);
    
    //Compute Euclide Algo to get new sequence of states with the new interval and pulses
    seqGenerator_.compute_bitmap (intervalVec_[row], pulseVec_[row]);
    std::vector<bool> sequence = seqGenerator_.getSequence();
    
    //Set the new states in the widget
    stepSequencerArray_[row]->setStepStates(sequence);
    
    //Setting of the number of intervals and their durations
    stepData_->setInterval(row,intervalVec_[row]);
    double duration = stepData_->computeStepDuration(row);
    stepData_->setStepTime(row, duration);
    
    //If a first row or this one is already started. We compute what is it's current step
    //and we start the new row at this step and timestamp
    //the rows may have different intervals durations and number of steps
    int absoluteStep = stepData_->computeAbsoluteStep(row);
    int currentStep = (absoluteStep)%stepData_->getInterval(row);
    float currentTimestamp = stepData_->getTimeStamp()+(float)absoluteStep*stepData_->getStepTime(row);
    
    //number of intervals change notification
    stepSequencerArray_[row]->setNumberOfSteps(intervalVec_[row]);
    
    //Set the step counters, for the audio and the widget
    stepData_->setGlobalStep(row, currentStep-1); //Audio step
    globalCurrentStepVec_[row] = currentStep-1; //Used in the timer callback to increment the widget step
    stepSequencerArray_[row]->setCurrentStep(globalCurrentStepVec_[row]);//Widget step
    
    //Get the states array, gives the state of each step, on or off
    std::vector< std::vector<bool> > statesArray = getStatesArray();
    jassert(row<statesArray.size());
    stepData_->setStates(row,statesArray[row]);
    
    //2022 memorize the new interval
    stepData_->setIntervalVec(row, intervalVec_[row]);
    
    //Create the next step with the new intervals and pulses
    //Will restart the row at currentStep if the row was playing before
    stepData_->createNextStep(row, currentStep,currentTimestamp);
    playVec_[row] = playOn;
    stepData_->setPlay(row,playVec_[row]);
    
}

void KlideAudioProcessorEditor::pulseValueChanged(juce::Slider *slider, int row)
{
    jassert(row<pulseVec_.size());
    jassert(row<intervalVec_.size());
    jassert(row<stepSequencerArray_.size());
    
    pulseVec_[row] = slider->getValue();
    stepSequencerArray_[row]->setNumberOfPulses(pulseVec_[row]);
    
    //Compute Euclide Algo
    seqGenerator_.compute_bitmap (intervalVec_[row], pulseVec_[row]);
    std::vector<bool> sequence = seqGenerator_.getSequence();
    
    stepSequencerArray_[row]->setStepStates(sequence);
    
    //2022 memorize the new pulse
    stepData_->setPulseVec(row, pulseVec_[row]);
    
}

void KlideAudioProcessorEditor::gainValueChanged(juce::Slider *slider, int row)
{
    gainVec_[row] = slider->getValue();
    
    processor_.setGain(slider->getValue(), row);
    
    //2022 memorize the new gain
    stepData_->setGainVec(row, slider->getValue());
}

void KlideAudioProcessorEditor::offsetValueChanged(juce::Slider *slider, int row)
{
    offsetVec_[row] = slider->getValue();
    
    //get the sequence
    std::vector<bool> sequence = getStatesArray()[row];
    //Set the sequence in seqGenerator
    seqGenerator_.setSequence(sequence);
    //Calculate the new sequence
    seqGenerator_.offsetSequence(slider->getValue());
    
    //repaint the sequence
    sequence = seqGenerator_.getSequence();
    
    stepSequencerArray_[row]->setStepStates(sequence);
    
    
    //memorize the new Offset
    stepData_->setOffsetVec(row, slider->getValue());
    
}

void KlideAudioProcessorEditor::frequencyValueChanged(juce::Slider *slider, int row)
{
    frequencyVec_[row] = slider->getValue();
    
    //memorize the new Offset
    stepData_->setFrequencyVec(row, slider->getValue());
    
}

void KlideAudioProcessorEditor::resonanceValueChanged(juce::Slider *slider, int row)
{
    resonanceVec_[row] = slider->getValue();
    
    //memorize the new Offset
    stepData_->setResonanceVec(row, slider->getValue());
    
}

void KlideAudioProcessorEditor::syncButtonClicked()
{
    
    //std::vector< std::vector<bool> > statesArray = getStatesArray();
    //processor_.setStatesArray(statesArray);
    processor_.setSyncOn(1);
    
}

std::vector< std::vector<bool> > KlideAudioProcessorEditor::getStatesArray()
{
    std::vector< std::vector<bool> > statesArray;
    for(int row=0;row<stepSequencerArray_.size();row++)
    {
        std::vector<bool> states = std::vector<bool>(stepSequencerArray_[row]->getStepStates());
        statesArray.push_back(states);
    }
    return statesArray;
}
