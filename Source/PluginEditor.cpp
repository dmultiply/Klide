/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KlideAudioProcessorEditor::KlideAudioProcessorEditor (KlideAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor_ (p), presetPanel(p.getPresetManager(),p), state_ (stopped), noRowOn_(true)
{
    
    addAndMakeVisible(presetPanel);
    
    //Get StepSequencerData from the AudioProcessor
    stepData_ = p.getStepData();
    
    if(stepData_ != nullptr)
        init();
    
    stepData_->isInitialized();
}

void KlideAudioProcessorEditor::init() {
    
    //Getting the number of rows and bpm
    numrows_ = stepData_->getNumRows();
    bpm_ = stepData_->getBpm();
    
    //Initialize the steps
    globalCurrentStepVec_ = stepData_->getGlobalCurrentStepVec();
    
    //Initialize time signature (4/4)
    localTimeSigNumerator_ = stepData_->getlocalTimeSigNumerator();
    localTimeSignDenominator_ = stepData_->getlocalTimeSigDenominator();
    
    //Initialize parameters values
    playVec_ = stepData_->getPlayVec();
    notesVec_ = stepData_->getNotesVec();
    
    
    //Initialize the sequencers
    for(int row = 0;row<numrows_;row++)
    {
        StepSequencerComponent* s = new StepSequencerComponent();
        addAndMakeVisible(s);
        stepSequencerArray_.add(s);
    }
    
    //Initialize row controls
    initRowControls();
    
    //Set the states of the sequencers and their controls
    initComponents();
    
    //Add sync button
    syncButton_.setButtonText("Sync");
    syncButton_.addListener(this);
    addAndMakeVisible(&syncButton_);
   
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (700, 400);
    
    //Row Choice Attachment
    //It has to be after setSize, because the listener is calling resized()
    rowChoiceSliderAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"ROWCHOICE", rowChoiceSlider_);
    
    //Start the timer
    startTime_ = juce::Time::getMillisecondCounterHiRes() * 0.001;
    startTimer(10);
}

