#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "WavetableSynth.h"
#include <algorithm>
#include <vector>

void setWavetablePosition(std::vector<DSP::WavetableSynthVoice *> voices, float position, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [position, skipRamp] (auto& v) { v->setWavetablePosition(position, skipRamp);});
}

void setWavetableVol(std::vector<DSP::WavetableSynthVoice *> voices, float dB, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [dB, skipRamp] (auto& v) { v->setWavetableVol(dB, skipRamp); });
}

void setAttTime(std::vector<DSP::WavetableSynthVoice*> voices, float ms)
{
    std::for_each(voices.begin(), voices.end(), [ms] (auto& v) { v->setAttTime(ms); });
}

void setDecayTime(std::vector<DSP::WavetableSynthVoice*> voices, float ms)
{
    std::for_each(voices.begin(), voices.end(), [ms] (auto& v) { v->setDecayTime(ms); });
}

void setSustain(std::vector<DSP::WavetableSynthVoice*> voices, float norm)
{
    std::for_each(voices.begin(), voices.end(), [norm] (auto& v) { v->setSustain(norm); });
}

void setRelTime(std::vector<DSP::WavetableSynthVoice*> voices, float ms)
{
    std::for_each(voices.begin(), voices.end(), [ms] (auto& v) { v->setRelTime(ms); });
}

void setAttCurveX(std::vector<DSP::WavetableSynthVoice*> voices, float x)
{
    std::for_each(voices.begin(), voices.end(), [x] (auto& v) { v->setAttCurveX(x); });
}

void setAttCurveY(std::vector<DSP::WavetableSynthVoice*> voices, float y)
{
    std::for_each(voices.begin(), voices.end(), [y] (auto& v) { v->setAttCurveY(y); });
}

void setDecayCurveX(std::vector<DSP::WavetableSynthVoice*> voices, float x)
{
    std::for_each(voices.begin(), voices.end(), [x] (auto& v) { v->setDecayCurveX(x); });
}

void setDecayCurveY(std::vector<DSP::WavetableSynthVoice*> voices, float y)
{
    std::for_each(voices.begin(), voices.end(), [y] (auto& v) { v->setDecayCurveY(y); });
}

void setRelCurveX(std::vector<DSP::WavetableSynthVoice*> voices, float x)
{
    std::for_each(voices.begin(), voices.end(), [x] (auto& v) { v->setRelCurveX(x); });
}

void setRelCurveY(std::vector<DSP::WavetableSynthVoice*> voices, float y)
{
    std::for_each(voices.begin(), voices.end(), [y] (auto& v) { v->setRelCurveY(y); });
}

void setAttTimeVCF(std::vector<DSP::WavetableSynthVoice*> voices, float ms)
{
    std::for_each(voices.begin(), voices.end(), [ms] (auto& v) { v->setAttTimeVCF(ms); });
}

void setDecayTimeVCF(std::vector<DSP::WavetableSynthVoice*> voices, float ms)
{
    std::for_each(voices.begin(), voices.end(), [ms] (auto& v) { v->setDecayTimeVCF(ms); });
}

void setSustainVCF(std::vector<DSP::WavetableSynthVoice*> voices, float norm)
{
    std::for_each(voices.begin(), voices.end(), [norm] (auto& v) { v->setSustainVCF(norm); });
}

void setRelTimeVCF(std::vector<DSP::WavetableSynthVoice*> voices, float ms)
{
    std::for_each(voices.begin(), voices.end(), [ms] (auto& v) { v->setRelTimeVCF(ms); });
}

void setLFOFreqVCF(std::vector<DSP::WavetableSynthVoice*> voices, float Hz)
{
    std::for_each(voices.begin(), voices.end(), [Hz] (auto& v) { v->setLFOFreqVCF(Hz); });
}

void setLFOTypeVCF(std::vector<DSP::WavetableSynthVoice*> voices, DSP::WavetableSynthVoice::LFOType type)
{
    std::for_each(voices.begin(), voices.end(), [type] (auto& v) { v->setLFOTypeVCF(type); });
}

void setEnvAmountVCF(std::vector<DSP::WavetableSynthVoice*> voices, float bipolar, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [bipolar, skipRamp] (auto& v) { v->setEnvAmountVCF(bipolar, skipRamp); });
}

void setLFOAmountVCF(std::vector<DSP::WavetableSynthVoice*> voices, float bipolar, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [bipolar, skipRamp] (auto& v) { v->setLFOAmountVCF(bipolar, skipRamp); });
}

