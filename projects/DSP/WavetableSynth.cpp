#include "LFO.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_core/juce_core.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include "WavetableSynth.h"

namespace DSP
{

// float naive_lerp(float a, float b, float t)
// {
//     return a + t * (b - a);
// }

float naive_lerp(float a, float b, float t)
{
    // Clamp t to [0, 1] if necessary
    t = std::clamp(t, 0.0f, 1.0f);

    float gainA = std::cos(t * juce::MathConstants<float>::halfPi);
    float gainB = std::sin(t * juce::MathConstants<float>::halfPi);

    return a * gainA + b * gainB;
}

float convertMidiNoteToFreq(int MidiNote)
{
    return 440.f * std::pow(2.f, static_cast<float>(MidiNote - 69) / 12.f);
}

// https://en.wikipedia.org/wiki/Cent_(music)#Use
float moveFrequencyByCents(float frequency, float cents)
{
    return frequency * std::pow(2, cents / 1200.f);
}

WavetableSynthVoice::WavetableSynthVoice()
:envGen(sampleRate)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    File file("~/Downloads/ef2ad2c11729050ed05430a86caf147dRick_Astley_-_Never_Gonna_Give_You_Up_Official_Mus.wav");
    auto* reader = formatManager.createReaderFor (file);
    if (reader == nullptr) return;

    const int numberOfChannels = 1;
    juce::AudioSampleBuffer buffer(numberOfChannels, reader->lengthInSamples);

    reader->read(buffer.getArrayOfWritePointers(), numberOfChannels, 0, reader->lengthInSamples);
    
    loadFromBuffer(buffer, reader->sampleRate);
}

void WavetableSynthVoice::fillWavetable()
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

void WavetableSynthVoice::clearWavetable()
{
    wavetables.clear();
}

void WavetableSynthVoice::loadFromBuffer(const AudioSampleBuffer& buffer, double bufferSampleRate)
{
    int numOfSamples = buffer.getNumSamples();
    // round it up, so we dont lose samples
    int numberOfWaveforms = (numOfSamples + SampleSize - 1) / SampleSize;
    wavetables.resize(numberOfWaveforms);
    // 440.f to make it normal on A4
    originalSampleFrequency = static_cast<double>(SampleSize) * 440.0 / bufferSampleRate;

    for (auto waveform = 0; waveform < numberOfWaveforms; waveform++) {
        auto end = std::min(SampleSize, numOfSamples - waveform * SampleSize);
        for (auto sample = 0; sample < end; sample++) {
            wavetables[waveform][sample] = buffer.getSample(0, sample + waveform * SampleSize);
        }
    }
}

WavetableSynthVoice::~WavetableSynthVoice()
{
}

void WavetableSynthVoice::setWavetablePosition(float index, bool skip) {
    wavetableIndex.setValue(std::clamp(index, 0.f, static_cast<float>(wavetables.size() - 1)), skip);
}

void WavetableSynthVoice::setWavetableVol(float dB, bool skipRamp)
{
    wavetableVolRamp.setValue(std::pow(10.f, 0.05f * dB), skipRamp);
}

void WavetableSynthVoice::setUnisonVoices(uint8_t numberOfVoices)
{
    unisonVoices = numberOfVoices;
    unisonPhases.resize(unisonVoices);
    unisonIncrements.resize(unisonVoices);
}

void WavetableSynthVoice::setUnisonDetune(float cents, bool skip)
{
    unisonDetune.setValue(cents, skip);
}

void WavetableSynthVoice::setAttTime(float ms)
{
    envGen.setAttackTime(ms);
}

void WavetableSynthVoice::setDecayTime(float ms)
{
    envGen.setDecayTime(ms);
}

void WavetableSynthVoice::setSustain(float norm)
{
    envGen.setSustainLevel(norm);
}

void WavetableSynthVoice::setRelTime(float ms)
{
    envGen.setReleaseTime(ms);
}

void WavetableSynthVoice::setAttCurveX(float x)
{
    envGen.setAttackCurveX(x);
}

void WavetableSynthVoice::setAttCurveY(float y)
{
    envGen.setAttackCurveY(y);
}

void WavetableSynthVoice::setDecayCurveX(float x)
{
    envGen.setDecayCurveX(x);
}

void WavetableSynthVoice::setDecayCurveY(float y)
{
    envGen.setDecayCurveY(y);
}

void WavetableSynthVoice::setRelCurveX(float x)
{
    envGen.setReleaseCurveX(x);
}

void WavetableSynthVoice::setRelCurveY(float y)
{
    envGen.setReleaseCurveY(y);
}

void WavetableSynthVoice::setFilterCutoff(float Hz, bool skipRamp)
{
    vcfFreq.setValue(std::clamp(Hz, MinFreqHz, MaxFreqHz), skipRamp);
}

void WavetableSynthVoice::setFilterReso(float Q, bool skipRamp)
{
    vcfReso.setValue(std::clamp(Q, MinReso, MaxReso), skipRamp);
}

void WavetableSynthVoice::setFilterType(FilterType type, bool skipRamp)
{
    vcfLPFRamp.setTarget(type == LPF ? 1.f : 0.f, skipRamp);
    vcfBPFRamp.setTarget(type == BPF ? 1.f : 0.f, skipRamp);
    vcfHPFRamp.setTarget(type == HPF ? 1.f : 0.f, skipRamp);
}

