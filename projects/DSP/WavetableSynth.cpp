#include "Oscillator.h"
#include "juce_core/juce_core.h"
#include <algorithm>
#include <cmath>
#include "WavetableSynth.h"

namespace DSP
{

float naive_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

float convertMidiNoteToFreq(int MidiNote)
{
    return 440.f * std::pow(2.f, static_cast<float>(MidiNote - 69) / 12.f);
}

WavetableSynthVoice::WavetableSynthVoice()
{
    fillWavetable();

    vcaEnvGen.setAnalogStyle(false);
    vcfEnvGen.setAnalogStyle(false);
}

void WavetableSynthVoice::fillWavetable(double SampleRate)
{
    // FIXME: correctly fill the buffers and check phaseInc and phase
    const int wavetableCount = 4;
    wavetables.resize(wavetableCount);

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

WavetableSynthVoice::~WavetableSynthVoice()
{
}

/* REMOVE: 
void WavetableSynthVoice::setOscSawVol(float dB, bool skipRamp)
{
    sawOscVolRamp.setTarget(std::pow(10.f, 0.05f * dB), skipRamp);
}

void WavetableSynthVoice::setOscTriVol(float dB, bool skipRamp)
{
    triOscVolRamp.setTarget(std::pow(10.f, 0.05f * dB), skipRamp);
}

void WavetableSynthVoice::setOscSinVol(float dB, bool skipRamp)
{
    sinOscVolRamp.setTarget(std::pow(10.f, 0.05f * dB), skipRamp);
}
*/

void WavetableSynthVoice::setWavetablePosition(float index, bool skip) {
    wavetableIndex.setValue(std::clamp(index, 0.f, static_cast<float>(wavetables.size() - 1)), skip);
}

void WavetableSynthVoice::setWavetableVol(float dB, bool skipRamp)
{
    wavetableVolRamp.setValue(std::pow(10.f, 0.05f * dB), skipRamp);
}

void WavetableSynthVoice::setAttTimeVCA(float ms)
{
    vcaEnvGen.setAttackTime(ms);
}

void WavetableSynthVoice::setDecayTimeVCA(float ms)
{
    vcaEnvGen.setDecayTime(ms);
}

void WavetableSynthVoice::setSustainVCA(float norm)
{
    vcaEnvGen.setSustainLevel(std::clamp(norm, 0.f, 1.f));
}

void WavetableSynthVoice::setRelTimeVCA(float ms)
{
    vcaEnvGen.setReleaseTime(ms);
}

void WavetableSynthVoice::setAttTimeVCF(float ms)
{
    vcfEnvGen.setAttackTime(ms);
}

void WavetableSynthVoice::setDecayTimeVCF(float ms)
{
    vcfEnvGen.setDecayTime(ms);
}

void WavetableSynthVoice::setSustainVCF(float norm)
{
    vcfEnvGen.setSustainLevel(std::clamp(norm, 0.f, 1.f));
}

void WavetableSynthVoice::setRelTimeVCF(float ms)
{
    vcfEnvGen.setReleaseTime(ms);
}

void WavetableSynthVoice::setLFOFreqVCF(float Hz)
{
    lfoFreq = std::fmax(Hz, 0.f);
    lfoPhaseInc = static_cast<float>(2.0 * M_PI / sampleRate) * lfoFreq;
}

void WavetableSynthVoice::setLFOTypeVCF(LFOType type)
{
    lfoType = type;
}

void WavetableSynthVoice::setEnvAmountVCF(float bipolar, bool skipRamp)
{
    vcfEnvAmountRamp.setTarget(std::clamp(bipolar, -1.f, 1.f), skipRamp);
}

void WavetableSynthVoice::setLFOAmountVCF(float bipolar, bool skipRamp)
{
    vcfLFOAmountRamp.setTarget(std::clamp(bipolar, -1.f, 1.f), skipRamp);
}

void WavetableSynthVoice::setFilterCutoff(float Hz, bool skipRamp)
{
    vcfFreqRamp.setTarget(std::clamp(Hz, MinFreqHz, MaxFreqHz), skipRamp);
}

void WavetableSynthVoice::setFilterReso(float Q, bool skipRamp)
{
    vcfResoRamp.setTarget(std::clamp(Q, MinReso, MaxReso), skipRamp);
}

void WavetableSynthVoice::setFilterType(FilterType type, bool skipRamp)
{
    vcfLPFRamp.setTarget(type == LPF ? 1.f : 0.f, skipRamp);
    vcfBPFRamp.setTarget(type == BPF ? 1.f : 0.f, skipRamp);
    vcfHPFRamp.setTarget(type == HPF ? 1.f : 0.f, skipRamp);
}

void WavetableSynthVoice::setOutputVol(float dB, bool skipRamp)
{
    outputVolRamp.setTarget(std::pow(10.f, 0.05f * dB), skipRamp);
}


bool WavetableSynthVoice::canPlaySound(juce::SynthesiserSound* ptr)
{
    return true;
}

void WavetableSynthVoice::startNote(int midiNoteNumber, float newVelocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    // sinOsc.setFrequency(convertMidiNoteToFreq(midiNoteNumber));
    // triOsc.setFrequency(convertMidiNoteToFreq(midiNoteNumber));
    // sawOsc.setFrequency(convertMidiNoteToFreq(midiNoteNumber));

    const auto freq = convertMidiNoteToFreq(midiNoteNumber);
    wavetableInc = static_cast<float>(
        static_cast<double>(freq) / static_cast<double>(DefaultFreq) * static_cast<double>(SampleSize) / sampleRate
    );

    vcaEnvGen.start();
    vcfEnvGen.start();

    velocity = newVelocity;
    voiceStarted = true;
}

void WavetableSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    vcaEnvGen.end();
    vcfEnvGen.end();

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

        fillWavetable(sampleRate);

        vcaEnvGen.prepare(sampleRate);
        vcfEnvGen.prepare(sampleRate);
        filter.prepare(sampleRate);
        // sinOscVolRamp.prepare(sampleRate);
        // triOscVolRamp.prepare(sampleRate);
        // sawOscVolRamp.prepare(sampleRate);
        wavetableVolRamp.prepare(sampleRate);
        outputVolRamp.prepare(sampleRate);
        vcfEnvAmountRamp.prepare(sampleRate);
        vcfLFOAmountRamp.prepare(sampleRate);
        vcfFreqRamp.prepare(sampleRate);
        vcfResoRamp.prepare(sampleRate);
        vcfLPFRamp.prepare(sampleRate);
        vcfBPFRamp.prepare(sampleRate);
        vcfHPFRamp.prepare(sampleRate);

        lfoPhaseInc = static_cast<float>(2.0 * M_PI / sampleRate) * std::fmax(lfoFreq, 0.f);
    }

