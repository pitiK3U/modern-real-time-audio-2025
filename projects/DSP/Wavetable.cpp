#include "Wavetable.h"
#include "WavetablePlugins.h"

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

void Wavetable::loadFromPreset(const DSP::WavetablePlugins::PresetID wavetableId)
{
    auto preset = DSP::WavetablePlugins::getPreset(wavetableId);
    jassert(!preset.empty());

    wavetables.clear();
    wavetables.reserve(preset.size());

    for (const auto& wave : preset)
    {
        std::array<float, SampleSize> arr {};
        std::copy(wave.begin(), wave.end(), arr.begin());
        wavetables.push_back(std::move(arr));
    }

    originalSampleFrequency = 1.0;
}

std::vector<std::vector<float>> Wavetable::getRenderPreview() const
{
    constexpr int maxWavetables = 10;
    constexpr int maxPreviewSize = 2048;

    std::vector<std::vector<float>> preview;
    preview.reserve(std::min<int>(wavetables.size(), maxWavetables));

    for (size_t i = 0; i < std::min<size_t>(wavetables.size(), maxWavetables); ++i)
    {
        int size = std::min<int>(maxPreviewSize, SampleSize);
        preview.emplace_back(wavetables[i].begin(), wavetables[i].begin() + size);
    }

    return preview;
}

}