void KlideAudioProcessorEditor::initRowControls()
{
    //Row choice slider
    rowChoiceSlider_.setSliderStyle(Slider::SliderStyle::LinearVertical);
    rowChoiceSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 30);
    rowChoiceSlider_.setTextBoxIsEditable(true);
    rowChoiceSlider_.addListener(this);
    addAndMakeVisible(rowChoiceSlider_);
    
    //Row Choice Label
    rowChoiceLabel_.setText ("Row", juce::dontSendNotification);
    rowChoiceLabel_.setFont (10);
    rowChoiceLabel_.setJustificationType(4);
    rowChoiceLabel_.attachToComponent (&rowChoiceSlider_, false);
    addAndMakeVisible (rowChoiceLabel_);
    
    //ADSR generic Sliders     // ====================
    
    for(int row = 0;row<4;row++)
    {
        //Attack
        juce::Slider* a = new juce::Slider();
        a->setSliderStyle(Slider::SliderStyle::LinearVertical);
        a->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        a->setTextBoxIsEditable(true);
        a->setSkewFactor(0.8);
        a->addListener(this);
        addAndMakeVisible(a);
        
        attackSliderVec_.add(a);
        
        auto attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"ATTACK"+std::to_string(row), *attackSliderVec_[row]);
        
        
        attackAttachmentVec_.push_back(std::move(attackAttachment));
        
        //Decay
        juce::Slider* d = new juce::Slider();
        d->setSliderStyle(Slider::SliderStyle::LinearVertical);
        d->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        d->setTextBoxIsEditable(true);
        d->addListener(this);
        addAndMakeVisible(d);
        
        decaySliderVec_.add(d);
        
        auto decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"DECAY"+std::to_string(row), *decaySliderVec_[row]);
        
        
        decayAttachmentVec_.push_back(std::move(decayAttachment));
        
        //Sustain
        juce::Slider* s = new juce::Slider();
        s->setSliderStyle(Slider::SliderStyle::LinearVertical);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        s->setTextBoxIsEditable(true);
        s->addListener(this);
        addAndMakeVisible(s);
        
        sustainSliderVec_.add(s);
        
        auto sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"SUSTAIN"+std::to_string(row), *sustainSliderVec_[row]);
        
        
        sustainAttachmentVec_.push_back(std::move(sustainAttachment));
        
        //Release
        juce::Slider* r = new juce::Slider();
        r->setSliderStyle(Slider::SliderStyle::LinearVertical);
        r->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        r->setTextBoxIsEditable(true);
        r->addListener(this);
        addAndMakeVisible(r);
        
        releaseSliderVec_.add(r);
        
        auto releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"RELEASE"+std::to_string(row), *releaseSliderVec_[row]);
        
        
        releaseAttachmentVec_.push_back(std::move(releaseAttachment));
        
        //Pan
        juce::Slider* p = new juce::Slider();
        p->setSliderStyle(Slider::SliderStyle::LinearVertical);
        p->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        p->setTextBoxIsEditable(true);
        p->addListener(this);
        addAndMakeVisible(p);
        
        panSliderVec_.add(p);
        
        auto panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"PAN"+std::to_string(row), *panSliderVec_[row]);
        
        
        panAttachmentVec_.push_back(std::move(panAttachment));
        
        //Interval
        CustomKnob* i = new CustomKnob();
        i->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        i->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        i->setTextBoxIsEditable(true);
        i->addListener(this);
        addAndMakeVisible(i);
        
        intervalSliderVec_.add(i);
        
        
        
        //Pulse
        CustomKnob* pulse = new CustomKnob();
        pulse->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        pulse->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        pulse->setTextBoxIsEditable(true);
        pulse->addListener(this);
        addAndMakeVisible(pulse);
        
        pulseSliderVec_.add(pulse);
        
        //Offset
        CustomKnob* o = new CustomKnob();
        o->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        o->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        o->setTextBoxIsEditable(true);
        o->addListener(this);
        addAndMakeVisible(o);
        
        offsetSliderVec_.add(o);
        
        //Gain
        CustomKnob* g = new CustomKnob();
        g->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        g->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        g->setTextBoxIsEditable(true);
        g->addListener(this);
        addAndMakeVisible(g);
        
        gainSliderVec_.add(g);
        
        //Frequency
        CustomKnob* f = new CustomKnob();
        f->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        f->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        f->setTextBoxIsEditable(true);
        f->addListener(this);
        addAndMakeVisible(f);
        
        frequencySliderVec_.add(f);
        
        //Resonance
        CustomKnob* res = new CustomKnob();
        res->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        res->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
        res->setTextBoxIsEditable(true);
        res->addListener(this);
        addAndMakeVisible(res);
        
        resonanceSliderVec_.add(res);
        
        // === Attachments
        auto intervalAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"INTERVAL"+std::to_string(row), *intervalSliderVec_[row]);
        
        auto pulseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"PULSE"+std::to_string(row), *pulseSliderVec_[row]);
        
        auto offsetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"OFFSET"+std::to_string(row), *offsetSliderVec_[row]);
        
        auto gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"GAIN"+std::to_string(row), *gainSliderVec_[row]);
        
        auto frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"FREQUENCY"+std::to_string(row), *frequencySliderVec_[row]);
        
        auto resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.tree_,"RESONANCE"+std::to_string(row), *resonanceSliderVec_[row]);
        
        //Push the attachments inside the vectors
        intervalAttachmentVec_.push_back(std::move(intervalAttachment));
        pulseAttachmentVec_.push_back(std::move(pulseAttachment));
        offsetAttachmentVec_.push_back(std::move(offsetAttachment));
        gainAttachmentVec_.push_back(std::move(gainAttachment));
        frequencyAttachmentVec_.push_back(std::move(frequencyAttachment));
        resonanceAttachmentVec_.push_back(std::move(resonanceAttachment));
        
    }
    
    //ADSR Label
    //A
    attackLabel_.setText ("A", juce::dontSendNotification);
    attackLabel_.setFont (10);
    attackLabel_.setJustificationType(4);
    attackLabel_.attachToComponent (attackSliderVec_[0], false);
    addAndMakeVisible (attackLabel_);
    
    //D
    decayLabel_.setText ("D", juce::dontSendNotification);
    decayLabel_.setFont (10);
    decayLabel_.setJustificationType(4);
    decayLabel_.attachToComponent (decaySliderVec_[0], false);
    addAndMakeVisible (decayLabel_);
    
    //S
    sustainLabel_.setText ("S", juce::dontSendNotification);
    sustainLabel_.setFont (10);
    sustainLabel_.setJustificationType(4);
    sustainLabel_.attachToComponent (sustainSliderVec_[0], false);
    addAndMakeVisible (sustainLabel_);
    
    //R
    releaseLabel_.setText ("R", juce::dontSendNotification);
    releaseLabel_.setFont (10);
    releaseLabel_.setJustificationType(4);
    releaseLabel_.attachToComponent (releaseSliderVec_[0], false);
    addAndMakeVisible (releaseLabel_);
    
    //Pan
    panLabel_.setText ("Pan", juce::dontSendNotification);
    panLabel_.setFont (10);
    panLabel_.setJustificationType(4);
    panLabel_.attachToComponent (panSliderVec_[0], false);
    addAndMakeVisible (panLabel_);
    
    //Interval
    intervalLabel_.setText ("Interval", juce::dontSendNotification);
    intervalLabel_.setFont (10);
    intervalLabel_.setJustificationType(4);
    intervalLabel_.attachToComponent (intervalSliderVec_[0], false);
    addAndMakeVisible (intervalLabel_);
    
    //Pulse
    pulseLabel_.setText ("Pulse", juce::dontSendNotification);
    pulseLabel_.setFont (10);
    pulseLabel_.setJustificationType(4);
    pulseLabel_.attachToComponent (pulseSliderVec_[0], false);
    addAndMakeVisible (pulseLabel_);
    
    //Offset
    offsetLabel_.setText ("Offset", juce::dontSendNotification);
    offsetLabel_.setFont (10);
    offsetLabel_.setJustificationType(4);
    offsetLabel_.attachToComponent (offsetSliderVec_[0], false);
    addAndMakeVisible (offsetLabel_);
    
    //Gain
    gainLabel_.setText ("Gain", juce::dontSendNotification);
    gainLabel_.setFont (10);
    gainLabel_.setJustificationType(4);
    gainLabel_.attachToComponent (gainSliderVec_[0], false);
    addAndMakeVisible (gainLabel_);
    
    //Frequency
    frequencyLabel_.setText ("Freq", juce::dontSendNotification);
    frequencyLabel_.setFont (10);
    frequencyLabel_.setJustificationType(4);
    frequencyLabel_.attachToComponent (frequencySliderVec_[0], false);
    addAndMakeVisible (frequencyLabel_);
    
    //Resonance
    resonanceLabel_.setText ("Res", juce::dontSendNotification);
    resonanceLabel_.setFont (10);
    resonanceLabel_.setJustificationType(4);
    resonanceLabel_.attachToComponent (resonanceSliderVec_[0], false);
    addAndMakeVisible (resonanceLabel_);
}