    for (int i = 0; i < numSamples; ++i)
    {
        // const auto sin { wavetables[] };
        // const auto tri { triOsc.process() };
        // const auto saw { sawOsc.process() };

        float vcaEnv { 0.f };
        vcaEnvGen.process(&vcaEnv, 1);

        float vcfEnv { 0.f };
        vcfEnvGen.process(&vcfEnv, 1);

        // const auto sinVol { sinOscVolRamp.getNext() };
        // const auto triVol { triOscVolRamp.getNext() };
        // const auto sawVol { sawOscVolRamp.getNext() };

        // Indices of which of the waveform in wavetable to use wavetables[integralindex]
        float integralIndexfloat = 0.f;
        const float fractionalIndex = std::modf(wavetableIndex.getNext(), &integralIndexfloat);
        const auto integralIndex = static_cast<size_t>(integralIndexfloat);

        wavetablePhase = std::fmod( wavetablePhase + wavetableInc, SampleSize);

        const auto wavetableVol { wavetableVolRamp.getNext() };

        const auto vcfEnvAmout { vcfEnvAmountRamp.getNext() };
        const auto vcfLFOAmount { vcfLFOAmountRamp.getNext() };

        const auto vcfFreq { vcfFreqRamp.getNext() };
        const auto vcfReso { vcfResoRamp.getNext() };
        const auto vcfLPF { vcfLPFRamp.getNext() };
        const auto vcfBPF { vcfBPFRamp.getNext() };
        const auto vcfHPF { vcfHPFRamp.getNext() };

        const auto outputVol { outputVolRamp.getNext() };

        // Process LFO acording to mod type
        float lfo { 0.f };
        switch (lfoType)
        {
        case TRI:
            lfo = std::fabs((lfoPhaseState - static_cast<float>(M_PI)) / static_cast<float>(M_PI));
            break;

        case SIN:
            lfo = 0.5f + 0.5f * std::sin(lfoPhaseState);
            break;
        }
        lfoPhaseState = std::fmod(lfoPhaseState + lfoPhaseInc, static_cast<float>(2 * M_PI));

        const auto wavetablePhaseInteger = static_cast<size_t>(wavetablePhase);
        const auto wavetableLerped = naive_lerp(wavetables[integralIndex][wavetablePhaseInteger], wavetables[(integralIndex + 1) % wavetables.size()][wavetablePhaseInteger], fractionalIndex);
        const auto wavetableOut { wavetableLerped * wavetableVol * vcaEnv * velocity };

        const auto freqMod { std::clamp(vcfEnv * vcfEnvAmout + vcfLFOAmount * lfo, -1.f, 1.f) };
        const auto freq { std::clamp(FreqModRange * (std::pow(2.f, freqMod) - 1.f) + vcfFreq, MinFreqHz, MaxFreqHz) };

        float lpfOut { 0.f };
        float bpfOut { 0.f };
        float hpfOut { 0.f };
        filter.process(&lpfOut, &bpfOut, &hpfOut, &wavetableOut, &freq, &vcfReso, 1);

        const auto out { (vcfLPF * lpfOut + vcfBPF * bpfOut + vcfHPF * hpfOut) * outputVol };
        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        {
            outputBuffer.addSample(ch, startSample + i, out);
        }

        if (voiceStarted && vcaEnvGen.isOff() && vcfEnvGen.isOff())
        {
            voiceStarted = false;
            clearCurrentNote();
        }
    }
}

}
