#pragma once

#include <JuceHeader.h>

#include "Oscillator.h"
#include "ParametricEqualizer.h"
#include "StateVariableFilter.h"
#include "SynthVoice.h"
#include "EnvelopeGenerator.h"
#include "SynthVoiceEnvelope.h"
#include "juce_core/juce_core.h"
#include "mrta_utils/Source/Parameter/ParameterManager.h"

namespace Param
{

    namespace ID
    {
        static const juce::String WaveType { "wave_type" };

        static const juce::String Analog { "analog" };
        static const juce::String AttackTime { "attack_time" };
        static const juce::String DecayTime { "decay_time" };
        static const juce::String SustainLevel { "sustain_level" };
        static const juce::String ReleaseTime { "release_time" };

        static const juce::String BandType { "band_type" };
        static const juce::String BandFreq { "band_freq" };
        static const juce::String BandReso { "band_reso" };
        static const juce::String BandGain { "band_gain" };

        static const juce::String LfoEnable { "lfo_enable" };
        static const juce::String LfoType { "lfo_type" };
        static const juce::String LfoFreq { "lfo_freq" };
    }

    namespace Name
    {
        static const juce::String WaveType { "Wave Type" };

        static const juce::String Trigger { "Gate" };
        static const juce::String Analog { "Analog Style" };
        static const juce::String AttackTime { "Attack Time" };
        static const juce::String DecayTime { "Decay Time" };
        static const juce::String SustainLevel { "Sustain Level" };
        static const juce::String ReleaseTime { "Release Time" };

        static const juce::String BandType { "Band Type" };
        static const juce::String BandFreq { "Band Frequency" };
        static const juce::String BandReso { "Band Resonance" };
        static const juce::String BandGain { "Band Gain" };

        static const juce::String LfoEnable { "Low Freq. Osc. Enable"};
        static const juce::String LfoType { "Low Freq. Osc. Type" };
        static const juce::String LfoFreq { "Low Freq. Osc. Frequency" };
    }

    namespace Ranges
    {
        static const juce::StringArray WaveType { "Sine", "Tri. Aliased", "Saw Aliased", "Tri. AA", "Saw AA" };
    
        static constexpr float TimeMin { 1.f };
        static constexpr float TimeMax { 1000.f };
        static constexpr float TimeInc { 0.1f };
        static constexpr float TimeSkw { 0.5f };

        static constexpr float SustainMin { 0.f };
        static constexpr float SustainMax { 1.f };
        static constexpr float SustainInc { 0.01f };
        static constexpr float SustainSkw { 1.f };

        static const juce::String AnalogOn { "Analog" };
        static const juce::String AnalogOff { "Digital" };

        static constexpr float FreqMin { 20.f };
        static constexpr float FreqMax { 20000.f };
        static constexpr float FreqInc { 1.f };
        static constexpr float FreqSkw { 0.3f };

        static constexpr float ResoMin { 0.1f };
        static constexpr float ResoMax { 10.f };
        static constexpr float ResoInc { 0.01f };
        static constexpr float ResoSkw { 0.5f };

        static constexpr float GainMin { -24.f };
        static constexpr float GainMax { 24.f };
        static constexpr float GainInc { 0.1f };
        static constexpr float GainSkw { 1.f };

        static const juce::StringArray BandTypes { "Flat", "High Pass", "Low Shelf", "Peak", "Low Pass", "High Shelf" };
    
        static const juce::String LfoDisabled {"Off" };
        static const juce::String LfoEnabled { "On" };
        static const juce::StringArray LfoTypes { "Sin", "TriAliased", "SawAliased", "TriAA", "SawAA" };
        
        static constexpr float LfoFreqMin { 0.f };
        static constexpr float LfoFreqMax { 20.f };
        static constexpr float LfoFreqInc { 0.01f };
        static constexpr float LfoFreqSkw { 0.1f };
    }

    namespace Unit
    {
        static const juce::String Ms { "ms" };
        static const juce::String Freq { "Hz" };
        static const juce::String Gain { "dB" };
    }
}

class AssignmentSynthAudioProcessor : public juce::AudioProcessor
{
public:
    AssignmentSynthAudioProcessor();
    ~AssignmentSynthAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    mrta::ParameterManager& getParameterManager() { return parameterManager; }

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;
    //==============================================================================

private:
    mrta::ParameterManager parameterManager;
    juce::Synthesiser synth;
    DSP::SynthVoiceEnvelope* voice { nullptr };
    DSP::ParametricEqualizer equalizer;
    DSP::Oscillator lfo;
    bool lfo_enabled = false;

    DSP::Ramp<float> lfo_freq;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AssignmentSynthAudioProcessor)
};