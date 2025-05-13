#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "juce_core/juce_core.h"


static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::Enabled,   Param::Name::Enabled,   "Off", "On", true },
    { Param::ID::Frequency, Param::Name::Frequency, "Hz", 1000.f, 20.f, 20000.f, 1.f, 0.3f },
    { Param::ID::PostGain,  Param::Name::PostGain,  "dB", 0.0f, -60.f, 12.f, 0.1f, 3.8018f },
};

MainProcessor::MainProcessor() :
    parameterManager(*this, ProjectInfo::projectName, ParameterInfos),
    phase(0.0f)
{
    parameterManager.registerParameterCallback(Param::ID::Enabled,
    [this] (float value, bool /*forced*/)
    {
        DBG(Param::Name::Enabled + ": " + juce::String { value });
        enabled = value > 0.5f;
    });

    parameterManager.registerParameterCallback(Param::ID::Frequency,
    [this] (float value, bool /*forced*/)
    {
        DBG(Param::Name::Frequency + ": " + juce::String { value });
        frequency = (value);
    });

    parameterManager.registerParameterCallback(Param::ID::PostGain,
    [this] (float value, bool forced)
    {
        DBG(Param::Name::PostGain + ": " + juce::String { value });
        float dbValue { 0.f };
        if (value > -60.f)
            dbValue = std::pow(10.f, value * 0.05f);

        if (forced) {
            phase = 0.0f;
            outputGain.setCurrentAndTargetValue(dbValue);
        } else {
            outputGain.setTargetValue(dbValue);
        }
    });
}

MainProcessor::~MainProcessor()
{
}

void MainProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::uint32 numChannels { static_cast<juce::uint32>(std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };
    // filter.prepare({ sampleRate, static_cast<juce::uint32>(samplesPerBlock), numChannels });
    sampleRate = sampleRate;
    outputGain.reset(sampleRate, 0.01f);
    parameterManager.updateParameters(true);
}

void MainProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    parameterManager.updateParameters();
    if (!enabled) return;
    
 
    // {
    //     juce::dsp::AudioBlock<float> audioBlock(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples());
    //     // juce::dsp::ProcessContextReplacing<float> ctx(audioBlock);

    //     juce::HeapBlock<char> heapBlock;
    //     juce::dsp::AudioBlock<float> result(heapBlock, buffer.getNumChannels(), buffer.getNumSamples());
    //     juce::dsp::AudioBlock<float>::process(audioBlock, result, [this](float value) {
    //         auto result = value * ( 1.0f + cos(frequency * phase / sampleRate) );
    //         // phase += 1.0f;
    //         phase = fmod((phase + 1.0f), 2 * juce::MathConstants<float>::pi);
    //         return result;
    //     });

    //     result.copyTo(buffer);
    // }

    for (auto channel = 0; channel < buffer.getNumChannels(); channel++) {
      auto *samples = buffer.getWritePointer(channel);
      for (auto sample = 0; sample < buffer.getNumSamples(); sample++) {
        auto sample_data = samples[sample];
        auto amp = frequency * phase / sampleRate;
        buffer.setSample(channel, sample, sample_data * ( 1.0f + cos(amp)) );
        phase = fmod((phase + 1.0f), 2 * juce::MathConstants<float>::pi);
      }
    }

    // {
    //     juce::dsp::AudioBlock<float> audioBlock(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples());
    //     juce::dsp::ProcessContextReplacing<float> ctx(audioBlock);
    //     // filter.process(ctx);
    // }

    outputGain.applyGain(buffer, buffer.getNumSamples());

    // buffer.applyGain(1.0f);
}

void MainProcessor::releaseResources()
{
    // filter.reset();
}

void MainProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    parameterManager.getStateInformation(destData);
}

void MainProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    parameterManager.setStateInformation(data, sizeInBytes);
}

juce::AudioProcessorEditor* MainProcessor::createEditor()
{
    return new MainProcessorEditor(*this);
}

//==============================================================================
const juce::String MainProcessor::getName() const { return JucePlugin_Name; }
bool MainProcessor::acceptsMidi() const { return false; }
bool MainProcessor::producesMidi() const { return false; }
bool MainProcessor::isMidiEffect() const { return false; }
double MainProcessor::getTailLengthSeconds() const { return 0.0; }
int MainProcessor::getNumPrograms() { return 1; }
int MainProcessor::getCurrentProgram() { return 0; }
void MainProcessor::setCurrentProgram (int) { }
const juce::String MainProcessor::getProgramName(int) { return {}; }
void MainProcessor::changeProgramName(int, const juce::String&) { }
bool MainProcessor::hasEditor() const { return true; }
//==============================================================================

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MainProcessor();
}
