#include "ADSREnvelopeComponent.h"

ADSREnvelopeComponent::ADSREnvelopeComponent()
{
    setupSlider (attackSlider,  0.0, 2000.0, 100.0);
    setupSlider (decaySlider,   0.0, 2000.0, 100.0);
    setupSlider (sustainSlider, 0.0,    1.0,   1.0);
    setupSlider (releaseSlider, 0.0, 2000.0, 100.0);
}

ADSREnvelopeComponent::~ADSREnvelopeComponent() {}

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
    g.fillAll (juce::Colours::black);

    // top half drawing area
    auto area = getLocalBounds()
        .removeFromTop (getHeight() / 2)
        .toFloat()
        .reduced (10.0f);

    // read your four ADSR values
    float a = (float) attackSlider .getValue();
    float d = (float) decaySlider  .getValue();
    float s = (float) sustainSlider.getValue();
    float r = (float) releaseSlider.getValue();

    // map “time since start” → x-position
    auto mapX = [&](float timeFromStart)
    {
        return area.getX() + (timeFromStart / MAX_LENGTH) * area.getWidth();
    };

    // cumulative x’s and y’s
    float x0 = area.getX(), y0 = area.getBottom();
    float x1 = mapX (a), y1 = area.getY();
    float x2 = mapX (a + d), y2 = juce::jmap (s, 0.0f, 1.0f, area.getBottom(), area.getY());
    float x3 = mapX (a + d + r), y3 = area.getBottom();

    // build the outline
    path.clear();
    path.startNewSubPath (x0, y0);
    path.lineTo(x1, y1);
    path.lineTo(x2, y2);
    path.lineTo(x3, y3);

    // build & fill the under-curve area
    juce::Path filledPath = path;
    filledPath.lineTo(x3, area.getBottom()); // bottom right
    filledPath.lineTo(x0, area.getBottom()); // bottom left
    filledPath.closeSubPath();

    g.setColour(juce::Colours::lightgreen.withAlpha (0.3f));
    g.fillPath(filledPath);

    // stroke on top
    g.setColour (juce::Colours::lightgreen);
    g.strokePath (path, juce::PathStrokeType (3.0f));

    // handles (attack, decay/sustain, release)
    points.clear();
    points.add({x0, y0});  // start point (cannot be moved)
    points.add ({ x1, y1 });
    points.add ({ x2, y2 });
    points.add ({ x3, y3 });

    g.setColour (juce::Colours::white);
    for (auto& pt : points)
        g.fillEllipse (pt.x - handleRadius,
                       pt.y - handleRadius,
                       handleRadius * 2.0f,
                       handleRadius * 2.0f);
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
