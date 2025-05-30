#pragma once

#include <JuceHeader.h>

namespace GUI
{

class WavetablePlotComponent : public juce::Component
{
public:
    WavetablePlotComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void generateWavetables();

    static constexpr int sampleSize = 128;
    static constexpr int wavetableCount = 4;

    std::vector<std::vector<float>> wavetables;
    juce::Colour waveformColours[wavetableCount] = {
        juce::Colours::cyan,
        juce::Colours::yellow,
        juce::Colours::limegreen,
        juce::Colours::magenta
    };
};

}