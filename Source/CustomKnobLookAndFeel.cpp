/*
  ==============================================================================

    CustomKnobLookAndFeel.cpp
    Created: 10 Nov 2022 6:15:01pm
    Author:  Mael Derio

  ==============================================================================
*/

#include "CustomKnobLookAndFeel.h"
#include <JuceHeader.h>

using namespace juce;

//==============================================================================
CustomKnobLookAndFeel::CustomKnobLookAndFeel() : LookAndFeel_V4()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    myKnobImage = juce::ImageCache::getFromMemory(BinaryData::metallicKnob_png, BinaryData::metallicKnob_pngSize);
    
}

void CustomKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    /* First One, drawn
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // fill
    g.setColour (juce::Colours::black);
    g.fillEllipse (rx, ry, rw, rw);
    
    // outline
    g.setColour (juce::Colours::green);
    g.drawEllipse (rx, ry, rw, rw, 1.0f);
    
    juce::Path p;
    auto pointerLength = radius * 0.33f;
    auto pointerThickness = 2.0f;
    p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    
    // pointer
    g.setColour (juce::Colours::green);
    g.fillPath (p);
     
     */
    
    //Second one : image
    /*
    const double rotation = (slider.getValue()
                             - slider.getMinimum())
    / (slider.getMaximum()
       - slider.getMinimum());
    
    const int frames = myKnobImage.getHeight() / myKnobImage.getWidth();
    const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
    const float radius = jmin(width / 2.0f, height / 2.0f);
    const float centerX = x + width * 0.5f;
    const float centerY = y + height * 0.5f;
    const float rx = centerX - radius - 1.0f;
    const float ry = centerY - radius;
    
    g.drawImage(myKnobImage,
                (int)rx,
                (int)ry,
                2 * (int)radius,
                2 * (int)radius,
                0,
                frameId*myKnobImage.getWidth(),
                myKnobImage.getWidth(),
                myKnobImage.getWidth());
     
     */
    
    //Third one : drawn
    
    const float radius = jmin(width / 2, height / 2) *0.5f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    float fwidth = (float)width;
    float fheight = (float)height;
    const float angle = rotaryStartAngle
    + sliderPos
    * (rotaryEndAngle - rotaryStartAngle);
    
    g.setColour(Colour(0xff39587a));
    Path filledArc;
    filledArc.addPieSegment(rx, ry, rw + 1, rw + 1, rotaryStartAngle, rotaryEndAngle, 0.6);
    
    g.fillPath(filledArc);
    
    g.setColour(Colours::lightgreen);
    Path filledArc1;
    filledArc1.addPieSegment(rx, ry, rw + 1, rw + 1, rotaryStartAngle, angle, 0.6);
    
    g.fillPath(filledArc1);
    
    Path p;
    const float pointerLength = radius * 0.63f;
    const float pointerThickness = radius * 0.2f;
    p.addRectangle(-pointerThickness * 0.5f, -radius - 1, pointerThickness, pointerLength);
    p.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(Colour(0xff39587a));
    g.fillPath(p);
    
    const float dotradius = radius * (float)0.4;
    const float dotradius2 = rw * (float)0.4;
    g.setColour(Colour(0xff39587a));
    g.fillEllipse(centreX - (dotradius),
                  centreY - (dotradius),
                  dotradius2, dotradius2);
    
}

void CustomKnobLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    //g.setColour(juce::Colour(juce::uint8(255), 255, 255, 1.0f));
    g.setColour(juce::Colours::grey);
    g.drawRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f,2.0f);
    
    
    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font(getLabelFont(label));
        
        //g.setColour(juce::Colour(juce::uint8(0), 0, 0, 1.0f));
        g.setColour(juce::Colours::white);
        
        g.setFont(font);
        
        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
        
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         fmax(1, (int)(textArea.getHeight() / font.getHeight())),
                         label.getMinimumHorizontalScale());
        
        g.setColour(juce::Colour(juce::uint8(255), 255, 255, 0.1f));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(juce::Label::outlineColourId));
    }
    
    //g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f);
}
