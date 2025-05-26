#pragma once

#include "juce_graphics/juce_graphics.h"
#include <JuceHeader.h>

class ADSREnvelopeComponent
: public juce::Component
{
public:
    ADSREnvelopeComponent();
    ~ADSREnvelopeComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown  (const juce::MouseEvent& e) override;
    void mouseDrag  (const juce::MouseEvent& e) override;
    void mouseUp    (const juce::MouseEvent& e) override;

    static constexpr float MAX_LENGTH { 3000.0f }; // in milliseconds - how many ms corresponds to the full width

private:
    void setupSlider (juce::Slider& slider, double min, double max, double def);

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Path path;
    juce::Path backgroundPath;

    juce::Array<juce::Point<float>> points;
    int draggingPoint { -1 };
    static constexpr float handleRadius = 6.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADSREnvelopeComponent)
};