void KlideAudioProcessorEditor::initComponents()
{
    
    //Clear restart Array
    restartArray_.clear();
    
    //Loop on StepSequencerComponents and ControlsComponent to initiate the parameters there
    for(int row=0;row<numrows_;row++)
    {
        int currentStep = stepData_->getGlobalStep(row);
        
        //If a first initialization, generate, else give back sequence, in case it was modified by hand
        std::vector<bool> stepStates;
        
        if(stepData_->getInitialized() == false) {
            //Compute a first rhytmic sequence
            seqGenerator_.compute_bitmap (intervalSliderVec_[row]->getValue(), pulseSliderVec_[row]->getValue());
            std::vector<bool> sequence = seqGenerator_.getSequence();
            
            for(int i=0;i<sequence.size();i++)
                stepStates.push_back(sequence[i]);
        }
        
        //init Restart Array with the sequence that existed before closing the windows
        if(stepData_->getInitialized() == true)
            restartArray_.push_back(stepData_->getRestartVec(row));
        
        //Build Step Sequencer components. They will be added to a juce::OwnedArray, that will destroy them when it will be destroyed itself
        stepSequencerArray_[row]->setNumberOfSteps(intervalSliderVec_[row]->getValue());
        stepSequencerArray_[row]->setNumberOfPulses(pulseSliderVec_[row]->getValue());
        stepSequencerArray_[row]->setOffset(offsetSliderVec_[row]->getValue());
        stepSequencerArray_[row]->setCurrentStep(currentStep);
        stepSequencerArray_[row]->setStepStates(stepStates);
        
        //Controls
        ControlsComponent* c = new ControlsComponent();
        
        juce::TextButton* openButton = (juce::TextButton*)c->getChildComponent(0);
        openButton->addListener(this);
        
        juce::TextButton* playButton = (juce::TextButton*)c->getChildComponent(1);
        playButton->addListener(this);
        
        juce::TextButton* stopButton = (juce::TextButton*)c->getChildComponent(2);
        stopButton->addListener(this);
        
        addAndMakeVisible(c);
        controlsArray_.add(c);
        
    }
    /* CHEAT : Put states like they were before closing the window and before the states are reinitialized by the sliders changes
     TODO : find a better method */
    if(stepData_->getInitialized() == true){
        fakeSeqSlider_.setValue(10);
        fakeSeqSlider_.addListener(this);
    }
    
    addAndMakeVisible(fakeSeqSlider_);
    
}

