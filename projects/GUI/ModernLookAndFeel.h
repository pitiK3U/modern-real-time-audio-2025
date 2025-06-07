#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>

class ModernLookAndFeel : public LookAndFeel_V4
{
public:
    void drawRotarySlider(Graphics &, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, Slider &) override;
private:
};