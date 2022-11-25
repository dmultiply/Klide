/*
  ==============================================================================

    StepSequencerData.h
    Created: 28 Dec 2017 4:18:27pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class StepSequencerData
{
public:
    StepSequencerData();
    ~StepSequencerData();
    
    //Steps
    void createNextStep(int row, int step, float timeStamp);
    void next(int row);
        
    void addNextStep(int row, int step);
    void checkStep(const std::vector< std::vector<bool> > &statesArray, std::vector<double> stepTimeVec, int nbSteps, int row);    
    
    //Is playing
    bool isPlaying();
    
    //Compute Steps
    double computeStepDuration(int row);
    int computeAbsoluteStep(int row);
    
    //Getters/Setters
    void setSampleRate(int samplerate);
    int getSampleRate();
    void setStepTimeVec(const std::vector<double> &stepTimeVec);
    void setStepTime(int row, double stepTime);
    double getStepTime(int row);
    
    //States
    void setStatesArray(const std::vector< std::vector<bool> > &statesArray);
    
    void setRestartArray(const std::vector< std::vector<bool> > &restartArray);
    std::vector<bool> getRestartVec(int row);

    std::vector<bool> getStatesVec(int row);
    void setStates(int row, std::vector<bool> states);
    
    //StartTime
    void setStartTime(float startTime);
    float getStartTime();
    
    //TimeStamp
    void setTimeStamp(float timeStamp);
    float getTimeStamp();
    
    //Intervals
    void setIntervalVec(const std::vector<int> &intervalVec);
    void setIntervalVec(int row, int value);
    std::vector<int> getIntervalVec();
    void setInterval(int row, int interval);
    int getInterval(int row);
    
    //Notes
    void setNotesVec(const std::vector<int> &notesVec);
    std::vector<int> getNotesVec();
    void setNote(int row, int note);
    int getNote(int row);
    
    //Play
    void setPlayVec(const std::vector<int> &playVec);
    std::vector<int> getPlayVec();
    
    //Pulse
    void setPulseVec(const std::vector<int> &pulseVec);
    void setPulseVec(int row, int value);
    std::vector<int> getPulseVec();
    
    //Gain
    void setGainVec(const std::vector<float> &gainVec);
    void setGainVec(int row, float value);
    std::vector<float> getGainVec();
    float getGain(int row);
    
    //Offset
    void setOffsetVec(int row, float value);
    std::vector<int> getOffsetVec();
    
    //Filter
    std::vector<int> getFrequencyVec();
    void setFrequencyVec(int row, int value);
    std::vector<float> getResonanceVec();
    void setResonanceVec(int row, float value);
    
    //Get Range Vec
    std::vector< std::vector<int> > getIntervalSliderRangeVec_();
    std::vector< std::vector<int> > getPulseSliderRangeVec_();
    std::vector< std::vector<float> > getGainSliderRangeVec_();
    std::vector< std::vector<int> > getOffsetSliderRangeVec_();
    std::vector< std::vector<int> > getFrequencySliderRangeVec_();
    std::vector< std::vector<float> > getResonanceSliderRangeVec_();
    
    //Play
    void setPlay(int row, int play);
    int getPlay(int row);
    
    //Global Step
    int getGlobalStep(int row);
    void setGlobalStep(int row, int step);
    
    //Global Current Step Vec
    std::vector<int> getGlobalCurrentStepVec();
    
    //Current Note
    int getCurrentNote(int row);
    float getCurrentNoteTime(int row);
    
    //Num rows
    int getNumRows();
    void setNumRows(int numRows);
    
    //No row on
    void setNoRowOn(bool noRowOn);
    bool getNoRowOn();
    
    //BPM
    void setBpm(int bpm);
    int getBpm();
    
    //Local Time signature
    int getlocalTimeSigNumerator();
    void setlocalTimeSigNumerator(int localTimeSigNumerator);
    int getlocalTimeSigDenominator();
    void setlocalTimeSigDenominator(int localTimeSigDenominator);
    
    //Initialized
    bool getInitialized();
    void isInitialized();
    
private:    
    int sampleRate_;
    int numRows_;
    int bpm_;
    
    std::vector< std::vector<bool> > statesArray_;
    std::vector< std::vector<bool> > restartArray_;
    
    std::vector<double> stepTimeVec_;
    std::vector<int> globalStepVec_;

    std::vector<int> intervalVec_;
    std::vector<int> playVec_;
    std::vector<int> notesVec_;
    std::vector<int> pulseVec_;
    std::vector<float> gainVec_;
    std::vector<int> offsetVec_;
    std::vector<int> frequencyVec_;
    std::vector<float> resonanceVec_;
    
    std::vector< std::vector<int> > intervalSliderRangeVec_;
    std::vector< std::vector<int> > pulseSliderRangeVec_;
    std::vector< std::vector<float> > gainSliderRangeVec_;
    std::vector< std::vector<int> > offsetSliderRangeVec_;
    std::vector< std::vector<int> > frequencySliderRangeVec_;
    std::vector< std::vector<float> > resonanceSliderRangeVec_;
    
    bool syncOn_;
    float startTime_;
    float timeStamp_;
    bool noRowOn_;
    
    std::vector<int> globalCurrentStepVec_;
    
    std::vector<int> currentNoteVec_;
    std::vector<float> currentNoteTimeVec_;
    
    //Time signature
    int localTimeSigNumerator_;
    int localTimeSignDenominator_;
    
    //check if the StepSequencerData has been launched one time already
    bool initialized_ { false };
    
    
    
};
