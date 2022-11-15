/*
  ==============================================================================

    CustomKnob.cpp
    Created: 10 Nov 2022 6:12:27pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "CustomKnob.h"

#include <JuceHeader.h>


//==============================================================================
CustomKnob::CustomKnob() : Slider()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setLookAndFeel(&customKnobLookAndFeel);
}

CustomKnob::~CustomKnob()
{
    setLookAndFeel(nullptr);
}

void CustomKnob::mouseDown(const juce::MouseEvent& event)
{
    Slider::mouseDown(event);
    setMouseCursor(juce::MouseCursor::NoCursor);
    mousePosition = juce::Desktop::getMousePosition();
}

void CustomKnob::mouseUp(const juce::MouseEvent& event)
{
    Slider::mouseUp(event);
    juce::Desktop::setMousePosition(mousePosition);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}
