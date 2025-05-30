#include "HistoryPlotComponent.h"

namespace GUI
{

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
    int oldStart = bufferFull ? writeIndex : 0;

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
    juce::Path filledArea;

    float dx = static_cast<float>(w) / (numSamples - 1);

    // Calculate how many samples per horizontal pixel (on average)
    float samplesPerPixel = static_cast<float>(numSamples) / static_cast<float>(w);

    // first point
    {
        float v = buffer[startIndex];
        float y = h * 0.5f - 0.9f * v * (h * 0.5f);
        waveform.startNewSubPath (0.0f, y);
        filledArea.startNewSubPath (0.0f, static_cast<float>(h));
        filledArea.lineTo (0.0f, y);
    }

    // remaining points
    for (int pixelX = 1; pixelX < w; ++pixelX)
    {
        // calculate the index in the buffer for this pixel
        int i = static_cast<int>(pixelX * samplesPerPixel);
        if (i >= numSamples)
            break;

        int idx = (startIndex + i) % bufferSize;
        float v = buffer[idx];
        float y = h * 0.5f - 0.9f * v * (h * 0.5f);

        waveform.lineTo(static_cast<float>(pixelX), y);
        filledArea.lineTo(static_cast<float>(pixelX), y);
    }

    // close filled area back to the start
    filledArea.lineTo((numSamples - 1) * dx, static_cast<float>(h));
    filledArea.closeSubPath();

    g.setColour(juce::Colours::lightgreen.withAlpha (0.3f));
    g.fillPath (filledArea);

    g.setColour (juce::Colours::lightgreen);
    g.strokePath (waveform, juce::PathStrokeType (3.0f));
}

}