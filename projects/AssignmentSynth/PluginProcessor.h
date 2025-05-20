#pragma once

#include <JuceHeader.h>

#include "ParametricEqualizer.h"
#include "StateVariableFilter.h"
#include "SynthVoice.h"
#include "EnvelopeGenerator.h"
#include "SynthVoiceEnvelope.h"
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

        static const float FreqMin { 20.f };
        static const float FreqMax { 20000.f };
        static const float FreqInc { 1.f };
        static const float FreqSkw { 0.3f };

        static const float ResoMin { 0.1f };
        static const float ResoMax { 10.f };
        static const float ResoInc { 0.01f };
        static const float ResoSkw { 0.5f };

        static const float GainMin { -24.f };
        static const float GainMax { 24.f };
        static const float GainInc { 0.1f };
        static const float GainSkw { 1.f };

        static const juce::StringArray BandTypes { "Flat", "High Pass", "Low Shelf", "Peak", "Low Pass", "High Shelf" };
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
    // DSP::EnvelopeGenerator envelope;
    DSP::ParametricEqualizer equalizer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AssignmentSynthAudioProcessor)
};