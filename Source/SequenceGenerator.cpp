/*
  ==============================================================================

    SequenceGenerator.cpp
    Created: 21 Nov 2017 11:44:35am
    Author:  Mael Derio

  ==============================================================================
*/

#include "SequenceGenerator.h"

SequenceGenerator::SequenceGenerator()
{
    //sequence_ = {1, 1, 0, 0};
    remainder_ = {0,0,0,0};
    count_ = {0,0,0,0};;
}

SequenceGenerator::~SequenceGenerator()
{
    
}

void SequenceGenerator::build_string (int level,bool reverse)
{
    //Bjorklund Algorithm is modified to put the ones first
    
    bool a = 0;
    bool b = 1;
    
    if(reverse == 1)
    {
        a = 1;
        b = 0;
    }
    
    if (level == -1)
        sequence_.push_back(a);
    else if (level == -2)
        sequence_.push_back(b);
    else {
        for (int i=0; i < count_[level]; i++)
            build_string (level-1, reverse);
        if (remainder_[level] != 0)
            build_string (level-2, reverse);
    }/*end else*/
}

void SequenceGenerator::compute_bitmap (int num_slots, int num_pulses)
{
    /*---------------------
     * First, compute the count and remainder arrays
     */
    //Algorithm inspired from : "The Theory of Rep-Rate Pattern Generation
    //in the SNS Timing System", E. Bjorklund, 1999
    
    //The algorithm works if num_pulses <= num_slots/2
    //Beyond this, the algorithm is the same but starting with ones and filling with zeros
    //So it's just reversed
    
    bool reverse = 0;
    
    if(num_pulses<=((float)num_slots/2.0f))
        reverse = 0;
    else
        reverse = 1;
    
    if(reverse == 1)
        num_pulses = num_slots - num_pulses;
    
    int divisor = num_slots - num_pulses;
    remainder_[0] = num_pulses;
    int level = 0;
    
    sequence_.clear();
    
    //Special cases : nbPulses = 0, or nbPulses = num_slots
    if(remainder_[0] == 0 && reverse == 0)
    {
        for(int kk=0;kk<num_slots;kk++)
            sequence_.push_back(0);
        return;
    }
    else if(remainder_[0] == 0 && reverse == 1)
    {
        for(int kk=0;kk<num_slots;kk++)
            sequence_.push_back(1);
        return;
    }
        
    //General case
    do {
        count_[level] = divisor / remainder_[level];
        remainder_[level+1] = divisor % remainder_[level];
        divisor = remainder_[level];
        level = level + 1;
    } while (remainder_[level] > 1);
    
        count_[level] = divisor;
        /*---------------------
         * Now build the bitmap string
         */
        build_string (level, reverse);

    
    //Put the first bit with value = 1 at the beginning of the sequence
    int step = 0;
    while(sequence_[step] != 1)
        step++;
    
    rotateSequence(step);
}

void SequenceGenerator::rotateSequence(int numSteps)
{
    
    std::rotate(sequence_.begin(),sequence_.begin()+numSteps,sequence_.end());
}

void SequenceGenerator::offsetSequence(int offset)
{
    
    //Put the first bit with value = 1 at the beginning of the sequence
    int step = 0;
    while(sequence_[step] != 1)
        step++;
    
    rotateSequence(step);
    
    //put the offset
    rotateSequence(offset);
}


//Getters/Setters
std::vector<bool> SequenceGenerator::getSequence()
{
    return sequence_;
}

void SequenceGenerator::setSequence(std::vector<bool>& sequence)
{
    sequence_.clear();
    for(int kk=0;kk<sequence.size();kk++)
        sequence_.push_back(sequence[kk]);
}

