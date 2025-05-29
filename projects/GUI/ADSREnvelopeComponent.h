#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "juce_graphics/juce_graphics.h"
#include <JuceHeader.h>

class ADSREnvelopeComponent
: public juce::Component
, private juce::Timer
{
public:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    ADSREnvelopeComponent(
        juce::AudioProcessorValueTreeState& state,
        const juce::String& attackID,
        const juce::String& decayID,
        const juce::String& sustainID,
        const juce::String& releaseID
    );
    ~ADSREnvelopeComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown  (const juce::MouseEvent& e) override;
    void mouseDrag  (const juce::MouseEvent& e) override;
    void mouseUp    (const juce::MouseEvent& e) override;
    void mouseMove  (const juce::MouseEvent& e) override;
    void mouseExit  (const juce::MouseEvent& e) override;

private:

    static constexpr float MAX_LENGTH { 4000.0f }; // in milliseconds - how many ms corresponds to the full width
    static constexpr float handleRadius{6.0f};

    void setupSlider (juce::Slider& slider);
    void timerCallback() override;

    void drawBackground(juce::Graphics& g, juce::Rectangle<float> area);
    void drawEnvelope(juce::Graphics& g, juce::Rectangle<float> area,
                      float a, float d, float s, float r);
    void drawPlayhead(juce::Graphics& g, juce::Rectangle<float> area,
                      float a, float d, float s, float r);
    void drawHandles(juce::Graphics& g);

    float getYForX (float xQuery) const;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    std::unique_ptr<Attachment> attackAttachment;
    std::unique_ptr<Attachment> decayAttachment;
    std::unique_ptr<Attachment> sustainAttachment;
    std::unique_ptr<Attachment> releaseAttachment;

    juce::Path path;

    juce::Array<juce::Point<float>> points;
    juce::Array<juce::Point<float>> controlPoints;  
    juce::Array<juce::Point<float>> controlPointOffsets;

    int draggingPoint { -1 };

    double startTime;
    float currentPhaseTime {0};
    bool isMouseOverEnvelopeArea = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADSREnvelopeComponent)
};
