#pragma once

#include <JuceHeader.h>
#include <vector>
#include <atomic>

#include "ADSREnvelopeGenerator.h"

namespace DSP
{

// Copyable snapshot of one voice
struct EnvelopeSnapshot
{
    EnvelopeState state;
    float timerMs;
};

// Internal non-copyable storage struct
struct EnvelopeInfo
{
    std::atomic<EnvelopeState> state { OFF };
    std::atomic<float> stateTimerMs { 0.0f };
};

class EnvelopeStateCollector
{
public:
    explicit EnvelopeStateCollector(size_t numVoices)
        : envelopeStates(numVoices)
    {
        for (auto& info : envelopeStates)
        {
            info.state.store(OFF, std::memory_order_relaxed);
            info.stateTimerMs.store(0.0f, std::memory_order_relaxed);
        }
    }

    void setEnvelopeState(size_t voiceIndex, EnvelopeState state, float timerMs)
    {
        if (voiceIndex < envelopeStates.size())
        {
            envelopeStates[voiceIndex].state.store(state, std::memory_order_relaxed);
            envelopeStates[voiceIndex].stateTimerMs.store(timerMs, std::memory_order_relaxed);
        }
    }

    std::vector<EnvelopeSnapshot> getEnvelopeStateSnapshot() const
    {
        std::vector<EnvelopeSnapshot> snapshot;
        snapshot.reserve(envelopeStates.size());

        for (const auto& info : envelopeStates)
        {
            snapshot.push_back({
                info.state.load(std::memory_order_relaxed),
                info.stateTimerMs.load(std::memory_order_relaxed)
            });
        }

        return snapshot;
    }

    size_t getNumVoices() const { return envelopeStates.size(); }

private:
    std::vector<EnvelopeInfo> envelopeStates;
};

}
