#pragma once

namespace DSP {
template <typename FloatType> class DSP {
public:
  virtual ~DSP() {}
  virtual FloatType getCurrentValue() = 0;
  virtual void prepare(double SampleRate) = 0;
};
}