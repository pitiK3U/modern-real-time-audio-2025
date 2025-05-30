#include "ADSREnvelopeGenerator.h"

#include <juce_core/juce_core.h>

namespace DSP
{

ADSREnvelopeGenerator::ADSREnvelopeGenerator(float sampleRate)
    : sampleRate(sampleRate)
{
    // Initialize ADSR parameters (now just some defaults)
    attackTimeMs = 1.0f;   // ms
    decayTimeMs = 1.0f;    // ms
    sustainLevel = 0.5f;   // 0.0 to 1.0
    releaseTimeMs = 1.0f;  // ms

    // Initialize coefficients to 0.0f (no curvature)
    attackCurveCoefficients = { 0.0f,0.0f };
    decayCurveCoefficients = { 0.0f, 0.0f };
    releaseCurveCoefficients = { 0.0f, 0.0f };
}

// SETTERS

void ADSREnvelopeGenerator::prepare(double newSampleRate)
{
    sampleRate = static_cast<float>(newSampleRate);
}

void ADSREnvelopeGenerator::setAttackTime(float attackTimeMs)
{
    this->attackTimeMs = std::max(attackTimeMs, 1.0f);
}

void ADSREnvelopeGenerator::setDecayTime(float decayTimeMs)
{
    this->decayTimeMs = std::max(decayTimeMs, 1.0f);
}

void ADSREnvelopeGenerator::setSustainLevel(float sustainLevel)
{
    this->sustainLevel = std::clamp(sustainLevel, 0.0f, 1.0f);
}

void ADSREnvelopeGenerator::setReleaseTime(float releaseTimeMs)
{
    this->releaseTimeMs = std::max(releaseTimeMs, 1.0f);
}

void ADSREnvelopeGenerator::setAttackCurveX(float x)
{
    attackCurveCoefficients.first = x;
}

void ADSREnvelopeGenerator::setAttackCurveY(float y)
{
    attackCurveCoefficients.second = y;
}

void ADSREnvelopeGenerator::setDecayCurveX(float x)
{
    decayCurveCoefficients.first = x;
}

void ADSREnvelopeGenerator::setDecayCurveY(float y)
{
    decayCurveCoefficients.second = y;
}

void ADSREnvelopeGenerator::setReleaseCurveX(float x)
{
    releaseCurveCoefficients.first = x;
}

void ADSREnvelopeGenerator::setReleaseCurveY(float y)
{
    releaseCurveCoefficients.second = y;
}

// GETTERS

float ADSREnvelopeGenerator::getValue(bool midiGateState)
{
    // Advance state timer
    currentStateTimer += 1.0f / sampleRate * 1000.0f; // Convert to milliseconds

    // Update the envelope state based on MIDI gate state and current state timer
    updateEnvelopeState(midiGateState);

    // Calculate the current envelope value based on the current state
    switch (currentState) {
        case OFF:
            currentEnvelopeValue = getOffCurveValue();
            break;
        case ATTACK:
            currentEnvelopeValue = getAttackCurveValue();
            break;
        case DECAY:
            currentEnvelopeValue = getDecayCurveValue();
            break;
        case SUSTAIN:
            currentEnvelopeValue = getSustainCurveValue();
            break;
        case RELEASE:
            currentEnvelopeValue = getReleaseCurveValue();
            break;
    }

    // Clamp the envelope value to the range [0.0, 1.0]
    currentEnvelopeValue = std::clamp(currentEnvelopeValue, 0.0f, 1.0f);

    return currentEnvelopeValue;
}

bool ADSREnvelopeGenerator::isOff()
{
    return currentState == OFF;
}

// PRIVATE METHODS

void ADSREnvelopeGenerator::updateEnvelopeState(bool midiGateState)
{
    switch (currentState) {
        case OFF:
            if (midiGateState) {
                currentState = ATTACK;
                currentStateTimer = 0.f;
            }
            break;
        case ATTACK:
            if (midiGateState) {
                if (currentStateTimer >= attackTimeMs) {
                    currentState = DECAY;
                    currentStateTimer = 0.f; // Reset state timer
                    currentStateStartEnvelopeValue = currentEnvelopeValue;
                }
            } else {
                currentState = RELEASE;
                currentStateTimer = 0.f; // Reset state timer
                currentStateStartEnvelopeValue = currentEnvelopeValue;
            }
            break;
        case DECAY:
            if (midiGateState) {
                if (currentStateTimer >= decayTimeMs) {
                    currentState = SUSTAIN;
                    currentStateTimer = 0.f; // Reset state timer
                    currentStateStartEnvelopeValue = currentEnvelopeValue;
                }
            } else {
                currentState = RELEASE;
                currentStateTimer = 0.f; // Reset state timer
                currentStateStartEnvelopeValue = currentEnvelopeValue;
            }
            break;
        case SUSTAIN:
            if (!midiGateState) {
                currentState = RELEASE;
                currentStateTimer = 0.f; // Reset state timer
                currentStateStartEnvelopeValue = currentEnvelopeValue;
            }
            break;
        case RELEASE:
            if (midiGateState) {
                currentState = ATTACK;
                currentStateTimer = 0.f; // Reset state timer
                currentStateStartEnvelopeValue = currentEnvelopeValue;
            } else {
                if (currentStateTimer >= releaseTimeMs) {
                    currentState = OFF;
                    currentEnvelopeValue = 0.f;
                    currentStateStartEnvelopeValue = 0.f;
                }
            }
            break;
    }
}

float ADSREnvelopeGenerator::getOffCurveValue()
{
    currentEnvelopeValue = 0.0f; // Envelope is off
    return currentEnvelopeValue;
}

float ADSREnvelopeGenerator::getAttackCurveValue()
{
    double x = currentStateTimer;

    // Get curve coefficients
    double x0 = 0.0;
    double y0 = 0.0;
    double xn = attackTimeMs;
    double yn = 1.0;
    double xc = xn/2.0 + attackCurveCoefficients.first; // Control point x
    double yc = yn/2.0 + attackCurveCoefficients.second; // Control point y

    // Get envelope value
    float ye = solveQuadraticBezierAtX(x, x0, y0, xn, yn, xc, yc);

    // Scale envelope value
    return y0 + (1 - y0) * ye;
}

float ADSREnvelopeGenerator::getDecayCurveValue()
{
    double x = currentStateTimer;

    // Get curve coefficients
    double x0 = 0.0;
    double y0e = 1.0; // Start at 1.0 after attack (expected y start value)
    double xn = decayTimeMs;
    double yn = sustainLevel; // Expected end value is sustain level
    double xc = xn/2.0 + decayCurveCoefficients.first; // Control point x
    double yc = (y0e + yn)/2.0 + decayCurveCoefficients.second; // Control point y

    // Get envelope value
    float ye = solveQuadraticBezierAtX(x, x0, y0e, xn, yn, xc, yc);

    // Scale envelope value
    return yn + (currentStateStartEnvelopeValue - yn)/(y0e - yn) * (ye-yn);
}

float ADSREnvelopeGenerator::getSustainCurveValue()
{
    return currentEnvelopeValue;
}

float ADSREnvelopeGenerator::getReleaseCurveValue()
{
    double x = currentStateTimer;

    // Get curve coefficients
    double x0 = 0.0;
    double y0e = sustainLevel; // Decay should start at the sustain level
    double xn = releaseTimeMs;
    double yn = 0.0; // Expected end value is 0.0
    double xc = xn/2.0 + releaseCurveCoefficients.first; // Control point x
    double yc = (y0e + yn)/2.0 + releaseCurveCoefficients.second; // Control point y

    // Get envelope value
    float ye = solveQuadraticBezierAtX(x, x0, y0e, xn, yn, xc, yc);

    // Scale envelope value
    return yn + (currentStateStartEnvelopeValue - yn)/(y0e - yn) * (ye-yn);
}

float ADSREnvelopeGenerator::solveQuadraticBezierAtX(double xQuery, double x0, double y0, double x1, double y1, double cx, double cy)
{
    constexpr double eps = 1e-12;

    // Clamp to endpoints
    if (xQuery <= x0 + eps) return (float)y0;
    if (xQuery >= x1 - eps) return (float)y1;

    // Build quadratic A·t² + B·t + C = 0 so that x(t) = xQuery
    double A = x0 - 2.0*cx + x1;
    double B = 2.0*(cx - x0);
    double C = x0 - xQuery;

    double t = 0.0;
    if (std::abs(A) < eps)
    {
        // Degenerate → straight line
        if (std::abs(B) < eps)
            t = 0.0;
        else
            t = -C / B;
    }
    else
    {
        // Quadratic formula
        double disc = B*B - 4.0*A*C;
        if (disc < 0.0) disc = 0.0;
        double s  = std::sqrt(disc);
        double t1 = (-B + s) / (2.0*A);
        double t2 = (-B - s) / (2.0*A);

        // Pick the root in [0,1], otherwise clamp
        if      (t1 >= 0.0 && t1 <= 1.0) t = t1;
        else if (t2 >= 0.0 && t2 <= 1.0) t = t2;
        else                             t = std::clamp(t1, 0.0, 1.0);
    }

    // Evaluate y(t) = (1−t)² y0 + 2(1−t)t cy + t² y1
    t = std::clamp(t, 0.0, 1.0);
    double u = 1.0 - t;
    double y = u*u*y0 + 2.0*u*t*cy + t*t*y1;
    return (float)y;
}

EnvelopeState ADSREnvelopeGenerator::getCurrentState() const
{
    return currentState;
}

float ADSREnvelopeGenerator::getCurrentStateTimer() const
{
    return currentStateTimer;
}

}