#include "WavetablePlotComponent.h"

namespace GUI
{

WavetablePlotComponent::WavetablePlotComponent()
{
    generateWavetables();
    setOpaque(true);
}

void WavetablePlotComponent::generateWavetables()
{
    wavetables.resize(wavetableCount, std::vector<float>(sampleSize));

    for (int sample = 0; sample < sampleSize; sample++) {
        float phase = static_cast<float>(sample) / static_cast<float>(sampleSize); // [0, 1)

        // Sine: starts at 0
        wavetables[0][sample] = std::sin(juce::MathConstants<float>::twoPi * phase);

        // Triangle: starts at 0
        wavetables[1][sample] = 4.0f * phase - 1.0f; // ramp up
        if (phase < 0.25f)
            wavetables[1][sample] = 4.0f * phase;
        else if (phase < 0.75f)
            wavetables[1][sample] = 2.0f - 4.0f * phase;
        else
            wavetables[1][sample] = -4.0f + 4.0f * phase;

        // Sawtooth: starts at -1 → shifted to 0
        wavetables[2][sample] = 2.0f * phase - 1.0f;

        // Square: starts at 0 → use sine for phase-aligned threshold
        wavetables[3][sample] = std::sin(juce::MathConstants<float>::twoPi * phase) >= 0 ? 1.0f : -1.0f;
    }
}

void WavetablePlotComponent::resized()
{
    // Nothing to lay out
}

void WavetablePlotComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto fullBounds = getLocalBounds().toFloat();
    auto plotBounds = fullBounds.removeFromTop(fullBounds.getHeight() / 2.0f).reduced(10.0f);
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(plotBounds);
    g.setColour(juce::Colours::white);
    g.drawRect(plotBounds, 1.5f);

    auto waveformArea = plotBounds.reduced(16.0f);

    const float midY = waveformArea.getCentreY();
    const float scaleY = waveformArea.getHeight() / 2.2f;
    const float stepX = waveformArea.getWidth() / static_cast<float>(sampleSize - 1);

    const float xOffset = 20.0f;
    const float yOffset = -10.0f;

    const float totalXOffset = xOffset * (wavetableCount - 1);
    const float totalYOffset = std::abs(yOffset) * (wavetableCount - 1);
    const float scaleX = (waveformArea.getWidth() - totalXOffset) / waveformArea.getWidth();
    const float scaleYFactor = (waveformArea.getHeight() - totalYOffset) / waveformArea.getHeight();

    for (int i = wavetableCount - 1; i >= 0; --i)
    {
        const auto& waveform = wavetables[i];
        juce::Path path;

        // Include wrap-around (5 before, full, 5 after)
        const int viewLength = sampleSize + 2;
        const int viewOffset = sampleSize - 1;

        // First point
        int index0 = (viewOffset + 0) % sampleSize;
        path.startNewSubPath(waveformArea.getX(), midY - scaleY * waveform[index0]);

        for (int j = 1; j < viewLength; ++j)
        {
            int sampleIdx = (viewOffset + j) % sampleSize;
            float x = waveformArea.getX() + static_cast<float>(j) * stepX;
            float y = midY - scaleY * waveform[sampleIdx];
            path.lineTo(x, y);
        }

        // Fill under the curve
        juce::Path fillPath(path);
        float endX = waveformArea.getX() + static_cast<float>(viewLength - 1) * stepX;
        fillPath.lineTo(endX, waveformArea.getBottom());
        fillPath.lineTo(waveformArea.getX(), waveformArea.getBottom());
        fillPath.closeSubPath();

        // Apply scaling and offset
        juce::AffineTransform transform =
            juce::AffineTransform::scale(scaleX, scaleYFactor, waveformArea.getX(), waveformArea.getBottom())
                .followedBy(juce::AffineTransform::translation(i * xOffset, i * yOffset));

        // Filled area (light, translucent)
        g.setColour(waveformColours[i].withAlpha(0.2f));
        g.fillPath(fillPath, transform);

        // Stroke
        g.setColour(waveformColours[i].withAlpha(1.0f));
        g.strokePath(path, juce::PathStrokeType(1.5f), transform);
    }
}



}