void setFilterCutoff(std::vector<DSP::WavetableSynthVoice*> voices, float Hz, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [Hz, skipRamp] (auto& v) { v->setFilterCutoff(Hz, skipRamp); });
}

void setFilterReso(std::vector<DSP::WavetableSynthVoice*> voices, float Q, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [Q, skipRamp] (auto& v) { v->setFilterReso(Q, skipRamp); });
}

void setFilterType(std::vector<DSP::WavetableSynthVoice*> voices, DSP::WavetableSynthVoice::FilterType type, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [type, skipRamp] (auto& v) { v->setFilterType(type, skipRamp); });
}

void setOutputVol(std::vector<DSP::WavetableSynthVoice*> voices, float dB, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [dB, skipRamp] (auto& v) { v->setOutputVol(dB, skipRamp); });
}

void setLfoMult(std::vector<DSP::WavetableSynthVoice*> voices, juce::String paramId, float value, DSP::DSP<float> &reference)
{
    std::for_each(voices.begin(), voices.end(), [paramId, value, &reference] (auto& v) { v->setWavetablePositionEffect(paramId, Param::Ranges::WavetablePositionMax * value, reference); });
}

static const std::vector<mrta::ParameterInfo> paramVector
{
    { Param::ID::WavetablePosition, Param::Name::WavetablePos, "", 0, Param::Ranges::WavetablePositionMin, Param::Ranges::WavetablePositionMax, .1f, 1.f },
    { Param::ID::WavetableVol,    Param::Name::WavetableVol,    Param::Units::dB,   0.f, Param::Ranges::VolMin, Param::Ranges::VolMax, Param::Ranges::VolInc, Param::Ranges::VolSkw },

    { Param::ID::VCA_AttTime,   Param::Name::VCA_AttTime,   Param::Units::Ms,  50.0f, Param::Ranges::EnvTimeMin,    Param::Ranges::EnvTimeMax,    Param::Ranges::EnvTimeInc,    Param::Ranges::EnvTimeSkw },
    { Param::ID::VCA_DecayTime, Param::Name::VCA_DecayTime, Param::Units::Ms,  10.0f, Param::Ranges::EnvTimeMin,    Param::Ranges::EnvTimeMax,    Param::Ranges::EnvTimeInc,    Param::Ranges::EnvTimeSkw },
    { Param::ID::VCA_Sustain,   Param::Name::VCA_Sustain,   "",                 0.7f, Param::Ranges::EnvSustainMin, Param::Ranges::EnvSustainMax, Param::Ranges::EnvSustainInc, Param::Ranges::EnvSustainSkw },
    { Param::ID::VCA_RelTime,   Param::Name::VCA_RelTime,   Param::Units::Ms, 100.0f, Param::Ranges::EnvTimeMin,    Param::Ranges::EnvTimeMax,    Param::Ranges::EnvTimeInc,    Param::Ranges::EnvTimeSkw },

    { Param::ID::VCF_AttTime,   Param::Name::VCF_AttTime,   Param::Units::Ms,  10.0f, Param::Ranges::EnvTimeMin,    Param::Ranges::EnvTimeMax,    Param::Ranges::EnvTimeInc,    Param::Ranges::EnvTimeSkw },
    { Param::ID::VCF_DecayTime, Param::Name::VCF_DecayTime, Param::Units::Ms, 100.0f, Param::Ranges::EnvTimeMin,    Param::Ranges::EnvTimeMax,    Param::Ranges::EnvTimeInc,    Param::Ranges::EnvTimeSkw },
    { Param::ID::VCF_Sustain,   Param::Name::VCF_Sustain,   "",                 0.9f, Param::Ranges::EnvSustainMin, Param::Ranges::EnvSustainMax, Param::Ranges::EnvSustainInc, Param::Ranges::EnvSustainSkw },
    { Param::ID::VCF_RelTime,  Param::Name::VCF_RelTime,   Param::Units::Ms, 100.0f, Param::Ranges::EnvTimeMin,    Param::Ranges::EnvTimeMax,    Param::Ranges::EnvTimeInc,    Param::Ranges::EnvTimeSkw },

    { Param::ID::VCF_LFOFreq, Param::Name::VCF_LFOFreq, Param::Units::Hz, 0.5f, Param::Ranges::LFOFreqMin, Param::Ranges::LFOFreqMax, Param::Ranges::LFOFreqInc, Param::Ranges::LFOFreqSkw },
    { Param::ID::VCF_LFOType, Param::Name::VCF_LFOType, Param::Ranges::LFOType, 0 },

    { Param::ID::VCF_Cutoff, Param::Name::VCF_Cutoff, Param::Units::Hz, 2000.f, Param::Ranges::FilterFreqMin, Param::Ranges::FilterFreqMax, Param::Ranges::FilterFreqInc, Param::Ranges::FilterFreqSkw },
    { Param::ID::VCF_Reso,   Param::Name::VCF_Reso,   "",                0.71f, Param::Ranges::FilterResoMin, Param::Ranges::FilterResoMax, Param::Ranges::FilterResoInc, Param::Ranges::FilterResoSkw },
    { Param::ID::VCF_Type,   Param::Name::VCF_Type,   Param::Ranges::FilterType, 0 },

    { Param::ID::VCF_EnvAmount, Param::Name::VCF_EnvAmount, "", 0.f, Param::Ranges::AmountMin, Param::Ranges::AmountMax, Param::Ranges::AmountInc, Param::Ranges::AmountSkw },
    { Param::ID::VCF_LFOAmount, Param::Name::VCF_LFOAmount, "", 0.f, Param::Ranges::AmountMin, Param::Ranges::AmountMax, Param::Ranges::AmountInc, Param::Ranges::AmountSkw },

    { Param::ID::OutputVol, Param::Name::OutputVol, Param::Units::dB, 0.f, Param::Ranges::VolMin, Param::Ranges::VolMax, Param::Ranges::VolInc, Param::Ranges::VolSkw },

    { Param::ID::FinalVol, Param::Name::FinalVol, Param::Units::dB, 0.f, Param::Ranges::VolMin, Param::Ranges::VolMax, Param::Ranges::VolInc, Param::Ranges::VolSkw },
    
    {Param::ID::LFO1_Freq, Param::Name::LFO1_Freq, Param::Units::Hz, Param::Ranges::LFODefaultFreq, Param::Ranges::LFOFreqMin, Param::Ranges::LFOFreqMax, Param::Ranges::LFOFreqInc, Param::Ranges::LFOFreqSkw },
    {Param::ID::LFO1_Offset, Param::Name::LFO1_Offset, "", 0.f, Param::Ranges::LFOOffsetMin, Param::Ranges::LFOOffsetMax, Param::Ranges::LFOOffsetInc, Param::Ranges::LFOOffsetSkw },
    {Param::ID::LFO1_Type, Param::Name::LFO1_Type, Param::Ranges::LFO1Type, 0},
    { Param::ID::LFO1_mult, Param::Name::LFO1_mult, "", 0.f, Param::Ranges::AmountMin, Param::Ranges::AmountMax, Param::Ranges::AmountInc, Param::Ranges::AmountSkw },

    {Param::ID::LFO2_Freq, Param::Name::LFO2_Freq, Param::Units::Hz, Param::Ranges::LFODefaultFreq, Param::Ranges::LFOFreqMin, Param::Ranges::LFOFreqMax, Param::Ranges::LFOFreqInc, Param::Ranges::LFOFreqSkw },
    {Param::ID::LFO2_Offset, Param::Name::LFO2_Offset, "", 0.f, Param::Ranges::LFOOffsetMin, Param::Ranges::LFOOffsetMax, Param::Ranges::LFOOffsetInc, Param::Ranges::LFOOffsetSkw },
    {Param::ID::LFO2_Type, Param::Name::LFO2_Type, Param::Ranges::LFO1Type, 0},

    { Param::ID::HistoryPlotBufferSize, Param::Name::HistoryPlotBufferSize, Param::Units::Ms, 512.f, Param::Ranges::HistoryPlotBufferSizeMin, Param::Ranges::HistoryPlotBufferSizeMax, Param::Ranges::HistoryPlotBufferSizeInc, Param::Ranges::HistoryPlotBufferSizeSkw },
    
    {Param::ID::EnvelopeAttackTime, Param::Name::EnvelopeAttackTime, Param::Units::Ms, Param::Ranges::AttackDefault, Param::Ranges::EnvelopeTimeMin, Param::Ranges::ADSRPlotWidth, Param::Ranges::EnvelopeTimeInc, Param::Ranges::EnvelopeTimeSkw },
    {Param::ID::EnvelopeDecayTime, Param::Name::EnvelopeDecayTime, Param::Units::Ms, Param::Ranges::DecayDefault, Param::Ranges::EnvelopeTimeMin, Param::Ranges::ADSRPlotWidth, Param::Ranges::EnvelopeTimeInc, Param::Ranges::EnvelopeTimeSkw },
    {Param::ID::EnvelopeSustain, Param::Name::EnvelopeSustain, "", Param::Ranges::SustainDefault, Param::Ranges::EnvelopeLevelMin, Param::Ranges::EnvelopeLevelMax, Param::Ranges::EnvelopeLevelInc, Param::Ranges::EnvelopeLevelSkw },
    {Param::ID::EnvelopeReleaseTime, Param::Name::EnvelopeReleaseTime, Param::Units::Ms, Param::Ranges::ReleaseDefault, Param::Ranges::EnvelopeTimeMin, Param::Ranges::ADSRPlotWidth, Param::Ranges::EnvelopeTimeInc, Param::Ranges::EnvelopeTimeSkw },

    {Param::ID::EnvelopeAttackCurveX, Param::Name::EnvelopeAttackCurveX, "", 0.0f, -Param::Ranges::ADSRPlotWidth, Param::Ranges::ADSRPlotWidth, 0.01f, 1.0f},
    {Param::ID::EnvelopeAttackCurveY, Param::Name::EnvelopeAttackCurveY, "", 0.0f, -Param::Ranges::EnvelopeLevelMax, Param::Ranges::EnvelopeLevelMax, 0.01f, 1.0f},
    {Param::ID::EnvelopeDecayCurveX, Param::Name::EnvelopeDecayCurveX, "", 0.0f, -Param::Ranges::ADSRPlotWidth, Param::Ranges::ADSRPlotWidth, 0.01f, 1.0f},
    {Param::ID::EnvelopeDecayCurveY, Param::Name::EnvelopeDecayCurveY, "", 0.0f, -Param::Ranges::EnvelopeLevelMax, Param::Ranges::EnvelopeLevelMax, 0.01f, 1.0f},
    {Param::ID::EnvelopeReleaseCurveX, Param::Name::EnvelopeReleaseCurveX, "", 0.0f, -Param::Ranges::ADSRPlotWidth, Param::Ranges::ADSRPlotWidth, 0.01f, 1.0f},
    {Param::ID::EnvelopeReleaseCurveY, Param::Name::EnvelopeReleaseCurveY, "", 0.0f, -Param::Ranges::EnvelopeLevelMax, Param::Ranges::EnvelopeLevelMax, 0.01f, 1.0f}
};