KlideAudioProcessorEditor::~KlideAudioProcessorEditor()
{
    stopTimer();
    
    //Save states in StepData_ before closing of the window
    stepData_->setRestartArray(getStatesArray());
    
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
        else if(currentStep == 0 && globalCurrentStepVec_[row] == intervalSliderVec_[row]->getValue()-1)
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
     
     String m = "info : " + audioProcessor_.getInfo();
     
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
    
    //Areas, for the synth, separated in controls and sequencer
    juce::Rectangle<int> area = getLocalBounds();
    presetPanel.setBounds(area.removeFromTop(proportionOfHeight(0.1f)));

    
    juce::Rectangle<int> synthArea = area.removeFromTop(area.getHeight()*30/50);
    //juce::Rectangle<int> controlsArea = synthArea.removeFromLeft(area.getWidth()*3/5);
    juce::Rectangle<int> controlsArea = synthArea.removeFromLeft(area.getWidth()*15/50);

    
    const int controlsHeight = synthArea.getHeight()* 12/50;
    
    for(int kk=0;kk<numrows_;kk++) {
        controlsArray_[kk]->setBounds(controlsArea.removeFromTop(controlsHeight));
        stepSequencerArray_[kk]->setBounds(synthArea.removeFromTop(controlsHeight).reduced(controlsHeight/10));
    }
    
    //More controls, under the synth
    
    // === Row
    rowChoiceSlider_.setBounds(area.removeFromLeft(50).reduced(10));
    
    // === Make sliders of non selected rows invisible
    for(int row=0;row<numrows_;row++)
    {
        attackSliderVec_[row]->setVisible(false);
        sustainSliderVec_[row]->setVisible(false);
        panSliderVec_[row]->setVisible(false);
        
        intervalSliderVec_[row]->setVisible(false);
        pulseSliderVec_[row]->setVisible(false);
        offsetSliderVec_[row]->setVisible(false);
        
        gainSliderVec_[row]->setVisible(false);

        frequencySliderVec_[row]->setVisible(false);
        resonanceSliderVec_[row]->setVisible(false);
        
    }
    
    //Make sliders and labels visible for the chosen row
    attackSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    attackSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(50).reduced(10));
    attackLabel_.attachToComponent (attackSliderVec_[rowChoiceSlider_.getValue()], false);
    
    sustainSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    sustainSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(50).reduced(10));
    sustainLabel_.attachToComponent (sustainSliderVec_[rowChoiceSlider_.getValue()], false);

    panSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    panSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(50).reduced(10));
    panLabel_.attachToComponent (panSliderVec_[rowChoiceSlider_.getValue()], false);
    
    intervalSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    intervalSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(70).reduced(10));
    intervalLabel_.attachToComponent (intervalSliderVec_[rowChoiceSlider_.getValue()], false);
    
    pulseSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    pulseSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(70).reduced(10));
    pulseLabel_.attachToComponent (pulseSliderVec_[rowChoiceSlider_.getValue()], false);
    
    offsetSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    offsetSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(70).reduced(10));
    offsetLabel_.attachToComponent (offsetSliderVec_[rowChoiceSlider_.getValue()], false);
    
    gainSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    gainSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(70).reduced(10));
    gainLabel_.attachToComponent (gainSliderVec_[rowChoiceSlider_.getValue()], false);
    
    frequencySliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    frequencySliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(70).reduced(10));
    frequencyLabel_.attachToComponent (frequencySliderVec_[rowChoiceSlider_.getValue()], false);
    
    resonanceSliderVec_[rowChoiceSlider_.getValue()]->setVisible(true);
    resonanceSliderVec_[rowChoiceSlider_.getValue()]->setBounds(area.removeFromLeft(70).reduced(10));
    resonanceLabel_.attachToComponent (resonanceSliderVec_[rowChoiceSlider_.getValue()], false);
    
    // === Sync Button
    syncButton_.setBounds(area.removeFromRight(70).reduced(10));
    
    
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
                               juce::File::getSpecialLocation (juce::File::userMusicDirectory),
                               "*.wav");
    if (chooser.browseForFileToOpen())
    {
        juce::File file (chooser.getResult());
        
        //Put in the tree the filename
        Value audioPath = audioProcessor_.tree_.state.getPropertyAsValue("AUDIO_FILEPATH"+String(row), nullptr, true);
        audioPath.setValue(file.getFullPathName());
        
        audioProcessor_.getSynth()->setSample(stepData_->getNote(row), file.getFullPathName());
    }
    
    
}

