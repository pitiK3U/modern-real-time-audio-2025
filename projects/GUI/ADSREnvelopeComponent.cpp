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
    if (isMouseOverEnvelopeArea)
        drawHandles(g);
}

void ADSREnvelopeComponent::drawBackground(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(area);
    g.setColour(juce::Colours::white);
    g.drawRect(area, 1.5f);
}

void ADSREnvelopeComponent::drawEnvelope(juce::Graphics& g, juce::Rectangle<float> area, float a, float d, float s, float r)
{
    auto mapX = [&](float t)
    {
        return area.getX() + (t / MAX_LENGTH) * area.getWidth();
    };

    // 1) compute the 4 on-curve points
    float x0 = area.getX(), y0 = area.getBottom();
    float x1 = mapX (a), y1 = area.getY();
    float x2 = mapX (a + d), y2 = juce::jmap (s, 0.0f, 1.0f, area.getBottom(), area.getY());
    float x3 = mapX (a + d + r), y3 = area.getBottom();

    // 2) stash them into points[]
    points.clear();
    points.add ({ x0, y0 });
    points.add ({ x1, y1 });
    points.add ({ x2, y2 });
    points.add ({ x3, y3 });

    // 3) ensure we have 3 offsets (init to zero → handles at midpoints)
    if (controlPointOffsets.size() != 3)
    {
        controlPointOffsets.clear();
        controlPointOffsets.add ({ 0, 0 });
        controlPointOffsets.add ({ 0, 0 });
        controlPointOffsets.add ({ 0, 0 });
    }

    // 4) build absolute control-points = midpoint + offset
    controlPoints.clear();
    for (int i = 0; i < 3; ++i)
    {
        auto mid = (points[i] + points[i+1]) * 0.5f;
        controlPoints.add (mid + controlPointOffsets[i]);
    }

    // 5) stroke the quadratic path
    path.clear();
    path.startNewSubPath(points[0]);
    path.quadraticTo(controlPoints[0], points[1]);
    path.quadraticTo(controlPoints[1], points[2]);
    path.quadraticTo(controlPoints[2], points[3]);

    // 6) fill under it
    juce::Path fillPath (path);
    fillPath.lineTo (x3, area.getBottom());
    fillPath.lineTo (x0, area.getBottom());
    fillPath.closeSubPath();

    g.setColour(juce::Colours::lightgreen.withAlpha (0.3f));
    g.fillPath(fillPath);

    g.setColour(juce::Colours::lightgreen);
    g.strokePath(path, juce::PathStrokeType (3.0f));
}

void ADSREnvelopeComponent::drawPlayhead(juce::Graphics& g, juce::Rectangle<float> area, float a, float d, float s, float r)
{
    float t = currentPhaseTime;
    float total = a + d + r;
    if (t < 0 || t > total)
        return;

    float x = area.getX() + (t / MAX_LENGTH) * area.getWidth();

    // sample the Bézier at that x
    float y = getYForX (x);

    // draw vertical guide
    g.setColour(juce::Colours::white.withAlpha (0.2f));
    g.drawLine(x, area.getBottom(), x, y, 1.0f);

    // draw the playhead dot
    g.setColour (juce::Colours::white);
    float pr = handleRadius * 0.75f;
    g.fillEllipse (x - pr, y - pr, pr * 2.0f, pr * 2.0f);
}

void ADSREnvelopeComponent::drawHandles(juce::Graphics& g)
{
    g.setColour (juce::Colours::white);

    // on-curve handles
    for (auto& pt : points)
        g.fillEllipse (pt.x - handleRadius, pt.y - handleRadius, handleRadius * 2, handleRadius * 2);

    // off-curve handles (draw as rings)
    for (auto& cp : controlPoints)
        g.drawEllipse (cp.x - handleRadius, cp.y - handleRadius, handleRadius * 2, handleRadius * 2, 2.0f);
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
    // endpoints (0–3)
    for (int i = 0; i < points.size(); ++i)
        if (points[i].getDistanceFrom (e.position) < handleRadius * 1.5f)
        {
            draggingPoint = i;
            return;
        }

    // control handles (4–6)
    for (int i = 0; i < controlPoints.size(); ++i)
        if (controlPoints[i].getDistanceFrom (e.position) < handleRadius * 1.5f)
        {
            draggingPoint = 4 + i;
            return;
        }
}

void ADSREnvelopeComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingPoint < 0)
        return;

    // clamp mouse into envelope rect
    auto area = getLocalBounds()
        .removeFromTop (getHeight()/2)
        .toFloat()
        .reduced (10.0f);

    juce::Point<float> pos = e.position;
    pos.x = juce::jlimit(area.getX(), area.getRight(), pos.x);
    pos.y = juce::jlimit(area.getY(), area.getBottom(), pos.y);

    // helpers
    auto mapXToTime = [&](float x)
    {
        return ((x - area.getX()) / area.getWidth()) * MAX_LENGTH;
    };
    auto mapTimeToX = [&](float t)
    {
        return area.getX() + (t / MAX_LENGTH) * area.getWidth();
    };

    // current ADSR
    float a = (float) attackSlider.getValue();
    float d = (float) decaySlider.getValue();
    float s = (float) sustainSlider.getValue();
    float r = (float) releaseSlider.getValue();

    // endpoint drags (1–3)
    if (draggingPoint >= 1 && draggingPoint <= 3)
    {
        float newA = a, newD = d, newS = s, newR = r;

        if (draggingPoint == 1)
        {
            newA = juce::jlimit (0.0f, MAX_LENGTH - d - r, mapXToTime (pos.x));
        }
        else if (draggingPoint == 2)
        {
            float total = mapXToTime (pos.x);
            newD = juce::jlimit (0.0f, MAX_LENGTH - a - r, total - newA);
            newS = juce::jlimit (0.0f, 1.0f, (area.getBottom() - pos.y) / area.getHeight());
        }
        else // release
        {
            float total = mapXToTime (pos.x);
            newR = juce::jlimit (0.0f, MAX_LENGTH - newA - newD, total - newA - newD);
        }

        // compute old vs new pixel positions
        auto oldP = points.getReference (draggingPoint);
        juce::Point<float> newP;

        if(draggingPoint == 1)
            newP = { mapTimeToX (newA), area.getY() };
        else if (draggingPoint == 2)
            newP = { mapTimeToX (newA + newD), juce::jmap (newS, 0.0f, 1.0f, area.getBottom(), area.getY()) };
        else
            newP = { mapTimeToX (newA + newD + newR), area.getBottom() };

        auto delta = newP - oldP;

        // push new slider values
        attackSlider .setValue (newA, juce::dontSendNotification);
        decaySlider  .setValue (newD, juce::dontSendNotification);
        sustainSlider.setValue (newS, juce::dontSendNotification);
        releaseSlider.setValue (newR, juce::dontSendNotification);

        repaint();
        return;
    }

    // control-point drags (4–6)
    if (draggingPoint >= 4 && draggingPoint <= 6)
    {
        int idx = draggingPoint - 4;

        // grab the two neighbouring on-curve points
        auto p0 = points.getReference(idx);
        auto p1 = points.getReference(idx + 1);

        // clamp mouse X to lie between those two Xs
        float minX = juce::jmin(p0.x, p1.x);
        float maxX = juce::jmax(p0.x, p1.x);
        pos.x = juce::jlimit(minX, maxX, pos.x);

        // recompute midpoint
        auto mid = (p0 + p1) * 0.5f;

        // store the offset from midpoint (Y is free)
        controlPointOffsets.set(idx, { pos.x - mid.x,
                                    pos.y - mid.y });

        repaint();
    }
}

void ADSREnvelopeComponent::mouseUp (const juce::MouseEvent&)
{
    draggingPoint = -1;
}

void ADSREnvelopeComponent::mouseMove (const juce::MouseEvent& e)
{
    // build exactly the same rect you use to paint the envelope:
    auto envelopeArea = getLocalBounds()
        .removeFromTop (getHeight() / 2)
        .toFloat()
        .reduced (10.0f);

    bool nowOver = envelopeArea.contains (e.position);
    if (nowOver != isMouseOverEnvelopeArea)
    {
        isMouseOverEnvelopeArea = nowOver;
        repaint();
    }
}

void ADSREnvelopeComponent::mouseExit (const juce::MouseEvent&)
{
    // guarantee handles disappear if the mouse leaves the whole component
    if (isMouseOverEnvelopeArea)
    {
        isMouseOverEnvelopeArea = false;
        repaint();
    }
}

float ADSREnvelopeComponent::getYForX (float xQuery) const
{
    // each segment i goes from points[i] to points[i+1] with controlPoints[i]
    for (int i = 0; i < controlPoints.size(); ++i)
    {
        auto p0 = points.getReference(i);
        auto p1 = points.getReference(i+1);
        auto cp = controlPoints.getReference(i);

        // only try if xQuery is between the end-points of this segment
        if (xQuery < juce::jmin(p0.x, p1.x) || xQuery > juce::jmax(p0.x, p1.x))
            continue;

        // quadratic coefficients for B_x(t) = xQuery
        float A = p0.x - 2.0f * cp.x + p1.x;
        float B = 2.0f * (cp.x - p0.x);
        float C = p0.x - xQuery;

        float t = -1.0f;
        if (std::abs(A) < 1e-6f)  // degenerate → linear
        {
            t = -C / B;
        }
        else
        {
            float disc = B * B - 4.0f * A * C;
            if (disc < 0.0f) 
                continue;  // no real t
            float sqrtD = std::sqrt(disc);
            float t1 = (-B + sqrtD) / (2.0f * A);
            float t2 = (-B - sqrtD) / (2.0f * A);
            if (t1 >= 0.0f && t1 <= 1.0f)      t = t1;
            else if (t2 >= 0.0f && t2 <= 1.0f) t = t2;
        }

        if (t >= 0.0f && t <= 1.0f)
        {
            // Bézier in y
            float u = 1.0f - t;
            return u*u*p0.y
                 + 2.0f*u*t*cp.y
                 +     t*t*p1.y;
        }
    }

    // fallback: if xQuery is outside all segments, clamp to baseline
    return points.getFirst().y;
}
