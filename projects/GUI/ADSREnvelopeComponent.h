#pragma once

#include "juce_graphics/juce_graphics.h"
#include <JuceHeader.h>

class ADSREnvelopeComponent
: public juce::Component
, private juce::Timer
{
public:
    ADSREnvelopeComponent();
    ~ADSREnvelopeComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown  (const juce::MouseEvent& e) override;
    void mouseDrag  (const juce::MouseEvent& e) override;
    void mouseUp    (const juce::MouseEvent& e) override;

private:

    static constexpr float MAX_LENGTH { 4000.0f }; // in milliseconds - how many ms corresponds to the full width
    static constexpr float handleRadius{6.0f};

    void setupSlider (juce::Slider& slider, double min, double max, double def);
    void timerCallback() override;

    void drawBackground(juce::Graphics& g, juce::Rectangle<float> area);
    void drawEnvelope(juce::Graphics& g, juce::Rectangle<float> area,
                      float a, float d, float s, float r);
    void drawPlayhead(juce::Graphics& g, juce::Rectangle<float> area,
                      float a, float d, float s, float r);
    void drawHandles(juce::Graphics& g);

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    juce::Path path;
    juce::Array<juce::Point<float>> points;
    
    int draggingPoint { -1 };

    double startTime;
    float currentPhaseTime {0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADSREnvelopeComponent)
};
