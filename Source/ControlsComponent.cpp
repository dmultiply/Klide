/*
  ==============================================================================

    ControlsComponent.cpp
    Created: 22 Nov 2017 5:48:54pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "ControlsComponent.h"

ControlsComponent::ControlsComponent()
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    //setSize (400, 100);
    
    //Open Buttons
    openButton_.setButtonText ("Open...");
    juce::Image folderButton = juce::ImageCache::getFromMemory (BinaryData::folder_png, BinaryData::folder_pngSize);
    openButton_.setImages (false, true, true, folderButton, 1.0f, {}, folderButton, 1.0f, {}, folderButton, 1.0f, {});
    //openButton_.setColour (juce::TextButton::buttonColourId, juce::Colour((juce::uint8)92,(juce::uint8)96,(juce::uint8)83,1.0f));
    
    addAndMakeVisible(openButton_);
    
    //Play Buttons
    playButton_.setButtonText ("Play");
    juce::Image normalButton = juce::ImageCache::getFromMemory (BinaryData::normalButton_png, BinaryData::normalButton_pngSize);
    juce::Image overButton = juce::ImageCache::getFromMemory (BinaryData::overButton_png, BinaryData::overButton_pngSize);
    juce::Image downButton = juce::ImageCache::getFromMemory (BinaryData::downButton_png, BinaryData::downButton_pngSize);
    playButton_.setImages (false, true, true, normalButton, 1.0f, {}, normalButton, 1.0f, {}, downButton, 1.0f, {});
    //playButton_.setColour (juce::TextButton::buttonColourId, juce::Colour((juce::uint8)43,(juce::uint8)56,(juce::uint8)47,1.0f));
    playButton_.setEnabled (true);
    
    addAndMakeVisible(playButton_);
    
    //Stop Buttons
    stopButton_.setButtonText ("Stop");
    juce::Image stopButton = juce::ImageCache::getFromMemory (BinaryData::stop_png, BinaryData::stop_pngSize);
    stopButton_.setImages (false, true, true, stopButton, 1.0f, {}, stopButton, 1.0f, {}, stopButton, 1.0f, {});
    //stopButton_.setColour (juce::TextButton::buttonColourId, juce::Colour((juce::uint8)227,(juce::uint8)73,(juce::uint8)69,1.0f));
    stopButton_.setEnabled (true);
    
    addAndMakeVisible(stopButton_);
    
    //Sliders
    
    intervalSlider_.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    intervalSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 45, 20);
    intervalSlider_.setNumDecimalPlacesToDisplay(0);
    intervalSlider_.setRange(8, 32);
    
    addAndMakeVisible (intervalSlider_);
    
    pulseSlider_.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    pulseSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 45, 20);
    pulseSlider_.setNumDecimalPlacesToDisplay(0);
    pulseSlider_.setRange(0, 16);
    
    addAndMakeVisible (pulseSlider_);
    
    gainSlider_.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    gainSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 45, 20);
    gainSlider_.setNumDecimalPlacesToDisplay(0);
    gainSlider_.setRange(0.001, 5.0, 0.01);
    gainSlider_.setRange(0.0,1.0);
    
    addAndMakeVisible (gainSlider_);
    
    offsetSlider_.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    offsetSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 45, 20);
    offsetSlider_.setNumDecimalPlacesToDisplay(0);
    offsetSlider_.setRange(0.001, 5.0, 0.01);
    offsetSlider_.setRange(0,4,1);
    
    addAndMakeVisible (offsetSlider_);
    
    frequencySlider_.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    frequencySlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 45, 20);
    //frequencySlider_.setNumDecimalPlacesToDisplay(0);
    frequencySlider_.setRange(20,20000);
    frequencySlider_.setSkewFactor(0.2);
    
    addAndMakeVisible (frequencySlider_);
    
    resonanceSlider_.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    resonanceSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 45, 20);
    //resonanceSlider_.setNumDecimalPlacesToDisplay(0);
    resonanceSlider_.setRange(0.0f,2.0f);
    resonanceSlider_.setSkewFactor(0.2);
    
    addAndMakeVisible (resonanceSlider_);

}

ControlsComponent::~ControlsComponent()
{
}

//==============================================================================
void ControlsComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (juce::Colour((juce::uint8)44,(juce::uint8)53,(juce::uint8)57,1.0f));
    
    //g.setColour (juce::Colour((juce::uint8)34,(juce::uint8)43,(juce::uint8)47,1.0f));
    g.setFont (15.0f);
}


void ControlsComponent::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    juce::Rectangle<int> area = getLocalBounds();
    
    const int buttonsWidth = area.getWidth()*0.3;
    const int controlsSideMargin = area.getWidth()/350;
    
    area.removeFromTop(area.getHeight()*1/4);
    area.removeFromBottom(area.getHeight()*1/4);
    
    openButton_.setBounds(area.removeFromLeft(buttonsWidth-10)
                          .reduced(controlsSideMargin));
    playButton_.setBounds(area.removeFromLeft(buttonsWidth)
                          .reduced(controlsSideMargin));
    stopButton_.setBounds(area.removeFromLeft(buttonsWidth-20)
                          .reduced(controlsSideMargin));

    /* To put back if i want to put back the sliders
    //Seq 1
    const int buttonsWidth = area.getWidth()*7/65;
    const int controlsSideMargin = area.getWidth()/350;
    
    const int slidersWidth = area.getWidth()*7/65;
    const int slidersSideMargin = area.getWidth()/350;
    
    //Buttons
    juce::Rectangle<int> buttonsArea = area.removeFromLeft(area.getWidth()*1/3);
    buttonsArea.removeFromTop(buttonsArea.getHeight()*1/4);
    buttonsArea.removeFromBottom(buttonsArea.getHeight()*1/4);
    
    openButton_.setBounds(buttonsArea.removeFromLeft(buttonsWidth-10)
                                  .reduced(controlsSideMargin));
    playButton_.setBounds(buttonsArea.removeFromLeft(buttonsWidth)
                                  .reduced(controlsSideMargin));
    stopButton_.setBounds(buttonsArea.removeFromLeft(buttonsWidth-20)
                                  .reduced(controlsSideMargin));
    
    
    //Sliders
    area.removeFromLeft(area.getWidth()*1/20);
    intervalSlider_.setBounds(area.removeFromLeft(slidersWidth)
                                  .reduced(slidersSideMargin));
    pulseSlider_.setBounds(area.removeFromLeft(slidersWidth)
                               .reduced(slidersSideMargin));
    gainSlider_.setBounds(area.removeFromLeft(slidersWidth)
                          .reduced(slidersSideMargin));
    offsetSlider_.setBounds(area.removeFromLeft(slidersWidth)
                          .reduced(slidersSideMargin));
    frequencySlider_.setBounds(area.removeFromLeft(slidersWidth)
                            .reduced(slidersSideMargin));
    resonanceSlider_.setBounds(area.removeFromLeft(slidersWidth)
                            .reduced(slidersSideMargin));
     */

}

