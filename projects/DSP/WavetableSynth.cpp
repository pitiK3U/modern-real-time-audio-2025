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
    for (int ch = 0; ch < Channels; ch++) {
        unisonPhases[ch].resize(unisonVoices);
        unisonIncrements[ch].resize(unisonVoices);
    }
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

void WavetableSynthVoice::setPanning(float value, bool skipRamp)
{
    panning.setValue(std::clamp(value, -1.f, 1.f), skipRamp);
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
    for (auto ch = 0; ch < Channels; ch++) {
        for (auto unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
            float unisonMultiplier = unisonVoice * unisonStrength;
            unisonMultiplier = std::copysign(unisonMultiplier, ch == 0 ? -1.f : 1.f);
            const auto unisonFrequency = moveFrequencyByCents(frequency, unisonMultiplier * unisonDetune.getCurrentValue());
            unisonIncrements[ch][unisonVoice] = getWavetableIncrement(unisonFrequency, wavetable.originalSampleFrequency, wavetable.SampleSize, sampleRate);
        }
    }
}

void WavetableSynthVoice::startNote(int midiNoteNumber, float newVelocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    frequency = convertMidiNoteToFreq(midiNoteNumber);

    updateUnisonIncrements();

    for (auto unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
        unisonPhases[0][unisonVoice] = juce::Random::getSystemRandom().nextFloat() * wavetable.SampleSize;
        for (auto ch = 1; ch < Channels; ch++) {
            // Initialize with random phase to avoid phase alignment
            unisonPhases[ch][unisonVoice] = unisonPhases[0][unisonVoice];
        }
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
        filter[0].prepare(sampleRate);
        filter[1].prepare(sampleRate);
        wavetableVolRamp.prepare(sampleRate);
        outputVolRamp.prepare(sampleRate);
        unisonDetune.prepare(sampleRate);
        panning.prepare(sampleRate);
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
        lastWavetablePosition = wavetableIndex.getNext() * static_cast<float>(wavetable.wavetables.size());
        const float fractionalIndex = std::modf(lastWavetablePosition, &integralIndexfloat);
        const auto integralIndex = static_cast<size_t>(integralIndexfloat);

        for (auto ch = 0; ch < Channels; ch++) {
            for (auto unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
                unisonPhases[ch][unisonVoice] = std::fmod( unisonPhases[ch][unisonVoice] + unisonIncrements[ch][unisonVoice], wavetable.SampleSize);
            }
        }

        const auto wavetableVol { wavetableVolRamp.getNext() };

        const auto vcfFreqCurrent { vcfFreq.getNext() };
        const auto vcfResoCurrent { vcfReso.getNext() };
        const auto vcfLPF { vcfLPFRamp.getNext() };
        const auto vcfBPF { vcfBPFRamp.getNext() };
        const auto vcfHPF { vcfHPFRamp.getNext() };

        const auto pan { panning.getNext() };

        const auto outputVol { outputVolRamp.getNext() };
    
        for (int ch = 0; ch < Channels /* outputBuffer.getNumChannels() */ ; ++ch)
        {

        float wavetableOut = 0.f;
        for (int unisonVoice = 0; unisonVoice < unisonVoices; unisonVoice++) {
            const auto unisonPhaseInteger = static_cast<size_t>(unisonPhases[ch][unisonVoice]);
            const auto unisonLerped = naive_lerp(wavetable.wavetables[integralIndex % wavetable.wavetables.size()][unisonPhaseInteger], wavetable.wavetables[(integralIndex + 1) % wavetable.wavetables.size()][unisonPhaseInteger], fractionalIndex);
            const auto unisonOut { unisonLerped };

            wavetableOut += unisonOut;
        }
        wavetableOut /= unisonVoices;
        wavetableOut *= (wavetableVol * envValueA * velocity);

        float lpfOut { 0.f };
        float bpfOut { 0.f };
        float hpfOut { 0.f };
        filter[ch].process(&lpfOut, &bpfOut, &hpfOut, &wavetableOut, &vcfFreqCurrent, &vcfResoCurrent, 1);

        const auto out { (vcfLPF * lpfOut + vcfBPF * bpfOut + vcfHPF * hpfOut) * outputVol };

            float channelMultiplier = 1.f;

            if (ch == 1) {
                channelMultiplier = 1.f - std::fabs(std::clamp(pan, -1.f, 0.f));
            } else if (ch == 0) {
                channelMultiplier = 1.f - std::clamp(pan, 0.f, 1.f);
            }

            outputBuffer.addSample(ch, startSample + i, channelMultiplier * out);
            
            if (std::isnan(out))
            {
            voiceStarted = false;
            clearCurrentNote();
            DBG("WavetableSynthVoice: NaN detected in output buffer");
            return;
        }
    }

        if (voiceStarted && envGenA.isOff() && envGenB.isOff())
        {
            voiceStarted = false;
            clearCurrentNote();
        }
    }
}

}