WavetableSynthAudioProcessor::WavetableSynthAudioProcessor() :
    paramManager(*this, ProjectInfo::projectName, paramVector),
    lfo1(DSP::LFO(44100.f, Param::Ranges::LFODefaultFreq, DSP::Waveform::Sine)),
    lfo2(DSP::LFO(44100.f, Param::Ranges::LFODefaultFreq, DSP::Waveform::Sine))
{
    synth.addSound(new DSP::SynthSound());
    for (size_t i = 0; i < NUM_VOICES; ++i)
    {
        voices.emplace_back(new DSP::WavetableSynthVoice());
        synth.addVoice(voices.back());
    }
    synth.setNoteStealingEnabled(false);

    paramManager.registerParameterCallback(Param::ID::WavetablePosition, [this] (float value, bool force) { setWavetablePosition(voices, value, force); });
    paramManager.registerParameterCallback(Param::ID::WavetableVol, [this] (float value, bool force) { setWavetableVol(voices, value, force); });
    paramManager.registerParameterCallback(Param::ID::VCF_AttTime, [this] (float value, bool force) { setAttTimeVCF(voices, value); });
    paramManager.registerParameterCallback(Param::ID::VCF_DecayTime, [this] (float value, bool force) { setDecayTimeVCF(voices, value); });
    paramManager.registerParameterCallback(Param::ID::VCF_Sustain, [this] (float value, bool force) { setSustainVCF(voices, value); });
    paramManager.registerParameterCallback(Param::ID::VCF_RelTime, [this] (float value, bool force) { setRelTimeVCF(voices, value); });
    paramManager.registerParameterCallback(Param::ID::VCF_LFOFreq, [this] (float value, bool force) { setLFOFreqVCF(voices, value); });
    paramManager.registerParameterCallback(Param::ID::VCF_LFOType, [this] (float value, bool force) { setLFOTypeVCF(voices, static_cast<DSP::WavetableSynthVoice::LFOType>(std::round(value))); });
    paramManager.registerParameterCallback(Param::ID::VCF_Cutoff, [this] (float value, bool force) { setFilterCutoff(voices, value, force); });
    paramManager.registerParameterCallback(Param::ID::VCF_Reso, [this] (float value, bool force) { setFilterReso(voices, value, force); });
    paramManager.registerParameterCallback(Param::ID::VCF_Type, [this] (float value, bool force) { setFilterType(voices, static_cast<DSP::WavetableSynthVoice::FilterType>(std::round(value)), force); });
    paramManager.registerParameterCallback(Param::ID::VCF_EnvAmount, [this] (float value, bool force) { setEnvAmountVCF(voices, value, force); });
    paramManager.registerParameterCallback(Param::ID::VCF_LFOAmount, [this] (float value, bool force) { setLFOAmountVCF(voices, value, force); });
    paramManager.registerParameterCallback(Param::ID::OutputVol, [this] (float value, bool force) { setOutputVol(voices, value, force); });

    paramManager.registerParameterCallback(Param::ID::LFO1_Freq, [this] (float value, bool force)  {lfo1.setFrequency(value);});
    paramManager.registerParameterCallback(Param::ID::LFO1_Offset, [this] (float value, bool force) { lfo1.setOffset(value); });
    paramManager.registerParameterCallback(Param::ID::LFO1_Type, [this] (float value, bool force) { lfo1.setWaveform(static_cast<DSP::Waveform>(std::round(value))); });
    paramManager.registerParameterCallback(Param::ID::LFO1_mult, [this] (float value, bool force) { setLfoMult(voices, Param::ID::LFO1_mult, value, lfo1); });


    paramManager.registerParameterCallback(Param::ID::LFO2_Freq, [this] (float value, bool force)  {lfo2.setFrequency(value);});
    paramManager.registerParameterCallback(Param::ID::LFO2_Offset, [this] (float value, bool force) { lfo2.setOffset(value); });
    paramManager.registerParameterCallback(Param::ID::LFO2_Type, [this] (float value, bool force) { lfo2.setWaveform(static_cast<DSP::Waveform>(std::round(value))); });

    paramManager.registerParameterCallback(Param::ID::EnvelopeAttackTime, [this] (float value, bool force) { setAttTime(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeDecayTime, [this] (float value, bool force) { setDecayTime(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeSustain, [this] (float value, bool force) { setSustain(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeReleaseTime, [this] (float value, bool force) { setRelTime(voices, value); });

    paramManager.registerParameterCallback(Param::ID::EnvelopeAttackCurveX, [this] (float value, bool force) { setAttCurveX(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeAttackCurveY, [this] (float value, bool force) { setAttCurveY(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeDecayCurveX, [this] (float value, bool force) { setDecayCurveX(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeDecayCurveY, [this] (float value, bool force) { setDecayCurveY(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeReleaseCurveX, [this] (float value, bool force) { setRelCurveX(voices, value); });
    paramManager.registerParameterCallback(Param::ID::EnvelopeReleaseCurveY, [this] (float value, bool force) { setRelCurveY(voices, value); });
}

WavetableSynthAudioProcessor::~WavetableSynthAudioProcessor()
{
}

void WavetableSynthAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    paramManager.updateParameters(true);
    synth.setCurrentPlaybackSampleRate(sampleRate);

    lfo1.setSampleRate(sampleRate);
    lfo2.setSampleRate(sampleRate);
}

void WavetableSynthAudioProcessor::releaseResources()
{
}

void WavetableSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    paramManager.updateParameters();

    buffer.clear();
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
        // Advance LFO for each sample
        lfo1.advancePhase();
        lfo1History.pushSample(lfo1.getValue());

        lfo2.advancePhase();
        lfo2History.pushSample(lfo2.getValue());
    }
}

void WavetableSynthAudioProcessor::getLastLfo1Values (std::vector<float>& outValues)
{
    lfo1History.getHistory (outValues);
}

void WavetableSynthAudioProcessor::getLastLfo2Values (std::vector<float>& outValues)
{
    lfo2History.getHistory (outValues);
}

void WavetableSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    paramManager.getStateInformation(destData);
}

void WavetableSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    paramManager.setStateInformation(data, sizeInBytes);
}

bool WavetableSynthAudioProcessor::acceptsMidi() const
{
    return true;
}

//==============================================================================
const juce::String WavetableSynthAudioProcessor::getName() const { return JucePlugin_Name; }
bool WavetableSynthAudioProcessor::producesMidi() const { return false; }
bool WavetableSynthAudioProcessor::isMidiEffect() const { return false; }
double WavetableSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int WavetableSynthAudioProcessor::getNumPrograms() { return 1; }
int WavetableSynthAudioProcessor::getCurrentProgram() { return 0; }
void WavetableSynthAudioProcessor::setCurrentProgram(int) { }
const juce::String WavetableSynthAudioProcessor::getProgramName(int) { return {}; }
void WavetableSynthAudioProcessor::changeProgramName(int, const juce::String&) { }
bool WavetableSynthAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* WavetableSynthAudioProcessor::createEditor() { return new WavetableSynthAudioProcessorEditor(*this); }
//==============================================================================

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}