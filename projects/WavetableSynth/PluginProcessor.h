#pragma once

#include <JuceHeader.h>
#include "Parameter.h"
#include "WavetableSynth.h"
#include "LFO.h"
#include "juce_core/juce_core.h"
#include "ParameterHistoryBuffer.h"

namespace Param
{
    namespace ID
    {
        static const juce::String WavetablePosition { "wavetable_position" };
        static const juce::String WavetableVol { "wavetable_volume" };
        
        static const juce::String OutputVol { "output_vol" };

        static const juce::String UnisonVoices { "unison_voices" };
        static const juce::String UnisonDetune { "unison_detune" };

        static const juce::String VCF_AttTime { "vcf_att_time" };
        static const juce::String VCF_DecayTime { "vcf_decay_time" };
        static const juce::String VCF_Sustain { "vcf_sustain" };
        static const juce::String VCF_RelTime { "vcf_rel_time" };

        static const juce::String VCF_LFOFreq { "vcf_lfo_freq" };
        static const juce::String VCF_LFOType { "vcf_lfo_type" };

        static const juce::String VCF_Cutoff { "vcf_cutoff" };
        static const juce::String VCF_Reso { "vcf_reso" };
        static const juce::String VCF_Type { "vcf_type" };
        static const juce::String VCF_EnvAmount { "vcf_env_amount" };
        static const juce::String VCF_LFOAmount { "vcf_lfo_amount" };

        static const juce::String FinalVol { "final_vol" };
        
        static const juce::String LFO1_Freq { "lfo1_freq" };
        static const juce::String LFO1_Type { "lfo1_type" };
        static const juce::String LFO1_Offset { "lfo1_offset" };
        static const juce::String LFO1_mult { "lfo_amount" };

        static const juce::String LFO2_Freq { "lfo2_freq" };
        static const juce::String LFO2_Type { "lfo2_type" };
        static const juce::String LFO2_Offset { "lfo2_offset" };

        static const juce::String HistoryPlotBufferSize { "history_plot_buffer_size" };

        static const juce::String EnvelopeAttackTime { "envelope_attack_time" };
        static const juce::String EnvelopeDecayTime { "envelope_decay_time" };
        static const juce::String EnvelopeSustain { "envelope_sustain" };
        static const juce::String EnvelopeReleaseTime { "envelope_release_time" };

        static const juce::String EnvelopeAttackCurveX { "envelope_attack_curve_x" };
        static const juce::String EnvelopeAttackCurveY { "envelope_attack_curve_y" };
        static const juce::String EnvelopeDecayCurveX { "envelope_decay_curve_x" };
        static const juce::String EnvelopeDecayCurveY { "envelope_decay_curve_y" };
        static const juce::String EnvelopeReleaseCurveX { "envelope_release_curve_x" };
        static const juce::String EnvelopeReleaseCurveY { "envelope_release_curve_y" };
    }

    namespace Name
    {
        static const juce::String WavetablePos { "Wavetable Pos." };
        static const juce::String WavetableVol { "Wave. Vol." };
        static const juce::String OutputVol { "Output Vol." };

        static const juce::String UnisonVoices { "Unison Voices" };
        static const juce::String UnisonDetune { "Unison Detune" };

        static const juce::String VCF_AttTime { "VCF Attack Time" };
        static const juce::String VCF_DecayTime { "VCF Decay Time" };
        static const juce::String VCF_Sustain { "VCF Sustain" };
        static const juce::String VCF_RelTime { "VCF Release Time" };

        static const juce::String VCF_LFOFreq { "VCF LFO Freq." };
        static const juce::String VCF_LFOType { "VCF LFO Type" };

        static const juce::String VCF_Cutoff { "VCF Cutoff" };
        static const juce::String VCF_Reso { "VCF Resonance" };
        static const juce::String VCF_Type { "VCF Type" };

        static const juce::String VCF_EnvAmount { "VCF Env. Amount" };
        static const juce::String VCF_LFOAmount { "VCF LFO Amount" };

        static const juce::String FinalVol { "Final Vol." };
        
        static const juce::String LFO1_Freq { "LFO 1 Freq." };
        static const juce::String LFO1_Type { "LFO 1 Type" };
        static const juce::String LFO1_Offset { "LFO 1 Offset" };
        static const juce::String LFO1_mult {"LFO volume multiplier" };

        static const juce::String LFO2_Freq { "LFO 2 Freq." };
        static const juce::String LFO2_Type { "LFO 2 Type" };
        static const juce::String LFO2_Offset { "LFO 2 Offset" };

        static const juce::String HistoryPlotBufferSize { "History Plot Buffer Size" };

        static const juce::String EnvelopeAttackTime { "Attack" };
        static const juce::String EnvelopeDecayTime { "Decay" };
        static const juce::String EnvelopeSustain { "Sustain" };
        static const juce::String EnvelopeReleaseTime { "Release" };

        static const juce::String EnvelopeAttackCurveX { "Attack Curve X" };
        static const juce::String EnvelopeAttackCurveY { "Attack Curve Y" };
        static const juce::String EnvelopeDecayCurveX { "Decay Curve X" };
        static const juce::String EnvelopeDecayCurveY { "Decay Curve Y" };
        static const juce::String EnvelopeReleaseCurveX { "Release Curve X" };
        static const juce::String EnvelopeReleaseCurveY { "Release Curve Y" };
    }

