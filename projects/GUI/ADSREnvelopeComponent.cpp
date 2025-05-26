#include "ADSREnvelopeComponent.h"

ADSREnvelopeComponent::ADSREnvelopeComponent()
: startTime (juce::Time::getMillisecondCounterHiRes())
{
    setupSlider (attackSlider,  0.0, 2000.0, 100.0);
    setupSlider (decaySlider,   0.0, 2000.0, 300.0);
    setupSlider (sustainSlider, 0.0,    1.0,   0.6);
    setupSlider (releaseSlider, 0.0, 2000.0, 700.0);

    startTimerHz (60);  // repaint + update at 60 Hz
}

ADSREnvelopeComponent::~ADSREnvelopeComponent() {}

void ADSREnvelopeComponent::timerCallback()
{
    // read up-to-date times
    float a = (float) attackSlider.getValue();
    float d = (float) decaySlider.getValue();
    float r = (float) releaseSlider.getValue();
    float total = a + d + r;

    double now = juce::Time::getMillisecondCounterHiRes();
    double elapsed = now - startTime;

    // loop when we reach the end
    if (total <= 0.0f || elapsed > total)
    {
        startTime = now;
        elapsed = 0.0;
    }

    currentPhaseTime = (float) elapsed;
    repaint();
}

void ADSREnvelopeComponent::setupSlider (juce::Slider& s, double min, double max, double def)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setRange (min, max);
    s.setValue (def);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible (s);
    s.onValueChange = [this] { repaint(); };
}

void ADSREnvelopeComponent::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto area = getLocalBounds()
                    .removeFromTop(getHeight() / 2)
                    .toFloat()
                    .reduced(10.0f);

    float a = (float)attackSlider.getValue();
    float d = (float)decaySlider.getValue();
    float s = (float)sustainSlider.getValue();
    float r = (float)releaseSlider.getValue();

    drawBackground(g, area);
    drawEnvelope(g, area, a, d, s, r);
    drawPlayhead(g, area, a, d, s, r);
    drawHandles(g);
}

void ADSREnvelopeComponent::drawBackground(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(area);
    g.setColour(juce::Colours::white);
    g.drawRect(area, 1.5f);
}

void ADSREnvelopeComponent::drawEnvelope(juce::Graphics& g,
                                          juce::Rectangle<float> area,
                                          float a, float d, float s, float r)
{
    auto mapX = [&](float t) { return area.getX() + (t / MAX_LENGTH) * area.getWidth(); };

    float x0 = area.getX();
    float y0 = area.getBottom();
    float x1 = mapX(a);
    float y1 = area.getY();
    float x2 = mapX(a + d);
    float y2 = juce::jmap(s, 0.0f, 1.0f, area.getBottom(), area.getY());
    float x3 = mapX(a + d + r);
    float y3 = area.getBottom();

    path.clear();
    path.startNewSubPath(x0, y0);
    path.lineTo(x1, y1);
    path.lineTo(x2, y2);
    path.lineTo(x3, y3);

    juce::Path fillPath(path);
    fillPath.lineTo(x3, area.getBottom());
    fillPath.lineTo(x0, area.getBottom());
    fillPath.closeSubPath();

    g.setColour(juce::Colours::lightgreen.withAlpha(0.3f));
    g.fillPath(fillPath);

    g.setColour(juce::Colours::lightgreen);
    g.strokePath(path, juce::PathStrokeType(3.0f));

    points.clear();
    points.add({x0, y0});
    points.add({x1, y1});
    points.add({x2, y2});
    points.add({x3, y3});
}

void ADSREnvelopeComponent::drawPlayhead(juce::Graphics& g,
                                         juce::Rectangle<float> area,
                                         float a, float d, float s, float r)
{
    float t = currentPhaseTime;
    float total = a + d + r;
    if (t < 0 || t > total)
        return;

    float value = 0;
    if (t < a && a > 0)                value = t / a;
    else if (t < a + d && d > 0)       value = 1.0f - ((t - a) / d) * (1.0f - s);
    else if (t < a + d + r && r > 0)   value = s * (1.0f - ((t - a - d) / r));

    auto x = area.getX() + (t / MAX_LENGTH) * area.getWidth();
    auto y = juce::jmap(value, 0.0f, 1.0f, area.getBottom(), area.getY());

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawLine(x, area.getBottom(), x, y, 1.0f);

    g.setColour(juce::Colours::white);
    float pr = handleRadius * 0.75f;
    g.fillEllipse(x - pr, y - pr, pr * 2, pr * 2);
}

void ADSREnvelopeComponent::drawHandles(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    for (auto& pt : points)
        g.fillEllipse(pt.x - handleRadius,
                      pt.y - handleRadius,
                      handleRadius * 2,
                      handleRadius * 2);
}

void ADSREnvelopeComponent::resized()
{
    auto knobsArea = getLocalBounds().removeFromBottom (getHeight() / 2).reduced (10);
    int w = knobsArea.getWidth() / 4;
    attackSlider.setBounds  (knobsArea.removeFromLeft (w));
    decaySlider.setBounds   (knobsArea.removeFromLeft (w));
    sustainSlider.setBounds (knobsArea.removeFromLeft (w));
    releaseSlider.setBounds (knobsArea);
}

void ADSREnvelopeComponent::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < points.size(); ++i)
        if (points[i].getDistanceFrom (e.position) < handleRadius * 1.5f) // User can grab the handle bit further away then the radius
            draggingPoint = i;
}

void ADSREnvelopeComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingPoint < 0)
        return;

    // same drawing area
    auto area = getLocalBounds()
        .removeFromTop (getHeight() / 2)
        .toFloat()
        .reduced (10.0f);

    auto pos = e.position;
    pos.x = juce::jlimit (area.getX(), area.getRight(),  pos.x);
    pos.y = juce::jlimit (area.getY(), area.getBottom(), pos.y);

    // invert mapX
    auto mapXToTime = [&](float x)
    {
        float norm = (x - area.getX()) / area.getWidth();
        return norm * MAX_LENGTH;
    };

    // current values
    float a = (float) attackSlider .getValue();
    float d = (float) decaySlider  .getValue();

    // decide which slider to update
    if (draggingPoint == 1) // Dragging point 0 gets ignored, as it is the start point
    {
        // moving the attack handle
        float newA = mapXToTime (pos.x);
        attackSlider.setValue (newA, juce::dontSendNotification);
    }
    else if (draggingPoint == 2)
    {
        // moving the decay/sustain handle
        float total = mapXToTime (pos.x);
        float newD  = total - a; // decay = (attack+decay) – attack
        newD = juce::jlimit (0.0f, MAX_LENGTH - a, newD);
        decaySlider.setValue (newD, juce::dontSendNotification);

        // vertical still controls sustain
        float normY = (area.getBottom() - pos.y) / area.getHeight();
        float newS  = juce::jlimit (0.0f, 1.0f, normY);
        sustainSlider.setValue (newS, juce::dontSendNotification);
    }
    else if (draggingPoint == 3)
    {
        // moving the release handle
        float total = mapXToTime (pos.x);
        float newR  = total - (a + d); // release = (attack+decay+release) – (attack+decay)
        newR = juce::jlimit (0.0f, MAX_LENGTH - a - d, newR);
        releaseSlider.setValue (newR, juce::dontSendNotification);
    }

    repaint();
}

void ADSREnvelopeComponent::mouseUp (const juce::MouseEvent&)
{
    draggingPoint = -1;
}
