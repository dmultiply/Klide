/*
  ==============================================================================

    DragNDropButton.h
    Created: 26 Nov 2022 7:33:31pm
    Author:  Mael Derio

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomKnobLookAndFeel.h"

//==============================================================================
/*
 */
class DragNDropButton : public juce::ImageButton, public juce::FileDragAndDropTarget
{
public:
    DragNDropButton();
    ~DragNDropButton();
    void filesDropped (const StringArray &files, int x, int y) override;
    
private:
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragNDropButton)
};
