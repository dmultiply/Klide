/*
  ==============================================================================

    StepSequencerData.cpp
    Created: 28 Dec 2017 4:18:27pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "StepSequencerData.h"

StepSequencerData::StepSequencerData():numRows_(4), sampleRate_(44100),bpm_(100),syncOn_(0),startTime_(0),timeStamp_(-1)
{
    
    //Initialize the steps
    globalCurrentStepVec_ = {-1, -1, -1, -1};
    
    //Initialize parameters
    playVec_ = {0,0,0,0};
    pulseVec_ = {3,4,5,2};
    intervalVec_ = {8,8,8,8};
    //gainVec_ = {30,25, 25, 20};
    gainVec_ = {0.5f,0.5f, 0.5f, 0.5f};
    offsetVec_ = {4,1,3,2};
    frequencyVec_ = {6000,6000,6000,6000};
    resonanceVec_ = {0.5f,0.5f, 0.5f, 0.5f};
    
    notesVec_ = {60, 70, 80, 90};
    
    intervalSliderRangeVec_ = {{4,32},{4,32},{4,32},{4,32}};
    pulseSliderRangeVec_ = {{0,intervalVec_[0]},{0,intervalVec_[1]},{0,intervalVec_[2]},{0,intervalVec_[3]}};
    gainSliderRangeVec_ = {{0.0f,1.0f},{0.0f,1.0f},{0.0f,1.0f},{0.0f,1.0f}};
    offsetSliderRangeVec_ = {{0,4},{0,4},{0,4},{0,4}};
    frequencySliderRangeVec_ = {{20,20000},{10,20000},{10,20000},{10,20000}};
    resonanceSliderRangeVec_ = {{0.0f,2.0f},{0.0f,2.0f},{0.0f,2.0f},{0.0f,2.0f}};
    
    for(int row=0;row<numRows_;row++)
    {
        statesArray_.push_back({0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
    }
    
    
    stepTimeVec_ = {0.2,0.2,0.2,0.2};
    
    globalStepVec_ = {-1, -1, -1, -1};
    
    currentNoteVec_ = {-1, -1, -1, -1};
    currentNoteTimeVec_ = {-1, -1, -1, -1};
    
    //Initialize time signature (4/4)
    localTimeSigNumerator_ = 4;
    localTimeSignDenominator_ = 4;
    
    //Initialize step durations
    for(int row = 0;row < intervalVec_.size(); row++) {
        stepTimeVec_.push_back(computeStepDuration(row));
    }

}

StepSequencerData::~StepSequencerData()
{    
}

void StepSequencerData::createNextStep(int row, int step, float timeStamp)
{
    jassert(row<statesArray_.size());
    jassert(row<currentNoteVec_.size());
    jassert(row<currentNoteTimeVec_.size());
    jassert(step<statesArray_[row].size());
    
    if(statesArray_[row].at(step) == 1 && playVec_[row] == 1)
        currentNoteVec_[row] = notesVec_[row];
    else if(statesArray_[row].at(step) == 0 || playVec_[row] == 0)
        currentNoteVec_[row] = -1;
    
    currentNoteTimeVec_[row] = timeStamp;
}

void StepSequencerData::addNextStep(int row, int step)
{
    jassert(row<currentNoteTimeVec_.size());
    jassert(row<stepTimeVec_.size());
    createNextStep(row, step,currentNoteTimeVec_[row] + stepTimeVec_[row]);
}

//Check what is the time and compute the current step. If the step is more than the number of
//steps of the row, goes back to 0.
//The step is used by the editor to update the step visually in the step sequencer
//For each step createNextStep is called
//In the timer callback function, the editor updates the values of statesArray_, stepTimeVec_ and the number of steps, so the audio follows the changes from the user
void StepSequencerData::next(int row)
{
    jassert(row<statesArray_.size());
    jassert(row<globalStepVec_.size());
    jassert(row<currentNoteTimeVec_.size());
    
    int nbSteps = statesArray_[row].size();
    
    if(isPlaying())
    {
        //Check what is the current step.
        int currentStep = globalStepVec_[row];
        float currentTime = juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime_;
        
        int localStep = globalStepVec_[row];
        
        if(currentNoteTimeVec_[row] <= currentTime)
            localStep += 1;
        
        if(localStep>currentStep && localStep == nbSteps)
        {
            globalStepVec_[row] = 0;
            currentStep = -1;
            addNextStep(row, globalStepVec_[row]+1);
        }
        else if(localStep>currentStep && localStep == (nbSteps - 1))
        {
            globalStepVec_[row] = localStep;
            addNextStep(row, 0);
        }
        else if(localStep>currentStep)
        {
            globalStepVec_[row] = localStep;
            addNextStep(row, globalStepVec_[row]+1);
        }
        
    }
    
}


bool StepSequencerData::isPlaying()
{
    bool isPlaying = false;
    for(int row = 0;row < playVec_.size();row++)
    {
        if(playVec_[row] == 1)
            isPlaying = true;
    }
    return isPlaying;
}

double StepSequencerData::computeStepDuration(int row)
{
    jassert(row<intervalVec_.size());
    
    double duration = (60.0/(double)bpm_)* (localTimeSigNumerator_/localTimeSignDenominator_) * 4 / (double)intervalVec_[row];
    
    return duration;
}

int StepSequencerData::computeAbsoluteStep(int row)
{
    jassert(row<stepTimeVec_.size());
    
    float currentTime = juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime_;
    float timeDiff = currentTime - timeStamp_;
    
    //Absolute step computation
    float quotient = timeDiff/stepTimeVec_[row];
    float remainder = fmod(timeDiff,stepTimeVec_[row]);
    
    int absoluteStep;
    if(remainder != 0.0f)
        absoluteStep = ceil(quotient);
    else
        absoluteStep = floor(quotient);
    
    
    return absoluteStep;
    
}

//Getters/Setters
void StepSequencerData::setSampleRate(int samplerate)
{
    sampleRate_ = samplerate;
}

int StepSequencerData::getSampleRate()
{
    return sampleRate_;
}

void StepSequencerData::setStepTimeVec(const std::vector<double> &stepTimeVec)

{
    stepTimeVec_.clear();
    for(int kk=0;kk<stepTimeVec.size();kk++)
        stepTimeVec_.push_back(stepTimeVec[kk]);
}

void StepSequencerData::setStepTime(int row, double stepTime)
{
    jassert(row<stepTimeVec_.size());
    
    stepTimeVec_[row] = stepTime;
}

double StepSequencerData::getStepTime(int row)
{
    jassert(row<stepTimeVec_.size());
    
    return stepTimeVec_[row];
}

void StepSequencerData::setStatesArray(const std::vector< std::vector<bool> > &statesArray)
{
    statesArray_ = statesArray;
}

void StepSequencerData::setStates(int row, std::vector<bool> states)
{
    jassert(row<statesArray_.size());

    statesArray_[row] = states;
}

void StepSequencerData::setStartTime(float startTime)
{
    startTime_ = startTime;
}

float StepSequencerData::getStartTime()
{
    return startTime_;
}

void StepSequencerData::setTimeStamp(float timeStamp)
{
    timeStamp_ = timeStamp;
}

float StepSequencerData::getTimeStamp()
{
    return timeStamp_;
}


void StepSequencerData::setIntervalVec(const std::vector<int> &intervalVec)
{
    intervalVec_.clear();
    for(int kk=0;kk<intervalVec.size();kk++)
        intervalVec_.push_back(intervalVec[kk]);
}

void StepSequencerData::setIntervalVec(int row, int value)
{
    intervalVec_[row] = value;
}

std::vector<int> StepSequencerData::getIntervalVec()
{
    return intervalVec_;
}

void StepSequencerData::setInterval(int row, int interval)
{
    jassert(row<intervalVec_.size());
    
    intervalVec_[row] = interval;
}

int StepSequencerData::getInterval(int row)
{
    jassert(row<intervalVec_.size());
    
    return intervalVec_[row];
}

void StepSequencerData::setNotesVec(const std::vector<int> &notesVec)
{
    notesVec_.clear();
    for(int kk=0;kk<notesVec.size();kk++)
        notesVec_.push_back(notesVec[kk]);
}

std::vector<int> StepSequencerData::getNotesVec()
{
    return notesVec_;
}

void StepSequencerData::setNote(int row, int note)
{
    jassert(row<notesVec_.size());
    
    notesVec_[row] = note;
}

int StepSequencerData::getNote(int row)
{
    return notesVec_[row];
}

void StepSequencerData::setPlayVec(const std::vector<int> &playVec)
{
    playVec_.clear();
    for(int kk=0;kk<playVec.size();kk++)
        playVec_.push_back(playVec[kk]);
}

std::vector<int> StepSequencerData::getPlayVec() {
    return playVec_;
}

void StepSequencerData::setPulseVec(const std::vector<int> &pulseVec)
{
    pulseVec_.clear();
    for(int kk=0;kk<pulseVec.size();kk++)
        pulseVec_.push_back(pulseVec[kk]);
}

void StepSequencerData::setPulseVec(int row, int value)
{
    pulseVec_[row] = value;
}

std::vector<int> StepSequencerData::getPulseVec()
{
    return pulseVec_;
}

void StepSequencerData::setGainVec(const std::vector<float> &gainVec)
{
    gainVec_.clear();
    for(int kk=0;kk<gainVec.size();kk++)
        gainVec_.push_back(gainVec[kk]);
}

void StepSequencerData::setGainVec(int row, float value)
{
    gainVec_[row] = value;
}

std::vector<float> StepSequencerData::getGainVec()
{
    return gainVec_;
}

float StepSequencerData::getGain(int row)
{
    return gainVec_[row];
}

void StepSequencerData::setOffsetVec(int row, float value)
{
    offsetVec_[row] = value;
}

void StepSequencerData::setFrequencyVec(int row, int value)
{
    frequencyVec_[row] = value;
}

void StepSequencerData::setResonanceVec(int row, float value)
{
    resonanceVec_[row] = value;
}

std::vector<int> StepSequencerData::getOffsetVec()
{
    return offsetVec_;
}

std::vector<int> StepSequencerData::getFrequencyVec()
{
    return frequencyVec_;
}
std::vector<float> StepSequencerData::getResonanceVec()
{
    return resonanceVec_;
}

std::vector< std::vector<int> > StepSequencerData::getIntervalSliderRangeVec_()
{
    return intervalSliderRangeVec_;
}

std::vector< std::vector<int> > StepSequencerData::getPulseSliderRangeVec_()
{
    return pulseSliderRangeVec_;
}

std::vector< std::vector<float> > StepSequencerData::getGainSliderRangeVec_()
{
    return gainSliderRangeVec_;
}

std::vector< std::vector<int> > StepSequencerData::getOffsetSliderRangeVec_()
{
    return offsetSliderRangeVec_;
}

std::vector< std::vector<int> > StepSequencerData::getFrequencySliderRangeVec_()
{
    return frequencySliderRangeVec_;
}

std::vector< std::vector<float> > StepSequencerData::getResonanceSliderRangeVec_()
{
    return resonanceSliderRangeVec_;
}

int StepSequencerData::getPlay(int row)
{
    //Fix this jassert
    //jassert(row<playVec_.size());

    return playVec_[row];
}


void StepSequencerData::setPlay(int row, int play)
{
    //jassert(row<playVec_.size());
    
    playVec_[row] = play;
}

int StepSequencerData::getGlobalStep(int row)
{
    jassert(row<globalStepVec_.size());
    
    return globalStepVec_[row];
}

void StepSequencerData::setGlobalStep(int row, int step)
{
    jassert(row<globalStepVec_.size());
    
    globalStepVec_[row] = step;
}

int StepSequencerData::getCurrentNote(int row)
{
    jassert(row<currentNoteVec_.size());
    
    return currentNoteVec_[row];
}

float StepSequencerData::getCurrentNoteTime(int row)
{
    jassert(row<currentNoteTimeVec_.size());

    return currentNoteTimeVec_[row];
}

int StepSequencerData::getNumRows()
{
    return numRows_;
}

void StepSequencerData::setNumRows(int numRows)
{
    numRows_ = numRows;
}

void StepSequencerData::setBpm(int bpm)
{
    bpm_ = bpm;
}

int StepSequencerData::getBpm()
{
    return bpm_;
}

std::vector<int> StepSequencerData::getGlobalCurrentStepVec()
{
    return globalCurrentStepVec_;
}

void StepSequencerData::setNoRowOn(bool noRowOn)
{
    noRowOn_ = noRowOn;
}

bool StepSequencerData::getNoRowOn()
{
    return noRowOn_;
}

int StepSequencerData::getlocalTimeSigNumerator()
{
    return localTimeSigNumerator_;
}

void StepSequencerData::setlocalTimeSigNumerator(int localTimeSigNumerator)
{
    localTimeSigNumerator_ = localTimeSigNumerator;
}

int StepSequencerData::getlocalTimeSigDenominator()
{
    return localTimeSignDenominator_;
}

void StepSequencerData::setlocalTimeSigDenominator(int localTimeSigDenominator)
{
    localTimeSignDenominator_ = localTimeSigDenominator;
}

std::vector<bool> StepSequencerData::getStatesVec(int row)
{
    jassert(row<statesArray_.size());
    
    return(statesArray_[row]);
}

void StepSequencerData::isInitialized()
{
    initialized_ = true;
}

bool StepSequencerData::getInitialized()
{
    return initialized_;
}


