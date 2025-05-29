#include "ADSREnvelopeComponent.h"
#include "juce_core/juce_core.h"
#include <cassert>

ADSREnvelopeComponent::ADSREnvelopeComponent(
    juce::AudioProcessorValueTreeState& state,
    const juce::String& attackID,
    const juce::String& decayID,
    const juce::String& sustainID,
    const juce::String& releaseID,
    const juce::String& attackCurveXID,
    const juce::String& attackCurveYID,
    const juce::String& decayCurveXID,
    const juce::String& decayCurveYID,
    const juce::String& releaseCurveXID,
    const juce::String& releaseCurveYID
)
: startTime (juce::Time::getMillisecondCounterHiRes())
{
    setupSlider (attackSlider);
    setupSlider (decaySlider);
    setupSlider (sustainSlider);
    setupSlider (releaseSlider);

    setupSlider(attackCurveXSlider);
    setupSlider(attackCurveYSlider);
    setupSlider(decayCurveXSlider);
    setupSlider(decayCurveYSlider);
    setupSlider(releaseCurveXSlider);
    setupSlider(releaseCurveYSlider);

    attackAttachment  = std::make_unique<Attachment>(state, attackID,  attackSlider);
    decayAttachment   = std::make_unique<Attachment>(state, decayID,   decaySlider);
    sustainAttachment = std::make_unique<Attachment>(state, sustainID, sustainSlider);
    releaseAttachment = std::make_unique<Attachment>(state, releaseID, releaseSlider);

    attackCurveXAttachment  = std::make_unique<Attachment>(state, attackCurveXID,  attackCurveXSlider);
    attackCurveYAttachment  = std::make_unique<Attachment>(state, attackCurveYID,  attackCurveYSlider);
    decayCurveXAttachment   = std::make_unique<Attachment>(state, decayCurveXID,   decayCurveXSlider);
    decayCurveYAttachment   = std::make_unique<Attachment>(state, decayCurveYID,   decayCurveYSlider);
    releaseCurveXAttachment = std::make_unique<Attachment>(state, releaseCurveXID, releaseCurveXSlider);
    releaseCurveYAttachment = std::make_unique<Attachment>(state, releaseCurveYID, releaseCurveYSlider);

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

void ADSREnvelopeComponent::setupSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible (s); // TODO: disable for curve sliders?
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
    g.strokePath(
        path,
        juce::PathStrokeType (
            3.0f, 
            juce::PathStrokeType::JointStyle::curved, 
            juce::PathStrokeType::EndCapStyle::rounded
        )
    );
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
    if (draggingPoint >= 1 && draggingPoint <= 3) // TODO: When dragging point check if curve handle is not too far away
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
        attackSlider.setValue(newA);
        decaySlider.setValue(newD);
        sustainSlider.setValue(newS);
        releaseSlider.setValue(newR);

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
        auto offset = pos - mid;
        controlPointOffsets.set(idx, offset);

        // Map pixel offset to actual value in ms or amplitude
        float timeOffset = (offset.x / area.getWidth()) * MAX_LENGTH; // ms offset
        float levelOffset = -(offset.y / area.getHeight());           // y grows downwards, invert

        // Store the new values in the curve sliders
        switch (idx)
        {
            case 0:
                attackCurveXSlider.setValue(timeOffset);
                attackCurveYSlider.setValue(levelOffset);
                break;
            case 1:
                decayCurveXSlider.setValue(timeOffset);
                decayCurveYSlider.setValue(levelOffset);
                break;
            case 2:
                releaseCurveXSlider.setValue(timeOffset);
                releaseCurveYSlider.setValue(levelOffset);
                break;
        }

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

float ADSREnvelopeComponent::getYForX (float xQueryF) const
{
    // bump into double right away
    double xQuery = xQueryF;
    constexpr double eps = 1e-12;

    // iterate each Bézier segment
    for (int i = 0; i < controlPoints.size(); ++i)
    {
        auto p0f = points.getReference(i);
        auto p1f = points.getReference(i + 1);
        auto cpf = controlPoints.getReference(i);

        // cast endpoints/control to double
        double x0 = p0f.x,  y0 = p0f.y;
        double x1 = p1f.x,  y1 = p1f.y;
        double cx =  cpf.x, cy =  cpf.y;

        // quick‐reject with a tiny margin
        double minX = std::min(x0, x1) - eps;
        double maxX = std::max(x0, x1) + eps;
        if (xQuery < minX || xQuery > maxX)
            continue;

        // quadratic A·t² + B·t + C = 0 for Bₓ(t) == xQuery
        double A = x0 - 2.0*cx + x1;
        double B = 2.0*(cx - x0);
        double C = x0 - xQuery;

        double t = -1.0;

        if (std::abs(A) < eps)
        {
            // effectively linear
            if (std::abs(B) < eps)
                continue;
            t = -C / B;
        }
        else
        {
            double disc = B*B - 4.0*A*C;
            if (disc < 0.0)
                continue;
            double s = std::sqrt(disc);
            double t1 = (-B + s) / (2.0*A);
            double t2 = (-B - s) / (2.0*A);
            if      (t1 >= 0.0 && t1 <= 1.0) t = t1;
            else if (t2 >= 0.0 && t2 <= 1.0) t = t2;
            else                             continue;
        }

        if (t >= 0.0 && t <= 1.0)
        {
            double u = 1.0 - t;
            double y = u*u*y0 + 2.0*u*t*cy + t*t*y1;
            return (float)y;
        }
    }

    // clamp to endpoints if nothing matched
    return points.getFirst().y;
}

