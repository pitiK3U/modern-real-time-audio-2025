#pragma  once

#include "DSP.h"
#include "Oscillator.h"

namespace DSP {
    class LFO : public DSP<float> {
public:
    // Update new sample rate
    void prepare(double sampleRate) override {
        osc.prepare(sampleRate);
    };

     // Set a new frequency for the oscillator in Hz
     void setFrequency(float freqHz) {
        osc.setFrequency(freqHz);
     };

     // Select the waveform type
     void setType(Oscillator::OscType type) {
        osc.setType(type);
     };
    
    float getCurrentValue() override {
        return osc.getCurrentValue();
    }

    void advance() {
        osc.process();
    }

    private:
        Oscillator osc;
    };
}