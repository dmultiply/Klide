/*
  ==============================================================================

    SequenceGenerator.h
    Created: 21 Nov 2017 11:44:35am
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include <vector>
#include <bitset>
#include "../JuceLibraryCode/JuceHeader.h"

class SequenceGenerator
{
public:
    SequenceGenerator();
    ~SequenceGenerator();
    
    void build_string (int level, bool reverse);
    void compute_bitmap (int num_slots, int num_pulses);
    void rotateSequence(int numSteps);
    void offsetSequence(int offset);
    
    //Getters/Setters
    std::vector<bool> getSequence();
    void setSequence(std::vector<bool>& sequence);
    
private:
    std::vector<bool> sequence_;
    std::vector<int> count_;
    std::vector<int> remainder_;

};

