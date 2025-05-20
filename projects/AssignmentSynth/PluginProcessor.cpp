#include "PluginProcessor.h"
#include "JuceHeader.h"
#include "Oscillator.h"
#include "ParametricEqualizer.h"
#include "PluginEditor.h"
#include "SynthSound.h"
#include "SynthVoiceEnvelope.h"
#include "mrta_utils/Source/Parameter/ParameterInfo.h"
#include "mrta_utils/Source/Parameter/ParameterManager.h"
#include <vector>


static const std::vector<mrta::ParameterInfo> paramVector
{
    { Param::ID::WaveType, Param::Name::WaveType, Param::Ranges::WaveType, DSP::Oscillator::Sin },
    { Param::ID::Analog,       Param::Name::Analog,       Param::Ranges::AnalogOff,  Param::Ranges::AnalogOn, false },
    { Param::ID::AttackTime,   Param::Name::AttackTime,   Param::Unit::Ms,  50.0f, Param::Ranges::TimeMin,    Param::Ranges::TimeMax,    Param::Ranges::TimeInc,    Param::Ranges::TimeSkw },
    { Param::ID::DecayTime,    Param::Name::DecayTime,    Param::Unit::Ms,  25.0f, Param::Ranges::TimeMin,    Param::Ranges::TimeMax,    Param::Ranges::TimeInc,    Param::Ranges::TimeSkw },
    { Param::ID::SustainLevel, Param::Name::SustainLevel, "",                0.7f, Param::Ranges::SustainMin, Param::Ranges::SustainMax, Param::Ranges::SustainInc, Param::Ranges::SustainSkw },
    { Param::ID::ReleaseTime,  Param::Name::ReleaseTime,  Param::Unit::Ms, 100.0f, Param::Ranges::TimeMin,    Param::Ranges::TimeMax,    Param::Ranges::TimeInc,    Param::Ranges::TimeSkw },
    { Param::ID::BandType, Param::Name::BandType, Param::Ranges::BandTypes, DSP::ParametricEqualizer::LowShelf },
    { Param::ID::BandFreq, Param::Name::BandFreq, Param::Unit::Freq, 10000.f, Param::Ranges::FreqMin, Param::Ranges::FreqMax, Param::Ranges::FreqInc, Param::Ranges::FreqSkw },
    { Param::ID::BandReso, Param::Name::BandReso, "", 0.71f, Param::Ranges::ResoMin, Param::Ranges::ResoMax, Param::Ranges::ResoInc, Param::Ranges::ResoSkw },
    { Param::ID::BandGain, Param::Name::BandGain, Param::Unit::Gain, 0.f, Param::Ranges::GainMin, Param::Ranges::GainMax, Param::Ranges::GainInc, Param::Ranges::GainSkw },
    { Param::ID::LfoEnable, Param::Name::LfoEnable, Param::Ranges::LfoDisabled, Param::Ranges::LfoEnabled, false},
    { Param::ID::LfoType, Param::Name::LfoType, Param::Ranges::LfoTypes, DSP::Oscillator::Sin },
    { Param::ID::LfoFreq, Param::Name::LfoFreq, Param::Unit::Freq, 1.f, Param::Ranges::LfoFreqMin, Param::Ranges::LfoFreqMax, Param::Ranges::LfoFreqInc, Param::Ranges::LfoFreqSkw }
};

