#include "Wavetable.h"

namespace DSP
{

void Wavetable::fillWavetable()
{
    const int wavetableCount = 4;
    wavetables.resize(wavetableCount);
    
    originalSampleFrequency = 1.f;

    for (int sample = 0; sample < SampleSize; sample++) {
        auto phase = static_cast<float>(sample) / static_cast<float>(SampleSize);
        // Sine waveform
        wavetables[0][sample] = std::sin(juce::MathConstants<float>::twoPi * phase);
        // Triangle waveform
        wavetables[1][sample] = 4.f * std::fabs(phase - std::floor(phase + 1.f / 2.f)) - 1;
        // Sawtooth waveform
        wavetables[2][sample] = 2.f * phase - 1;
        // Square waveform
        wavetables[3][sample] = std::copysign(1.f, wavetables[0][sample]);
    }
}

void Wavetable::clearWavetable()
{
    wavetables.clear();
}

void Wavetable::loadFromBuffer(const AudioSampleBuffer& buffer, double bufferSampleRate)
{
    int numOfSamples = buffer.getNumSamples();
    // round it up, so we dont lose samples
    int numberOfWaveforms = (numOfSamples + SampleSize - 1) / SampleSize;
    wavetables.resize(numberOfWaveforms);
    // 440.f to make it normal on A4
    
    originalSampleFrequency = static_cast<double>(SampleSize) * defaultNoteFrequency / bufferSampleRate;

    for (auto waveform = 0; waveform < numberOfWaveforms; waveform++) {
        auto end = std::min(SampleSize, numOfSamples - waveform * SampleSize);
        for (auto sample = 0; sample < end; sample++) {
            wavetables[waveform][sample] = buffer.getSample(0, sample + waveform * SampleSize);
        }
    }
}
}