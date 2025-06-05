#pragma once

#include <JuceHeader.h>
#include <cstddef>
#include <cstdint>

#include "ADSREnvelopeGenerator.h"
#include "Parameter.h"
#include "StateVariableFilter.h"
#include "Ramp.h"
#include "EnvelopeStateCollector.h"
#include "Wavetable.h"

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
    WavetableSynthVoice(const Wavetable& initialWavetable);
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

    using EffectEvaluator = Parameter<float>::EffectEvaluator;
    static constexpr auto defaultEffect = Parameter<float>::defaultEffect;

    // Parameters    
    void setWavetablePosition(float index, bool skip);

    void setWavetableVol(float db, bool skip);

    void setUnisonVoices(uint8_t numberOfVoices);
    void setUnisonDetune(float cents, bool skip);

    void setAttTime(float ms, int env_id);
    void setDecayTime(float ms, int env_id);
    void setSustain(float norm, int env_id);
    void setRelTime(float ms, int env_id);

    void setAttCurveX(float x, int env_id);
    void setAttCurveY(float y, int env_id);
    void setDecayCurveX(float x, int env_id);
    void setDecayCurveY(float y, int env_id);
    void setRelCurveX(float x, int env_id);
    void setRelCurveY(float y, int env_id);

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
    void setEnvelopeMonitor(EnvelopeStateCollector& collector, size_t index, int envelopeIndex);

    bool canPlaySound(juce::SynthesiserSound* ptr) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    float getLastWavetablePosition();

    static constexpr float MaxFreqHz { 20000.f };
    static constexpr float MinFreqHz { 20.f };

    static constexpr float MaxReso { 10.f };
    static constexpr float MinReso { 0.5f };

    static constexpr float FreqModRange { 10000.f };

    static constexpr double DefaultSampleRate { 1.0 };

    static constexpr float DefaultFrequency { 1.f };
    
    ADSREnvelopeGenerator envGenA;
    ADSREnvelopeGenerator envGenB;

    // Wave table position/index in the wavetables table - which sample in the set to use 
    Parameter<float> wavetableIndex { Parameter<float>(0) };
    
    Parameter<float> vcfFreq;
    Parameter<float> vcfReso;

    // Volume of individual wavetable voice at the beginning
    Parameter<float> wavetableVolRamp;
    // Volume of the voice after lfo's, env's and filter
    Parameter<float> outputVolRamp;
    
    private:
    void updateUnisonIncrements();
    
    static float getWavetableIncrement(float frequency, double originalFrequency, size_t sampleSize, double SampleRate);
    
    double sampleRate { DefaultSampleRate };
    
    float velocity { 1.f };
    
    const Wavetable& wavetable;

    StateVariableFilter filter;

    float frequency { 1.f };
    uint8_t unisonVoices { 1 };
    Parameter<float> unisonDetune {0.f };
    std::vector<float> unisonPhases;
    std::vector<float> unisonIncrements;

    Ramp<float> vcfLPFRamp;
    Ramp<float> vcfBPFRamp;
    Ramp<float> vcfHPFRamp;

    bool voiceStarted { false };
    bool gateState { false };

    EnvelopeStateCollector* envelopeStateCollectorA = nullptr;
    EnvelopeStateCollector* envelopeStateCollectorB = nullptr;
    size_t voiceIndex = 0;

    float lastWavetablePosition = 0;
};

}
