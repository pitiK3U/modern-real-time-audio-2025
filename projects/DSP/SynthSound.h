#pragma once

#include <JuceHeader.h>

namespace DSP
{
    
class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

}