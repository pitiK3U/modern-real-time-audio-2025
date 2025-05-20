#pragma once

#include <JuceHeader.h>

#include "EnvelopeGenerator.h"
#include "Oscillator.h"
#include "Ramp.h"

namespace DSP
{

float convertMidiNoteToFreq(int MidiNote);

class SynthVoiceEnvelope : public juce::SynthesiserVoice
{
public:
    static constexpr float DefaultGainTime = 0.005f;

    SynthVoiceEnvelope();
    ~SynthVoiceEnvelope();

    SynthVoiceEnvelope(const SynthVoiceEnvelope&) = delete;
    SynthVoiceEnvelope(SynthVoiceEnvelope&&) = delete;
    const SynthVoiceEnvelope& operator=(const SynthVoiceEnvelope&) = delete;
    const SynthVoiceEnvelope& operator=(SynthVoiceEnvelope&&) = delete;

    void setWaveType(Oscillator::OscType type);

    void setRampTime(float rampTime);

    void setAnalogStyle(bool isAnalogStyle);
    void setAttackTime(float attackTimeMs);
    void setDecayTime(float decayTimeMs);
    void setSustainLevel(float sustainLevelLinear);
    void setReleaseTime(float releaseTimeMs);

    bool canPlaySound(SynthesiserSound* ptr) override;
    void startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    double sampleRate { 48000.0 };

    Ramp<float> gain = DefaultGainTime;
    Oscillator osc;
    EnvelopeGenerator envelope;
};

}
