#include "WavetablePlotComponent.h"

namespace GUI
{

WavetablePlotComponent::WavetablePlotComponent()
{
    generateWavetables();
    setOpaque(true);
}

BoxCorners BoxCorners::getBoxCorners(
    juce::Rectangle<float> area,
    float xOffset,
    float yOffset,
    float scaleX,
    float scaleY,
    juce::Point<float> anchor,
    int depthCount)
{
    juce::AffineTransform scaleTransform = juce::AffineTransform::scale(scaleX, scaleY, anchor.x, anchor.y);

    juce::Point<float> offset = { xOffset * (depthCount - 1), yOffset * (depthCount - 1) };

    auto apply = [&](juce::Point<float> p, bool applyOffset) -> juce::Point<float> {
        float x = p.x, y = p.y;
        scaleTransform.transformPoint(x, y);
        juce::Point<float> result { x, y };
        return applyOffset ? result.translated(offset.x, offset.y) : result;
    };

    return {
        apply(area.getBottomLeft(), false),
        apply(area.getBottomRight(), false),
        apply(area.getTopRight(), false),
        apply(area.getTopLeft(), false),

        apply(area.getBottomLeft(), true),
        apply(area.getBottomRight(), true),
        apply(area.getTopRight(), true),
        apply(area.getTopLeft(), true)
    };
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
    const auto plotBounds = fullBounds.removeFromTop(fullBounds.getHeight() / 2.0f).reduced(10.0f);
    drawPlotBackground(g, plotBounds);

    auto waveformArea = plotBounds.reduced(16.0f);
    const float xOffset = 20.0f;
    const float yOffset = -10.0f;

    const float totalXOffset = xOffset * (wavetableCount - 1);
    const float totalYOffset = std::abs(yOffset) * (wavetableCount - 1);

    const float scaleX = (waveformArea.getWidth() - totalXOffset) / waveformArea.getWidth();
    const float scaleYFactor = (waveformArea.getHeight() - totalYOffset) / waveformArea.getHeight();
    const juce::Point<float> anchor = { waveformArea.getX(), waveformArea.getBottom() };

    auto corners = BoxCorners::getBoxCorners(waveformArea, xOffset, yOffset, scaleX, scaleYFactor, anchor, wavetableCount);

    drawBackBoxFaces(g, corners);
    drawWaveforms(g, waveformArea, scaleX, scaleYFactor, xOffset, yOffset);
    drawFrontBoxFaces(g, corners);
}

void WavetablePlotComponent::drawPlotBackground(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(area);

    g.setColour(juce::Colours::white);
    g.drawRect(area, 1.5f);
}

void WavetablePlotComponent::drawWaveforms(
    juce::Graphics& g,
    juce::Rectangle<float> area,
    float scaleX,
    float scaleYFactor,
    float xOffset,
    float yOffset)
{
    const float midY = area.getCentreY();
    const float scaleY = area.getHeight() / 2.2f;
    const float stepX = area.getWidth() / static_cast<float>(sampleSize - 1);

    for (int i = wavetableCount - 1; i >= 0; --i)
        drawSingleWaveform(g, i, area, midY, scaleY, stepX, scaleX, scaleYFactor, xOffset, yOffset);
}

void WavetablePlotComponent::drawSingleWaveform(
    juce::Graphics& g,
    int index,
    juce::Rectangle<float> area,
    float midY,
    float scaleY,
    float stepX,
    float scaleX,
    float scaleYFactor,
    float xOffset,
    float yOffset)
{
    const auto& waveform = wavetables[index];
    juce::Path path;

    const int viewLength = sampleSize + 2;
    const int viewOffset = sampleSize - 1;

    int index0 = (viewOffset + 0) % sampleSize;
    path.startNewSubPath(area.getX(), midY - scaleY * waveform[index0]);

    for (int j = 1; j < viewLength; ++j)
    {
        int sampleIdx = (viewOffset + j) % sampleSize;
        float x = area.getX() + static_cast<float>(j) * stepX;
        float y = midY - scaleY * waveform[sampleIdx];
        path.lineTo(x, y);
    }

    juce::Path fillPath(path);
    float endX = area.getX() + static_cast<float>(viewLength - 1) * stepX;
    fillPath.lineTo(endX, area.getBottom());
    fillPath.lineTo(area.getX(), area.getBottom());
    fillPath.closeSubPath();

    juce::AffineTransform transform =
        juce::AffineTransform::scale(scaleX, scaleYFactor, area.getX(), area.getBottom())
            .followedBy(juce::AffineTransform::translation(index * xOffset, index * yOffset));

    g.setColour(waveformColours[index].withAlpha(0.2f));
    g.fillPath(fillPath, transform);

    g.setColour(waveformColours[index].withAlpha(1.0f));
    g.strokePath(path, juce::PathStrokeType(1.5f), transform);
}

void WavetablePlotComponent::fillFace(
    juce::Graphics& g,
    juce::Point<float> a,
    juce::Point<float> b,
    juce::Point<float> c,
    juce::Point<float> d,
    juce::Colour colour)
{
    juce::Path face;
    face.startNewSubPath(a);
    face.lineTo(b);
    face.lineTo(c);
    face.lineTo(d);
    face.closeSubPath();

    // Fill with translucent color
    g.setColour(colour.withAlpha(0.1f));
    g.fillPath(face);

    // Stroke the edges
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.strokePath(face, juce::PathStrokeType(1.0f));
}

void WavetablePlotComponent::drawBackBoxFaces(juce::Graphics& g, const BoxCorners& c)
{
    fillFace(g, c.backTopLeft, c.backTopRight, c.backBottomRight, c.backBottomLeft, juce::Colours::white);
    fillFace(g, c.backTopLeft, c.frontTopLeft, c.frontBottomLeft, c.backBottomLeft, juce::Colours::white);
    fillFace(g, c.backTopLeft, c.backTopRight, c.frontTopRight, c.frontTopLeft, juce::Colours::white);
}

void WavetablePlotComponent::drawFrontBoxFaces(juce::Graphics& g, const BoxCorners& c)
{
    fillFace(g, c.frontBottomLeft, c.frontBottomRight, c.frontTopRight, c.frontTopLeft, juce::Colours::white);
    fillFace(g, c.backBottomRight, c.frontBottomRight, c.frontTopRight, c.backTopRight, juce::Colours::white);
    fillFace(g, c.backBottomLeft, c.backBottomRight, c.frontBottomRight, c.frontBottomLeft, juce::Colours::white);
}

}