void KlideAudioProcessorEditor::playButtonClicked(int row)
{
    jassert(row<intervalSliderVec_.size());
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
        stepData_->setInterval(row, intervalSliderVec_[row]->getValue());
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
        stepData_->setInterval(row, intervalSliderVec_[row]->getValue());
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
    
    for(int row=0;row<stepData_->getNumRows();row++)
    {
        if((juce::Slider*) intervalSliderVec_[row] == slider)
            KlideAudioProcessorEditor::intervalValueChanged(slider, row);
        if((juce::Slider*) pulseSliderVec_[row] == slider)
            KlideAudioProcessorEditor::pulseValueChanged(slider, row);
        if((juce::Slider*) offsetSliderVec_[row] == slider)
            KlideAudioProcessorEditor::offsetValueChanged(slider, row);
        if((juce::Slider*) gainSliderVec_[row] == slider)
            KlideAudioProcessorEditor::gainValueChanged(slider, row);
        if((juce::Slider*) frequencySliderVec_[row] == slider)
            KlideAudioProcessorEditor::frequencyValueChanged(slider, row);
        if((juce::Slider*) resonanceSliderVec_[row] == slider)
            KlideAudioProcessorEditor::resonanceValueChanged(slider, row);

    }
    
    
    
    //CHEAT (started in the initComponents), Fake slider to put the same sequence that was before closing the window, in case it was changed directly by clicking on the sequence
    
    if(isNotRestarted_) {
        if(&fakeSeqSlider_ == slider){
            for(int i = 0; i<stepData_->getNumRows();i++)
                stepSequencerArray_[i]->setStepStates(restartArray_[i]);
            
            isNotRestarted_ = false;
        }
        
    }
    
        
    if(&rowChoiceSlider_ == slider){
        
        // === Replace the sliders for the current row
        repaint();
        resized();
    }
    
}