void ControlsComponent::setIntervalSliderVal(int val)
{
    intervalSlider_.setValue(val);
}

void ControlsComponent::setPulseSliderVal(int val)
{
    pulseSlider_.setValue(val);
}

void ControlsComponent::setGainSliderVal(float val)
{
    gainSlider_.setValue(val);
}

void ControlsComponent::setOffsetSliderVal(int val)
{
    offsetSlider_.setValue(val);
}

void ControlsComponent::setFrequencySliderVal(int val)
{
    frequencySlider_.setValue(val);
}

void ControlsComponent::setResonanceSliderVal(int val)
{
    resonanceSlider_.setValue(val);
}

//Ranges

void ControlsComponent::setIntervalSliderRange(int min, int max)
{
    intervalSlider_.setRange(min, max);
}

void ControlsComponent::setPulseSliderRange(int min, int max)
{
    pulseSlider_.setRange(min, max);
}

void ControlsComponent::setGainSliderRange(double min, double max)
{
    gainSlider_.setRange(min, max);
}

void ControlsComponent::setOffsetSliderRange(int min, int max)
{
    offsetSlider_.setRange(min, max);
}

void ControlsComponent::setFrequencySliderRange(int min, int max)
{
    frequencySlider_.setRange(min, max);
}

void ControlsComponent::setResonanceSliderRange(float min, float max)
{
    resonanceSlider_.setRange(min, max);
}

int ControlsComponent::getPulseSliderVal()
{
    return pulseSlider_.getValue();
}