void WavetableSynthVoice::setOutputVol(float dB, bool skipRamp)
{
    outputVolRamp.setValue(std::pow(10.f, 0.05f * dB), skipRamp);
}

void WavetableSynthVoice::setEnvelopeMonitor(EnvelopeStateCollector& collector, size_t index)
{
    envelopeStateCollector = &collector;
    envelopeVoiceIndex = index;
}

bool WavetableSynthVoice::canPlaySound(juce::SynthesiserSound* ptr)
{
    return true;
}

float WavetableSynthVoice::getWavetableIncrement(float frequency, double originalFrequency, size_t sampleSize, double sampleRate)
{
    return static_cast<float>(
        static_cast<double>(frequency) / originalFrequency * static_cast<double>(SampleSize) / sampleRate
    );
}

void WavetableSynthVoice::updateUnisonIncrements()
{
    for (auto unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
        // to make it like: [0, 1, -1, 2, -2, 3, -3, ...]
        auto unisonMultiplier = 0;
        if (unisonVoice > 0) {
            unisonMultiplier = (static_cast<float>(unisonVoice) - 1) / 2;
            unisonMultiplier = std::copysign(unisonMultiplier, (static_cast<float>(unisonVoice % 2)) - 1);
        }
        const auto unisonFrequency = moveFrequencyByCents(frequency, unisonMultiplier * unisonDetune.getCurrentValue());
        unisonIncrements[unisonVoice] = getWavetableIncrement(unisonFrequency, originalSampleFrequency, SampleSize, sampleRate);
    }
}

void WavetableSynthVoice::startNote(int midiNoteNumber, float newVelocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    frequency = convertMidiNoteToFreq(midiNoteNumber);

    updateUnisonIncrements();

    for (auto unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
        unisonPhases[unisonVoice] = 0.f;
    }

    velocity = newVelocity;
    voiceStarted = true;
    gateState = true;
}

void WavetableSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    gateState = false;

    if (!allowTailOff)
        clearCurrentNote();
}

void WavetableSynthVoice::pitchWheelMoved(int newPitchWheelValue)
{
}

void WavetableSynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
}

void WavetableSynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    const auto newSampleRate { getSampleRate() };
    if (sampleRate != newSampleRate)
    {
        sampleRate = newSampleRate;

        envGen.prepare(sampleRate);
        filter.prepare(sampleRate);
        wavetableVolRamp.prepare(sampleRate);
        outputVolRamp.prepare(sampleRate);
        unisonDetune.prepare(sampleRate);
        vcfFreq.prepare(sampleRate);
        vcfReso.prepare(sampleRate);
        vcfLPFRamp.prepare(sampleRate);
        vcfBPFRamp.prepare(sampleRate);
        vcfHPFRamp.prepare(sampleRate);
    }

    for (int i = 0; i < numSamples; ++i)
    {
        unisonDetune.getNext();

        float envValue = envGen.getValue(gateState);

        // Send envelope state to the GUI collector
        if (envelopeStateCollector != nullptr)
            envelopeStateCollector->setEnvelopeState(envelopeVoiceIndex, envGen.getCurrentState(), envGen.getCurrentStateTimer());

        // Indices of which of the waveform in wavetable to use wavetables[integralindex]
        float integralIndexfloat = 0.f;
        const float fractionalIndex = std::modf(wavetableIndex.getNext(), &integralIndexfloat);
        const auto integralIndex = static_cast<size_t>(integralIndexfloat);

        for (auto unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
            unisonPhases[unisonVoice] = std::fmod( unisonPhases[unisonVoice] + unisonIncrements[unisonVoice], SampleSize);
        }

        const auto wavetableVol { wavetableVolRamp.getNext() };

        const auto vcfFreqCurrent { vcfFreq.getNext() };
        const auto vcfResoCurrent { vcfReso.getNext() };
        const auto vcfLPF { vcfLPFRamp.getNext() };
        const auto vcfBPF { vcfBPFRamp.getNext() };
        const auto vcfHPF { vcfHPFRamp.getNext() };

        const auto outputVol { outputVolRamp.getNext() };

        float wavetableOut = 0.f;
        for (int unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
            const auto unisonPhaseInteger = static_cast<size_t>(unisonPhases[unisonVoice]);
            const auto unisonLerped = naive_lerp(wavetables[integralIndex][unisonPhaseInteger], wavetables[(integralIndex + 1) % wavetables.size()][unisonPhaseInteger], fractionalIndex);
            const auto unisonOut { unisonLerped };

            wavetableOut += unisonOut;
        }
        wavetableOut /= unisonVoices;
        wavetableOut *= (wavetableVol * envValue * velocity);

        float lpfOut { 0.f };
        float bpfOut { 0.f };
        float hpfOut { 0.f };
        filter.process(&lpfOut, &bpfOut, &hpfOut, &wavetableOut, &vcfFreqCurrent, &vcfResoCurrent, 1);

        const auto out { (vcfLPF * lpfOut + vcfBPF * bpfOut + vcfHPF * hpfOut) * outputVol };
        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        {
            outputBuffer.addSample(ch, startSample + i, out);
        }

        if (std::isnan(out))
        {
            voiceStarted = false;
            clearCurrentNote();
            DBG("WavetableSynthVoice: NaN detected in output buffer");
            return;
        }

        if (voiceStarted && envGen.isOff())
        {
            voiceStarted = false;
            clearCurrentNote();
        }
    }
}

}
