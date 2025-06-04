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
    const float padding = 4.0f;

    // Define padded area and skip filling the outer padding
    auto bounds = getLocalBounds().toFloat();
    auto plotArea = bounds.reduced(padding);

    // fill inner plot area only
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(plotArea);

    // draw border around inner plot area
    g.setColour(juce::Colours::white);
    g.drawRect(plotArea, 1.5f);

    int numSamples = bufferFull ? bufferSize : writeIndex;
    if (numSamples < 2)
        return;

    int startIndex = bufferFull ? writeIndex : 0;

    juce::Path waveform, filledArea;
    float dx = plotArea.getWidth() / static_cast<float>(numSamples - 1);
    float samplesPerPixel = static_cast<float>(numSamples) / plotArea.getWidth();

    // first point
    {
        float v = buffer[startIndex];
        float y = plotArea.getCentreY() - 0.9f * v * (plotArea.getHeight() * 0.5f);
        waveform.startNewSubPath(plotArea.getX(), y);
        filledArea.startNewSubPath(plotArea.getX(), plotArea.getBottom());
        filledArea.lineTo(plotArea.getX(), y);
    }

    for (int pixelX = 1; pixelX < plotArea.getWidth(); ++pixelX)
    {
        int i = static_cast<int>(pixelX * samplesPerPixel);
        if (i >= numSamples)
            break;

        int idx = (startIndex + i) % bufferSize;
        float v = buffer[idx];
        float x = plotArea.getX() + pixelX;
        float y = plotArea.getCentreY() - 0.9f * v * (plotArea.getHeight() * 0.5f);

        waveform.lineTo(x, y);
        filledArea.lineTo(x, y);
    }

    filledArea.lineTo(plotArea.getRight(), plotArea.getBottom());
    filledArea.closeSubPath();

    g.setColour(juce::Colours::lightgreen.withAlpha(0.3f));
    g.fillPath(filledArea);

    g.setColour(juce::Colours::lightgreen);
    g.strokePath(waveform, juce::PathStrokeType(3.0f));
}

}