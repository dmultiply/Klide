/*
  ==============================================================================

    CustomKnob.h
    Created: 10 Nov 2022 6:12:27pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomKnobLookAndFeel.h"

//==============================================================================
/*
 */
class CustomKnob : public juce::Slider
{
public:
    CustomKnob();
    ~CustomKnob();
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    
    
private:
    CustomKnobLookAndFeel customKnobLookAndFeel;
    juce::Point<int> mousePosition;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomKnob)
};
