#include "HistoryPlotComponent.h"

HistoryPlotComponent::HistoryPlotComponent (int initialBufferSize)
  : bufferSize (initialBufferSize),
    buffer     (static_cast<size_t> (initialBufferSize), 0.0f)
{
}

void HistoryPlotComponent::addValue (float newValue)
{
    // clamp to [–1,1] just in case
    newValue = juce::jlimit (-1.0f, 1.0f, newValue);

    buffer[writeIndex] = newValue;
    writeIndex = (writeIndex + 1) % bufferSize;
    if (writeIndex == 0)
        bufferFull = true;

    repaint();
}

void HistoryPlotComponent::setBufferSize (int newSize)
{
    if (newSize <= 0)
        return;

    // How many samples we actually have right now
    int oldSize       = bufferSize;
    int oldCount      = bufferFull ? oldSize : writeIndex;
    int keepCount     = std::min(oldCount, newSize);

    // Build a new buffer and copy the last 'keepCount' samples into it
    std::vector<float> newBuffer (static_cast<size_t>(newSize), 0.0f);

    // Where the oldest valid sample lives in the old buffer
    int oldStart = bufferFull ? writeIndex
                              : 0;

    // If we have more samples than newSize, skip the earliest ones
    int skip = oldCount - keepCount;
    int copyPos = (oldStart + skip) % oldSize;

    for (int i = 0; i < keepCount; ++i)
    {
        newBuffer[static_cast<size_t>(i)]
            = buffer[(copyPos + i) % oldSize];
    }

    // Swap in the new buffer
    buffer.swap (newBuffer);
    bufferSize = newSize;

    // Next write goes after our preserved samples
    writeIndex = keepCount % newSize;
    bufferFull = (oldCount >= newSize);

    repaint();
}

void HistoryPlotComponent::paint (juce::Graphics& g)
{
    auto w = getWidth();
    auto h = getHeight();

    // background
    g.fillAll (juce::Colours::black);

    // how many points do we actually have?
    int numSamples = bufferFull ? bufferSize : writeIndex;
    if (numSamples < 2)
        return;

    // oldest sample is at writeIndex (if full), or at 0 otherwise
    int startIndex = bufferFull ? writeIndex : 0;

    // prepare drawing
    juce::Path waveform;
    g.setColour (juce::Colours::lightgreen);

    float dx = static_cast<float>(w) / (numSamples - 1);

    // first point
    {
        float v = buffer[startIndex];
        float y = h * 0.5f - v * (h * 0.5f);
        waveform.startNewSubPath (0.0f, y);
    }

    // remaining points
    for (int i = 1; i < numSamples; ++i)
    {
        int idx = (startIndex + i) % bufferSize;
        float x = i * dx;
        float v = buffer[idx];
        float y = h * 0.5f - v * (h * 0.5f);
        waveform.lineTo (x, y);
    }

    g.strokePath (waveform, juce::PathStrokeType (2.0f));
}
