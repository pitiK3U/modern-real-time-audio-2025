#include "LFO.h"

#include <cmath>
#include <juce_core/juce_core.h>

namespace DSP {

LFO::LFO(float sr, float freq, Waveform wf)
  : sampleRate(sr), frequency(freq), waveform(wf)
{
    updatePhaseIncrement();
}

void LFO::prepare(float newSampleRate, float newFrequency, Waveform newWaveform)
{
    sampleRate   = newSampleRate;
    frequency    = newFrequency;
    waveform     = newWaveform;
    updatePhaseIncrement();
}

void LFO::updatePhaseIncrement()
{
    phaseIncrement = frequency / sampleRate;
}

float LFO::computeWaveform(float p) const
{
    switch (waveform)
    {
        case Waveform::Sine:     return amplitude * std::sin(2.0f * juce::MathConstants<float>::pi * p) + offset;
        case Waveform::Square:   return amplitude * (p < 0.5f ? 1.0f : -1.0f) + offset;
        case Waveform::Triangle: return amplitude * (4.0f * std::abs(p - 0.5f) - 1.0f) + offset;
        case Waveform::Sawtooth: return amplitude * (2.0f * (p - 0.5f)) + offset;
    }
    return offset;
}

float LFO::getValue(bool advance)
{
    float value = computeWaveform(phase);
    if (advance)
        advancePhase();
    return value;
}

void LFO::advancePhase()
{
    phase += phaseIncrement;
    if (phase >= 1.0f)
        phase -= 1.0f;
}

}