#pragma once

#include <JuceHeader.h>
#include <array>

#include "EnvelopeGenerator.h"
#include "Parameter.h"
#include "StateVariableFilter.h"
#include "Ramp.h"

namespace DSP
{

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class WavetableSynthVoice : public juce::SynthesiserVoice
{
public:
    WavetableSynthVoice();
    ~WavetableSynthVoice();

    enum LFOType : unsigned int
    {
        SIN = 0,
        TRI
    };

    enum FilterType : unsigned int
    {
        LPF = 0,
        BPF,
        HPF,
    };

    WavetableSynthVoice(const WavetableSynthVoice&) = delete;
    WavetableSynthVoice(WavetableSynthVoice&&) = delete;
    const WavetableSynthVoice& operator=(const WavetableSynthVoice&) = delete;
    const WavetableSynthVoice& operator=(WavetableSynthVoice&&) = delete;


    // Parameters
    

    // void setOscSawVol(float dB, bool skipRamp);
    // void setOscTriVol(float dB, bool skipRamp);
    // void setOscSinVol(float dB, bool skipRamp);
    // void setOscVol(float dB, bool skipRamp);

    void setWavetablePosition(float index, bool skip);
    void setWavetableVol(float db, bool skip);

    void setAttTimeVCA(float ms);
    void setDecayTimeVCA(float ms);
    void setSustainVCA(float norm);
    void setRelTimeVCA(float ms);

    void setAttTimeVCF(float ms);
    void setDecayTimeVCF(float ms);
    void setSustainVCF(float norm);
    void setRelTimeVCF(float ms);

    void setLFOFreqVCF(float Hz);
    void setLFOTypeVCF(LFOType type);

    void setEnvAmountVCF(float bipolar, bool skipRamp);
    void setLFOAmountVCF(float bipolar, bool skipRamp);

    void setFilterCutoff(float Hz, bool skipRamp);
    void setFilterReso(float Q, bool skipRamp);
    void setFilterType(FilterType type, bool skipRamp);

    void setOutputVol(float dB, bool skipRamp);


    bool canPlaySound(juce::SynthesiserSound* ptr) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    static constexpr float MaxFreqHz { 20000.f };
    static constexpr float MinFreqHz { 20.f };

    static constexpr float MaxReso { 10.f };
    static constexpr float MinReso { 0.5f };

    static constexpr float FreqModRange { 10000.f };

    static constexpr size_t SampleSize { 512 };
    static constexpr double DefaultSampleRate { 1.0 };

private:
    void fillWavetable(double SampleRate = DefaultSampleRate);

    double sampleRate { DefaultSampleRate };

    float lfoFreq { 1.f };
    float velocity { 1.f };

    std::vector<std::array<float, SampleSize>> wavetables;

    EnvelopeGenerator vcaEnvGen;
    EnvelopeGenerator vcfEnvGen;

    StateVariableFilter filter;

    LFOType lfoType;
    float lfoPhaseState { 0.f };
    float lfoPhaseInc { 0.f };

    /*
    Ramp<float> sinOscVolRamp;
    Ramp<float> triOscVolRamp;
    Ramp<float> sawOscVolRamp;
    */

    Parameter<float> wavetableIndex { Parameter<float>(0) };
    size_t wavetablePhase { 0 };
    size_t wavetableInc { 0 };

    Parameter<float> wavetableVolRamp;
    Ramp<float> outputVolRamp;

    Ramp<float> vcfEnvAmountRamp;
    Ramp<float> vcfLFOAmountRamp;

    Ramp<float> vcfFreqRamp;
    Ramp<float> vcfResoRamp;
    Ramp<float> vcfLPFRamp;
    Ramp<float> vcfBPFRamp;
    Ramp<float> vcfHPFRamp;

    bool voiceStarted { false };
};

}
