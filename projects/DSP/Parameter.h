#pragma once

#include "LFO.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include <unordered_map>
#include <utility>

namespace DSP {

template <typename FloatType> class Parameter {
    public:
  Parameter<FloatType>() : smoothedValue() {}

  Parameter<FloatType>(FloatType defaultValue) : smoothedValue(defaultValue) {}

  void prepare(double SampleRate) { smoothedValue.reset(SampleRate); }

  void setValue(FloatType targetValue) {
    smoothedValue.setTargetValue(targetValue);
  }

  void setEffect(juce::String paramId, FloatType paramMult,
                 LFO &reference) {
    // effects.insert_or_assign(paramId, std::make_pair(paramMult, reference));
    // auto pair = std::make_pair(paramMult, reference);
    auto [iterator, inserted] = effects.try_emplace(paramId, paramMult, reference);
    if (!inserted) { iterator->second.first = paramMult; }

  }

  FloatType getCurrentValue() {
    auto rampedValue = smoothedValue.getCurrentValue();

    auto finalValue = rampedValue;
    for (auto [key, val] : effects) {
      auto [val_mult, assoc_val] = val;
      finalValue += finalValue * val_mult * assoc_val.getCurrentValue();
    }

    return finalValue;
  }

private:
  juce::SmoothedValue<FloatType> smoothedValue;
  double sampleRate;

  std::unordered_map<juce::String, std::pair<FloatType, LFO&>>
      effects;
};
}