AssignmentSynthAudioProcessor::AssignmentSynthAudioProcessor() :
    parameterManager(*this, ProjectInfo::projectName, paramVector),
    equalizer(1)
{
    voice = new DSP::SynthVoiceEnvelope();
    synth.addSound(new DSP::SynthSound());
    synth.addVoice(voice);

    parameterManager.registerParameterCallback(Param::ID::WaveType,
    [this] (float value, bool /*force*/)
    {
        DSP::Oscillator::OscType type = static_cast<DSP::Oscillator::OscType>(std::rint(value));
        voice->setWaveType(type);
    });
    
    parameterManager.registerParameterCallback(Param::ID::Analog,
    [this] (float value, bool /*force*/)
    {
        voice->setAnalogStyle(value > 0.5f);
    });

    parameterManager.registerParameterCallback(Param::ID::AttackTime,
    [this] (float value, bool force)
    {
        voice->setAttackTime(value);
    });

    parameterManager.registerParameterCallback(Param::ID::DecayTime,
    [this] (float value, bool force)
    {
        voice->setDecayTime(value);
    });

    parameterManager.registerParameterCallback(Param::ID::SustainLevel,
    [this] (float value, bool force)
    {
        voice->setSustainLevel(value);
    });

    parameterManager.registerParameterCallback(Param::ID::ReleaseTime,
    [this] (float value, bool force)
    {
        voice->setReleaseTime(value);
    });

    parameterManager.registerParameterCallback(Param::ID::BandType,
    [this] (float val, bool /*force*/)
    {
        equalizer.setBandType(0, static_cast<DSP::ParametricEqualizer::FilterType>(std::round(val)));
    });

    parameterManager.registerParameterCallback(Param::ID::BandFreq,
    [this] (float value, bool force)
    {
        equalizer.setBandFrequency(0, value);
        eqFreq.setTarget(value, force);
    });

    parameterManager.registerParameterCallback(Param::ID::BandReso,
    [this] (float value, bool force)
    {
        equalizer.setBandResonance(0, value);
        eqReso.setTarget(value, force);
    });

    parameterManager.registerParameterCallback(Param::ID::BandGain,
    [this] (float value, bool force)
    {
        equalizer.setBandGain(0, value);
        eqGain.setTarget(value, force);
    });

    parameterManager.registerParameterCallback(Param::ID::LfoEnable,
    [this](float value, bool /* force */)
    {
        lfo_enabled = value > 0.5f;
    });

    parameterManager.registerParameterCallback(Param::ID::LfoType, 
    [this](float val, bool /* force */) 
    {
        lfo.setType(static_cast<DSP::Oscillator::OscType>(val));
    });

    parameterManager.registerParameterCallback(Param::ID::LfoFreq,
    [this](float val, bool force)
    {
        lfoFreq.setTarget(val, force);
    });

     // TODO: to fix issue with unset target on inital launch
     voice->setSustainLevel(0.7f);
}

AssignmentSynthAudioProcessor::~AssignmentSynthAudioProcessor()
{
}

void AssignmentSynthAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    unsigned int maxNumChannels = std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels());

    synth.setCurrentPlaybackSampleRate(sampleRate);
    equalizer.prepare(sampleRate, maxNumChannels);
    lfo.prepare(sampleRate);

    eqFreq.prepare(sampleRate);
    eqReso.prepare(sampleRate);
    eqGain.prepare(sampleRate);
    lfoFreq.prepare(sampleRate);

    parameterManager.parameterChanged(Param::ID::SustainLevel, 0.7f);
    parameterManager.updateParameters(true);
}

void AssignmentSynthAudioProcessor::releaseResources()
{
    equalizer.clear();
}

void AssignmentSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    parameterManager.updateParameters();
    
    const unsigned int numChannels{ static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples{ static_cast<unsigned int>(buffer.getNumSamples()) };

    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    for (int sample = 0; sample < numSamples; sample++) {
        float freq = 0;
        eqFreq.applySum(&freq, 1);
        equalizer.setBandFrequency(0, freq);

        float reso = 0;
        eqReso.applySum(&reso, 1);
        equalizer.setBandFrequency(0, reso);

        float gain = 0;
        eqGain.applySum(&gain, 1);
        equalizer.setBandFrequency(0, gain);

        for (int channel = 0; channel < numChannels; channel++){
            auto currentSample = buffer.getSample(channel, sample);
            float outSample;
            equalizer.process(&outSample, &currentSample, 1);
            buffer.setSample(channel, sample, outSample);
        }
    }

    if (lfo_enabled) {
        for (int sample = 0; sample < numSamples; sample++) {
            float current_lfoFreq = 0.0f;
            lfoFreq.applySum(&current_lfoFreq, 1);
            
            lfo.setFrequency(current_lfoFreq);
            auto lfo_out = lfo.process();
            
            for (int channel = 0; channel < numChannels; channel++){
                buffer.setSample(channel, sample, lfo_out * buffer.getSample(channel, sample));
            }
        }
    }
}

void AssignmentSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    parameterManager.getStateInformation(destData);
}

void AssignmentSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    parameterManager.setStateInformation(data, sizeInBytes);
}

bool AssignmentSynthAudioProcessor::acceptsMidi() const
{
    return true;
}

//==============================================================================
const juce::String AssignmentSynthAudioProcessor::getName() const { return JucePlugin_Name; }
bool AssignmentSynthAudioProcessor::producesMidi() const { return false; }
bool AssignmentSynthAudioProcessor::isMidiEffect() const { return false; }
double AssignmentSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int AssignmentSynthAudioProcessor::getNumPrograms() { return 1; }
int AssignmentSynthAudioProcessor::getCurrentProgram() { return 0; }
void AssignmentSynthAudioProcessor::setCurrentProgram(int) { }
const juce::String AssignmentSynthAudioProcessor::getProgramName(int) { return {}; }
void AssignmentSynthAudioProcessor::changeProgramName(int, const juce::String&) { }
bool AssignmentSynthAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* AssignmentSynthAudioProcessor::createEditor() { return new AssignmentSynthAudioProcessorEditor(*this); }
//==============================================================================

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AssignmentSynthAudioProcessor();
}