    namespace Ranges
    {
        static constexpr float WavetablePositionMin { 0 };
        static constexpr float WavetablePositionMax { 3 };

        static constexpr float UnisonVoicesMin { 1 };
        static constexpr float UnisonVoicesMax { 16 };
        static constexpr float UnisonVoicesInc { 1.f };
        static constexpr float UnisonVoicesSkw { 1.f };

        static constexpr float UnisonDetuneMin { 0.f };
        static constexpr float UnisonDetuneMax { 100.f };
        static constexpr float UnisonDetuneInc { 0.01f };
        static constexpr float UnisonDetuneSkw { 1.f };

        static constexpr float VolMin { -60.f };
        static constexpr float VolMax { 12.f };
        static constexpr float VolInc { 0.1f };
        static constexpr float VolSkw { 2.8f };

        static constexpr float EnvTimeMin { 1.f };
        static constexpr float EnvTimeMax { 1000.f };
        static constexpr float EnvTimeInc { 1.f };
        static constexpr float EnvTimeSkw { 0.5f };

        static constexpr float EnvSustainMin { 0.f };
        static constexpr float EnvSustainMax { 1.f };
        static constexpr float EnvSustainInc { 0.001f };
        static constexpr float EnvSustainSkw { 1.f };

        static constexpr float LFOFreqMin { 0.1f };
        static constexpr float LFOFreqMax { 50.f };
        static constexpr float LFOFreqInc { 0.1f };
        static constexpr float LFOFreqSkw { 0.25f };

        static constexpr float LFOOffsetMin { -1.f };
        static constexpr float LFOOffsetMax { 1.f };
        static constexpr float LFOOffsetInc { 0.001f };
        static constexpr float LFOOffsetSkw { 1.f };

        static constexpr float LFODefaultFreq { 0.5f };

        static constexpr float FilterFreqMin { 20.0f };
        static constexpr float FilterFreqMax { 20000.f };
        static constexpr float FilterFreqInc { 1.f };
        static constexpr float FilterFreqSkw { 0.5f };

        static constexpr float FilterResoMin { 0.5f };
        static constexpr float FilterResoMax { 10.f };
        static constexpr float FilterResoInc { 0.01f };
        static constexpr float FilterResoSkw { 0.5f };

        static constexpr float AmountMin { -1.f };
        static constexpr float AmountMax { 1.f };
        static constexpr float AmountInc { 0.001f };
        static constexpr float AmountSkw { 1.f };

        static constexpr int HistoryPlotBufferSizeMin { 512 };
        static constexpr int HistoryPlotBufferSizeMax { 32768 };
        static constexpr int HistoryPlotBufferSizeInc { 512 };
        static constexpr int HistoryPlotBufferSizeSkw { 1 };

        static const juce::StringArray LFOType { "Sin", "Tri" };
        static const juce::StringArray FilterType { "Low Pass", "Band Pass", "High Pass" };
        static const juce::StringArray LFO1Type { "Sine", "Square", "Triangle", "Sawtooth" };

        static constexpr float EnvelopeTimeMin { 0.0f };
        static constexpr float EnvelopeTimeMax { 2000.f }; // Is not used - ADSRPlotWidth is used instead
        static constexpr float EnvelopeTimeInc { 0.1f };
        static constexpr float EnvelopeTimeSkw { 0.5f };

        static constexpr float EnvelopeLevelMin { 0.0f };
        static constexpr float EnvelopeLevelMax { 1.0f };
        static constexpr float EnvelopeLevelInc { 0.001f };
        static constexpr float EnvelopeLevelSkw { 1.0f };

        static constexpr float AttackDefault { 950.0f };
        static constexpr float DecayDefault { 1150.0f };
        static constexpr float SustainDefault { 0.7f };
        static constexpr float ReleaseDefault { 400.0f };

        static constexpr float ADSRPlotWidth { 4000.f }; // In milliseconds
    }

    namespace Units
    {
        static const juce::String Hz { "Hz" };
        static const juce::String dB { "dB" };
        static const juce::String Ms { "ms" };
        static const juce::String Cent { "cent" };
    }
}

class WavetableSynthAudioProcessor : public juce::AudioProcessor
{
public:
    WavetableSynthAudioProcessor();
    ~WavetableSynthAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    mrta::ParameterManager& getParamManager() { return paramManager; }

    /** Called by the editor to grab & clear the LFO history. */
    void getLastLfo1Values (std::vector<float>& outValues);
    void getLastLfo2Values (std::vector<float>& outValues);

    DSP::EnvelopeStateCollector* getEnvelopeStateCollector() const;

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

    static constexpr size_t NUM_VOICES { 8 };

    std::optional<std::reference_wrapper<const juce::String>> selectedParameter {};

private:
    mrta::ParameterManager paramManager;
    std::vector<DSP::WavetableSynthVoice *> voices;
    juce::Synthesiser synth;

    DSP::LFO lfo1;
    DSP::LFO lfo2;

    DSP::Parameter<float> volume;

    DSP::ParameterHistoryBuffer<float> lfo1History { 32768 };  // remember up to 2^15 samples
    DSP::ParameterHistoryBuffer<float> lfo2History { 32768 };  // remember up to 2^15 samples

    std::unique_ptr<DSP::EnvelopeStateCollector> envelopeCollector;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableSynthAudioProcessor)
};