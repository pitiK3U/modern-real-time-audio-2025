#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "EnvelopeStateCollector.h"
#include <JuceHeader.h>

namespace GUI
{

struct EnvelopePoint {
    float time;
    float level;

    juce::Point<float> toPixel(const juce::Rectangle<float>& area, float maxLength) const {
        float x = area.getX() + (time / maxLength) * area.getWidth();
        float y = juce::jmap(level, 0.0f, 1.0f, area.getBottom(), area.getY());
        return { x, y };
    }

    juce::Point<float> toPixelOffset(const juce::Rectangle<float>& area, float maxLength) const {
        float x = (time / maxLength) * area.getWidth();
        float y = -level * area.getHeight(); // invert Y
        return { x, y };
    }

    static EnvelopePoint fromPixel(const juce::Point<float>& pixel, const juce::Rectangle<float>& area, float maxLength)
    {
        float t = ((pixel.x - area.getX()) / area.getWidth()) * maxLength;
        float l = juce::jlimit(0.0f, 1.0f, (area.getBottom() - pixel.y) / area.getHeight()); // invert Y
        return { t, l };
    }
};

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

    EnvelopePoint getEnvelopePoint (int idx) const;
    EnvelopePoint getControlPoint (int idx) const;

    void checkControlPoints();
    void saveControlPointSliderValue(int index, EnvelopePoint value);

    // Sliders for ADSR envelope parameters in real values
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    // Curve sliders save bezier control points for the envelope in real values
    juce::Slider attackCurveXSlider, attackCurveYSlider, decayCurveYSlider, decayCurveXSlider, releaseCurveXSlider, releaseCurveYSlider;

    std::unique_ptr<Attachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;
    std::unique_ptr<Attachment> attackCurveXAttachment, attackCurveYAttachment, decayCurveXAttachment, decayCurveYAttachment, releaseCurveXAttachment, releaseCurveYAttachment;

    juce::Path path;

    juce::Array<juce::Point<float>> points; // Envelope ADSR points in pixel coordinates
    juce::Array<juce::Point<float>> controlPoints; // Control points in pixel coordinates
    juce::Array<juce::Point<float>> controlPointOffsets; // Offsets for control points to maintain their position relative to the envelope in pixel coordinates

    int draggingPoint { -1 };

    double startTime;
    float currentPhaseTime {0};
    bool isMouseOverEnvelopeArea = false;
    float maxLength; // how many ms corresponds to the full width

    Rectangle<float> envelopeArea = {}; // area where the envelope is drawn in pixel coordinates

    DSP::EnvelopeStateCollector* envelopeStateCollector = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADSREnvelopeComponent)
};

}