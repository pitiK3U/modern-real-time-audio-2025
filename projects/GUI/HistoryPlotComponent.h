#pragma once
#include <JuceHeader.h>
#include <vector>

/// A component that keeps the last N values you feed it and draws them as a scrolling waveform.
class HistoryPlotComponent  : public juce::Component
{
public:
    /** @param initialBufferSize  how many samples to remember */
    explicit HistoryPlotComponent (int initialBufferSize = 512);
    ~HistoryPlotComponent() override = default;

    /** Push one new value in (−1…1).  Oldest value is dropped once full. */
    void addValue (float newValue);

    /** Change how many samples are stored. */
    void setBufferSize (int newSize);

    void paint (juce::Graphics& g) override;

private:
    int bufferSize, writeIndex = 0;
    bool bufferFull = false;
    std::vector<float> buffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryPlotComponent)
};
