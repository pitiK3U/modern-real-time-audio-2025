#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "DSP.h"
#include <functional>
#include <unordered_map>
#include <utility>

namespace DSP {

template <typename FloatType> class Parameter {
    public:
  Parameter<FloatType>() : smoothedValue() {}

  Parameter<FloatType>(FloatType defaultValue) : smoothedValue(defaultValue) {}

  void prepare(double SampleRate) { smoothedValue.reset(SampleRate); }

  void setValue(FloatType targetValue, bool force = false) {
    if (force) {
      smoothedValue.setCurrentAndTargetValue(targetValue);
    } else {
      smoothedValue.setTargetValue(targetValue);
    }
  }

  /**
  * \param paramId   Unique identifier of the parameter, to differentiate between different modifiers.
  * \param paramMult The multiplier for this parameter for the effect \p reference.
  *                  This value should be `parameterMax * influence`, where `influence` is in $[0,1]$.
  * \param reference The actual effect, which is used to get the value.
  */
  void setEffect(juce::String paramId, FloatType paramMult,
                 DSP<FloatType> &reference) {
    effects.insert_or_assign(paramId, std::make_pair(paramMult, std::ref(reference)));
  }

  FloatType getCurrentValue() {
    auto rampedValue = smoothedValue.getCurrentValue();

    auto finalValue = rampedValue;
    for (auto [key, val] : effects) {
      auto [val_mult, assoc_val] = val;
      finalValue += val_mult * assoc_val.get().getCurrentValue();
    }

    return finalValue;
  }

  FloatType getNext() {
    smoothedValue.getNextValue();

    return getCurrentValue();
  }

private:
  juce::SmoothedValue<FloatType> smoothedValue;
  double sampleRate { 48000.f };

  std::unordered_map<juce::String, std::pair<FloatType, std::reference_wrapper<DSP<FloatType>>>>
      effects;
};
}