#pragma once

#include <vector>
#include <array>
#include <JuceHeader.h>

namespace DSP {
class Wavetable {
public:

    static constexpr std::size_t SampleSize { 2048ul };
    static constexpr double DefaultSampleRate { 48000.0 };
    static constexpr auto A4 = 440.0;

    void clearWavetable();
    void fillWavetable();
    void loadFromBuffer(const AudioSampleBuffer& buffer, double bufferSampleRate);

    std::vector<std::array<float, SampleSize>> wavetables;
    double sampleRate { DefaultSampleRate };
    // How long is one waveform
    // Usually:
    //  - waveforms: 1 Hz
    //  - samples: static_cast<double>(SampleSize) * defaultNoteFrequency / bufferSampleRate
    double originalSampleFrequency { 1.0 };

    double defaultNoteFrequency = A4;

private:
};
}