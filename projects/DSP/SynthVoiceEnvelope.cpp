#include "SynthVoiceEnvelope.h"
#include "SynthSound.h"
#include <vector>

namespace DSP
{

float convertMidiNoteToFreq(int MidiNote)
{
    return 440.f * std::pow(2.f, static_cast<float>(MidiNote - 69) / 12.f);
}

SynthVoiceEnvelope::SynthVoiceEnvelope()
{
}

SynthVoiceEnvelope::~SynthVoiceEnvelope()
{
}

void SynthVoiceEnvelope::setWaveType(Oscillator::OscType type)
{
    osc.setType(type);
}

void SynthVoiceEnvelope::setRampTime(float rampTime) {
    gain.setRampTime(rampTime);
}

void SynthVoiceEnvelope::setAnalogStyle(bool isAnalogStyle) {
    envelope.setAnalogStyle(isAnalogStyle);
}

void SynthVoiceEnvelope::setAttackTime(float attackTimeMs) {
    envelope.setAttackTime(attackTimeMs);
}

void SynthVoiceEnvelope::setDecayTime(float decayTimeMs) {
    envelope.setDecayTime(decayTimeMs);
}

void SynthVoiceEnvelope::setSustainLevel(float sustainLevelLinear) {
    envelope.setSustainLevel(sustainLevelLinear);
}

void SynthVoiceEnvelope::setReleaseTime(float releaseTimeMs) {
    envelope.setReleaseTime(releaseTimeMs);
}


// just check if this is "ours" synth sound
bool SynthVoiceEnvelope::canPlaySound(SynthesiserSound* ptr)
{
    if (dynamic_cast<SynthSound*>(ptr))
        return true;
    return false;
}

void SynthVoiceEnvelope::startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int)
{
    gain.setTarget(velocity);
    envelope.start();
    osc.setFrequency(convertMidiNoteToFreq(midiNoteNumber));
}

void SynthVoiceEnvelope::stopNote(float velocity, bool allowTailOff)
{
    // gain.setTarget(0.f);
    envelope.end();
}

void SynthVoiceEnvelope::pitchWheelMoved(int)
{
}

void SynthVoiceEnvelope::controllerMoved(int, int)
{
}

void SynthVoiceEnvelope::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // there's no "prepare" call to synth voice, so we need to manually check
    // every buffer call if the sample rate has changed
    double newSampleRate { getSampleRate() };
    if (newSampleRate != sampleRate)
    {
        sampleRate = newSampleRate;
        osc.prepare(sampleRate);
        envelope.prepare(sampleRate);
        gain.prepare(sampleRate);
    }

    auto* outputPtr = outputBuffer.getWritePointer(0, startSample);

    // process the oscillator
    osc.process(outputPtr, static_cast<unsigned int>(numSamples));

    std::vector<float> tmp(static_cast<unsigned int>(numSamples));
    float * data = tmp.data();
    // apply simple ramp as VCA
    envelope.process(data, static_cast<unsigned int>(numSamples));
    gain.applyGain(&data, 1u, static_cast<unsigned int>(numSamples));

    for (auto i = 0; i < numSamples; i++) {
        outputPtr[i] *= tmp[i];
    }

    // if the output is stereo, just copy contents from left to right channel
    if (outputBuffer.getNumChannels() > 1)
        outputBuffer.copyFrom(1, startSample, outputBuffer, 0, startSample, numSamples);
}

}
