#include "ModernLookAndFeel.h"
#include "juce_graphics/juce_graphics.h"

void ModernLookAndFeel::drawRotarySlider(
    Graphics &g, 
    int x, int y,
    int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    Slider &slider
)
{
    auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (10);
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineWidth = jmin (4.0f, radius * 0.5f);
    auto arcRadius = radius - lineWidth * 0.5f;

    auto knobRadius = radius - lineWidth;
    auto rx = centreX - knobRadius;
    auto ry = centreY - knobRadius;
    auto rw = knobRadius * 2.0f;

    auto outline = slider.findColour (Slider::rotarySliderOutlineColourId);
    auto fill    = slider.findColour (Slider::rotarySliderFillColourId);

    // fill
    g.setColour (fill);
    g.fillEllipse (rx, ry, rw, rw);
    // outline
    g.setColour (outline);
    g.drawEllipse (rx, ry, rw, rw, 1.0f);

    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getCentreY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);

    g.setColour (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineWidth, PathStrokeType::curved, PathStrokeType::square));

    if (slider.isEnabled())
    {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(),
                                bounds.getCentreY(),
                                arcRadius,
                                arcRadius,
                                0.0f,
                                rotaryStartAngle,
                                angle,
                                true);

        g.setColour (juce::Colours::blueviolet);
        g.strokePath (valueArc, PathStrokeType (lineWidth, PathStrokeType::curved, PathStrokeType::square));
    }

    juce::Path p;
    auto pointerLength = knobRadius * 0.66f;
    auto pointerThickness = 2.0f;
    p.addRectangle (-pointerThickness * 0.5f, -knobRadius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

    // pointer
    g.setColour (juce::Colours::white);
    g.fillPath (p);
}