void KlideAudioProcessorEditor::intervalValueChanged(juce::Slider *slider, int row)
{
    //Fix this jassert
    //jassert(row<playVec_.size());
    //jassert(row<intervalVec_.size());
    jassert(row<intervalSliderVec_.size());
    jassert(row<globalCurrentStepVec_.size());
    jassert(row<stepSequencerArray_.size());
    
    //If the row is playing, the row is stopped during processing of the new interval
    int playOn = playVec_[row];
    playVec_[row] = 0;
    stepData_->setPlay(row,playVec_[row]);
    
    //Get the new number of steps for the row
    //intervalVec_[row] = slider->getValue();
    auto interval = slider->getValue();
    
    //Change the number of pulses if superior to the number of steps
    if(pulseSliderVec_[row]->getValue() > interval)
    {
        pulseSliderVec_[row]->setValue(interval/2.0);
    }

    //The numer of pulses cannot be more than the number of intervals
    pulseSliderVec_[row]->setRange(0,interval);
    
    //Compute Euclide Algo to get new sequence of states with the new interval and pulses
    seqGenerator_.compute_bitmap (interval, pulseSliderVec_[row]->getValue());
    std::vector<bool> sequence = seqGenerator_.getSequence();
    
    //Set the new states in the widget
    stepSequencerArray_[row]->setStepStates(sequence);
    
    //Setting of the number of intervals and their durations
    stepData_->setInterval(row,interval);
    double duration = stepData_->computeStepDuration(row);
    stepData_->setStepTime(row, duration);
    
    //If a first row or this one is already started. We compute what is it's current step
    //and we start the new row at this step and timestamp
    //the rows may have different intervals durations and number of steps
    int absoluteStep = stepData_->computeAbsoluteStep(row);
    int currentStep = (absoluteStep)%stepData_->getInterval(row);
    float currentTimestamp = stepData_->getTimeStamp()+(float)absoluteStep*stepData_->getStepTime(row);
    
    //number of intervals change notification
    stepSequencerArray_[row]->setNumberOfSteps(interval);
    
    //Set the step counters, for the audio and the widget
    stepData_->setGlobalStep(row, currentStep-1); //Audio step
    globalCurrentStepVec_[row] = currentStep-1; //Used in the timer callback to increment the widget step
    stepSequencerArray_[row]->setCurrentStep(globalCurrentStepVec_[row]);//Widget step
    
    //Get the states array, gives the state of each step, on or off
    std::vector< std::vector<bool> > statesArray = getStatesArray();
    jassert(row<statesArray.size());
    stepData_->setStates(row,statesArray[row]);
    
    //Create the next step with the new intervals and pulses
    //Will restart the row at currentStep if the row was playing before
    stepData_->createNextStep(row, currentStep,currentTimestamp);
    playVec_[row] = playOn;
    stepData_->setPlay(row,playVec_[row]);
    
}

void KlideAudioProcessorEditor::pulseValueChanged(juce::Slider *slider, int row)
{
    jassert(row<pulseSliderVec_.size());
    jassert(row<intervalSliderVec_.size());
    jassert(row<stepSequencerArray_.size());
    
    auto pulse = slider->getValue();
    if(pulse>intervalSliderVec_[row]->getValue())
    {
        pulse = intervalSliderVec_[row]->getValue();
        pulseSliderVec_[row]->setRange(0,intervalSliderVec_[row]->getValue());
    }
        
        
    stepSequencerArray_[row]->setNumberOfPulses(pulse);
    
    //Compute Euclide Algo
    seqGenerator_.compute_bitmap (intervalSliderVec_[row]->getValue(), pulse);
    std::vector<bool> sequence = seqGenerator_.getSequence();
    
    stepSequencerArray_[row]->setStepStates(sequence);
    
    //2022 memorize the new pulse
    stepData_->setPulseVec(row, pulse);
    
}

void KlideAudioProcessorEditor::gainValueChanged(juce::Slider *slider, int row)
{
    auto gain = slider->getValue();
    
    audioProcessor_.setGain(gain, row);
    
    //2022 memorize the new gain
    stepData_->setGainVec(row, gain);
}

void KlideAudioProcessorEditor::offsetValueChanged(juce::Slider *slider, int row)
{
    auto offset = slider->getValue();
    
    //get the sequence
    std::vector<bool> sequence = getStatesArray()[row];
    //Set the sequence in seqGenerator
    seqGenerator_.setSequence(sequence);
    //Calculate the new sequence
    seqGenerator_.offsetSequence(offset);
    
    //repaint the sequence
    sequence = seqGenerator_.getSequence();
    
    stepSequencerArray_[row]->setStepStates(sequence);
    
    
    //memorize the new Offset
    stepData_->setOffsetVec(row, offset);
    
}

void KlideAudioProcessorEditor::frequencyValueChanged(juce::Slider *slider, int row)
{
    auto frequency = slider->getValue();
    
    //memorize the new Offset
    stepData_->setFrequencyVec(row, frequency);
    
}

void KlideAudioProcessorEditor::resonanceValueChanged(juce::Slider *slider, int row)
{
    auto resonance = slider->getValue();
    
    //memorize the new Offset
    stepData_->setResonanceVec(row, resonance);
    
}

void KlideAudioProcessorEditor::syncButtonClicked()
{
    
    //std::vector< std::vector<bool> > statesArray = getStatesArray();
    //audioProcessor_.setStatesArray(statesArray);
    audioProcessor_.setSyncOn(1);
    
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

