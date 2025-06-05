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

WavetableSynthVoice::WavetableSynthVoice(const Wavetable& initialWavetable)
: wavetable(initialWavetable)
, sampleRate(DefaultSampleRate)
, envGenA(sampleRate)
, envGenB(sampleRate)
{}

WavetableSynthVoice::~WavetableSynthVoice()
{
}

void WavetableSynthVoice::setWavetablePosition(float index, bool skip) {
    wavetableIndex.setValue(std::clamp(index, 0.f, 1.f), skip);
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

void WavetableSynthVoice::setAttTime(float ms, int env_id)
{
    if (env_id == 0) {
        envGenA.setAttackTime(ms);
    } else {
        envGenB.setAttackTime(ms);
    }
}

void WavetableSynthVoice::setDecayTime(float ms, int env_id)
{
    if (env_id == 0) {
        envGenA.setDecayTime(ms);
    } else {
        envGenB.setDecayTime(ms);
    }
}

void WavetableSynthVoice::setSustain(float norm, int env_id)
{
    if (env_id == 0) {
        envGenA.setSustainLevel(norm);
    } else {
        envGenB.setSustainLevel(norm);
    }
}

void WavetableSynthVoice::setRelTime(float ms, int env_id)
{
    if (env_id == 0) {
        envGenA.setReleaseTime(ms);
    } else {
        envGenB.setReleaseTime(ms);
    }
}

void WavetableSynthVoice::setAttCurveX(float x, int env_id)
{
    if (env_id == 0) {
        envGenA.setAttackCurveX(x);
    } else {
        envGenB.setAttackCurveX(x);
    }
}

void WavetableSynthVoice::setAttCurveY(float y, int env_id)
{
    if (env_id == 0) {
        envGenA.setAttackCurveY(y);
    } else {
        envGenB.setAttackCurveY(y);
    }
}

void WavetableSynthVoice::setDecayCurveX(float x, int env_id)
{
    if (env_id == 0) {
        envGenA.setDecayCurveX(x);
    } else {
        envGenB.setDecayCurveX(x);
    }
}

void WavetableSynthVoice::setDecayCurveY(float y, int env_id)
{
    if (env_id == 0) {
        envGenA.setDecayCurveY(y);
    } else {
        envGenB.setDecayCurveY(y);
    }
}

void WavetableSynthVoice::setRelCurveX(float x, int env_id)
{
    if (env_id == 0) {
        envGenA.setReleaseCurveX(x);
    } else {
        envGenB.setReleaseCurveX(x);
    }
}

void WavetableSynthVoice::setRelCurveY(float y, int env_id)
{
    if (env_id == 0) {
        envGenA.setReleaseCurveY(y);
    } else {
        envGenB.setReleaseCurveY(y);
    }
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

void WavetableSynthVoice::setEnvelopeMonitor(EnvelopeStateCollector& collector, size_t index, int envelopeIndex)
{
    if (envelopeIndex == 0) {
        envelopeStateCollectorA = &collector;
    } else {
        envelopeStateCollectorB = &collector;
    }
    voiceIndex = index;
}

float WavetableSynthVoice::getLastWavetablePosition() {
    return lastWavetablePosition;
}

bool WavetableSynthVoice::canPlaySound(juce::SynthesiserSound* ptr)
{
    return true;
}

float WavetableSynthVoice::getWavetableIncrement(float frequency, double originalFrequency, size_t sampleSize, double sampleRate)
{
    return static_cast<float>(
        static_cast<double>(frequency) / originalFrequency * static_cast<double>(sampleSize) / sampleRate
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
        unisonIncrements[unisonVoice] = getWavetableIncrement(unisonFrequency, wavetable.originalSampleFrequency, wavetable.SampleSize, sampleRate);
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

        envGenA.prepare(sampleRate);
        envGenB.prepare(sampleRate);
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

        float envValueA = envGenA.getValue(gateState);
        float envValueB = envGenB.getValue(gateState);

        // Send envelope states to the GUI collector
        if (envelopeStateCollectorA != nullptr)
            envelopeStateCollectorA->setEnvelopeState(voiceIndex, envGenA.getCurrentState(), envGenA.getCurrentStateTimer());
        if (envelopeStateCollectorB != nullptr)
            envelopeStateCollectorB->setEnvelopeState(voiceIndex, envGenB.getCurrentState(), envGenB.getCurrentStateTimer());

        // Indices of which of the waveform in wavetable to use wavetables[integralindex]
        float integralIndexfloat = 0.f;
        lastWavetablePosition = wavetableIndex.getNext() * static_cast<float>(wavetable.wavetables.size() - 1);
        const float fractionalIndex = std::modf(lastWavetablePosition, &integralIndexfloat);
        const auto integralIndex = static_cast<size_t>(integralIndexfloat);

        for (auto unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
            unisonPhases[unisonVoice] = std::fmod( unisonPhases[unisonVoice] + unisonIncrements[unisonVoice], wavetable.SampleSize);
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
            const auto unisonLerped = naive_lerp(wavetable.wavetables[integralIndex % wavetable.wavetables.size()][unisonPhaseInteger], wavetable.wavetables[(integralIndex + 1) % wavetable.wavetables.size()][unisonPhaseInteger], fractionalIndex);
            const auto unisonOut { unisonLerped };

            wavetableOut += unisonOut;
        }
        wavetableOut /= unisonVoices;
        wavetableOut *= (wavetableVol * envValueA * velocity);

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

        if (voiceStarted && envGenA.isOff() && envGenB.isOff())
        {
            voiceStarted = false;
            clearCurrentNote();
        }
    }
}

}
