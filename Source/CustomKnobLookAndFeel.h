/*
  ==============================================================================

    CustomKnobLookAndFeel.h
    Created: 10 Nov 2022 6:15:01pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class CustomKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomKnobLookAndFeel();
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
    void drawLabel(juce::Graphics& g, juce::Label& label);
    
private:
    juce::Image myKnobImage;
};
