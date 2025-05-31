#pragma once

#include "juce_dsp/juce_dsp.h"
#include <JuceHeader.h>

namespace GUI
{

struct BoxCorners
{
    juce::Point<float> frontBottomLeft;
    juce::Point<float> frontBottomRight;
    juce::Point<float> frontTopRight;
    juce::Point<float> frontTopLeft;

    juce::Point<float> backBottomLeft;
    juce::Point<float> backBottomRight;
    juce::Point<float> backTopRight;
    juce::Point<float> backTopLeft;

    BoxCorners static getBoxCorners(
        juce::Rectangle<float> area,
        float xOffset,
        float yOffset,
        float scaleX,
        float scaleY,
        juce::Point<float> anchor,
        int depthCount
    );
};

class WavetablePlotComponent
: public juce::Component
, private juce::Timer
{
public:
    WavetablePlotComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;
    static constexpr int wavetableCount = 4;

private:
    void generateWavetables();

    void drawPlotBackground(juce::Graphics& g, juce::Rectangle<float> area);
    void drawWaveforms(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        float scaleX,
        float scaleYFactor,
        float xOffset,
        float yOffset
    );

    void drawSingleWaveform(
    juce::Graphics& g,
    int index,
    juce::Rectangle<float> area,
    float midY,
    float scaleY,
    float stepX,
    float scaleX,
    float scaleYFactor,
    float xOffset,
    float yOffset
    );

    void fillFace(juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Point<float> c, juce::Point<float> d, juce::Colour colour);
    void drawBackBoxFaces(juce::Graphics& g, const BoxCorners& c);
    void drawFrontBoxFaces(juce::Graphics& g, const BoxCorners& c);
    std::vector<float> generateMorphedWaveform(float t);
    void drawMorphedWaveform( juce::Graphics& g, juce::Rectangle<float> area, float midY, float scaleY, float stepX);
    void timerCallback() override;
    float naive_lerp(float a, float b, float t);

    static constexpr int sampleSize = 128;

    float morphT = 0.0f;

    std::vector<std::vector<float>> wavetables;
    juce::Colour waveformColours[wavetableCount] = {
        juce::Colours::cyan,
        juce::Colours::yellow,
        juce::Colours::limegreen,
        juce::Colours::magenta
    };
};

}