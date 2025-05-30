#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "EnvelopeStateCollector.h"
#include <JuceHeader.h>

namespace GUI
{

class ADSREnvelopeComponent
: public juce::Component
, private juce::Timer
{
public:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    ADSREnvelopeComponent(
        juce::AudioProcessorValueTreeState& state,
        DSP::EnvelopeStateCollector* envelopeStateCollector,
        const juce::String& attackID,
        const juce::String& decayID,
        const juce::String& sustainID,
        const juce::String& releaseID,
        const juce::String& attackCurveXID,
        const juce::String& attackCurveYID,
        const juce::String& decayCurveXID,
        const juce::String& decayCurveYID,
        const juce::String& releaseCurveXID,
        const juce::String& releaseCurveYID,
        float maxLength
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
    juce::Slider attackCurveXSlider, attackCurveYSlider, decayCurveYSlider, decayCurveXSlider, releaseCurveXSlider, releaseCurveYSlider;

    std::unique_ptr<Attachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;
    std::unique_ptr<Attachment> attackCurveXAttachment, attackCurveYAttachment, decayCurveXAttachment, decayCurveYAttachment, releaseCurveXAttachment, releaseCurveYAttachment;

    juce::Path path;

    juce::Array<juce::Point<float>> points;
    juce::Array<juce::Point<float>> controlPoints;  
    juce::Array<juce::Point<float>> controlPointOffsets;

    int draggingPoint { -1 };

    double startTime;
    float currentPhaseTime {0};
    bool isMouseOverEnvelopeArea = false;
    float maxLength; // how many ms corresponds to the full width

    DSP::EnvelopeStateCollector* envelopeStateCollector = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADSREnvelopeComponent)
};

struct EnvelopePoint {
    float time;
    float value;

    juce::Point<float> toPixel(const juce::Rectangle<float>& area, float maxLength) const {
        float x = area.getX() + (time / maxLength) * area.getWidth();
        float y = juce::jmap(value, 0.0f, 1.0f, area.getBottom(), area.getY());
        return { x, y };
    }
};

}