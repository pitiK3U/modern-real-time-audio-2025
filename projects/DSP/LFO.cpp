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
    frequency.setValue(newFrequency, true);
    waveform     = newWaveform;
    updatePhaseIncrement();
}

void LFO::updatePhaseIncrement()
{
    phaseIncrement = frequency.getCurrentValue() / sampleRate;
}

float LFO::computeWaveform (float p) const
{
    // 1) apply horizontal (phase) offset
    float phase = p + offset;

    // wrap into [0…1)
    phase -= std::floor (phase);

    float value = 0.0f;
    switch (waveform)
    {
        case Waveform::Sine:
            value = std::sin (2.0f * juce::MathConstants<float>::pi * phase);
            break;

        case Waveform::Square:
            value = (phase < 0.5f ?  1.0f : -1.0f);
            break;

        case Waveform::Triangle:
            value = 4.0f * std::abs (phase - 0.5f) - 1.0f;
            break;

        case Waveform::Sawtooth:
            value = 2.0f * (phase - 0.5f);
            break;
    }

    // 2) apply amplitude only (no vertical offset here)
    return amplitude * value;
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
    frequency.getNext();
    updatePhaseIncrement();

    phase += phaseIncrement;
    if (phase >= 1.0f)
        phase -= 1.0f;
}

// Set a new frequency for the oscillator in Hz
void LFO::setFrequency(float freqHz) {
    frequency.setValue(freqHz);
    updatePhaseIncrement();
}

void LFO::setOffset(float newOffset) {
    offset = newOffset;
    updatePhaseIncrement();
}

void LFO::setWaveform(Waveform newWaveform) {
    waveform = newWaveform;
    updatePhaseIncrement();
}

void LFO::setSampleRate(float newSampleRate) {
    sampleRate = newSampleRate;
    updatePhaseIncrement();
}

}