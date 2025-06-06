#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "DSP.h"
#include <functional>
#include <tuple>
#include <unordered_map>

namespace DSP {

template <typename FloatType> class Parameter {
public:

  using EffectEvaluator = std::function<FloatType(FloatType previousValue, FloatType originalValue, FloatType dspMultiplier, DSP<FloatType>& dsp)>;

  Parameter<FloatType>() : smoothedValue() {}

  Parameter<FloatType>(FloatType defaultValue) : originalValue(defaultValue) {}

  void prepare(double SampleRate) { originalValue.reset(SampleRate, DefaultRampTimeInSecs); smoothedValue.reset(SampleRate, DefaultRampTimeInSecs); }

  void setValue(FloatType targetValue, bool force = false) {
    if (force) {
      originalValue.setCurrentAndTargetValue(targetValue);
    } else {
      originalValue.setTargetValue(targetValue);
    }
  }

  /**
  * \param paramId   Unique identifier of the parameter, to differentiate between different modifiers.
  * \param paramMult The multiplier for this parameter for the effect \p reference.
  *                  This value should be `parameterMax * influence`, where `influence` is in $[0,1]$ for the `defaultEffect`.
  * \param reference The actual effect, which is used to get the value.
  * \param effectEvalutor Function (or lambda) that takes previous value, original value, \p paramMult
  *                       and dsp \p reference and calculates the result parameter value after the effect.
  */
  void setEffect(juce::String paramId, FloatType paramMult,
                 DSP<FloatType> &reference,
                 EffectEvaluator effectEvaluator = defaultEffect
                ) {
    for (auto & element : effects) {
      auto key = std::get<0>(element);
      if (paramId.compare(key) == 0) {
        element = std::make_tuple(paramId, paramMult, std::ref(reference), effectEvaluator);
        return;
      }
    }

    effects.emplace_back(paramId, paramMult, reference, effectEvaluator);
  }

  FloatType getCurrentValue() {
    auto rampedValue = originalValue.getCurrentValue();

    auto finalValue = rampedValue;
    for (auto [_key, valueMultiplier, dspEffector, effect] : effects) {
      finalValue = effect(finalValue, rampedValue, valueMultiplier, dspEffector);
    }

    smoothedValue.setTargetValue(finalValue);
    return smoothedValue.getCurrentValue();

    // return finalValue;
  }

  /**
   * NOTE: You must first progress the `effect`s on their own before using `getNext()` on the final value.
   */
  FloatType getNext() {
    originalValue.getNextValue();
    smoothedValue.getNextValue();

    return getCurrentValue();
  }

  // TODO: should add clamp
  static constexpr FloatType defaultEffect(FloatType previousValue, FloatType originalValue, FloatType valueMultiplier, DSP<FloatType> & dsp) {
    return previousValue + valueMultiplier * dsp.getCurrentValue();
  };

  // Use vector, since hash map used more than 40% cpu on 2 parameters
  std::vector<std::tuple<juce::String, FloatType, std::reference_wrapper<DSP<FloatType>>, EffectEvaluator>>
      effects;

private:
  juce::SmoothedValue<FloatType> originalValue;
  juce::SmoothedValue<FloatType> smoothedValue;

  double sampleRate { 48000.f };
  static constexpr auto DefaultRampTimeInSecs = 0.05f;
};

}