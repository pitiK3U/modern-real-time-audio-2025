#pragma once

namespace DSP
{
    enum class Waveform {
        Sine,
        Square,
        Triangle,
        Sawtooth
    };
    
    class LFO {
        public:
            LFO(float sampleRate, float frequency, Waveform waveform);
            ~LFO() = default;

            void prepare(float sampleRate, float frequency, Waveform waveform);
            
            // ———— SAMPLE-LEVEL API ————

            /** 
            * Read the current LFO value, optionally advancing the phase.
            * @param advancePhase  if true, calls advancePhase() after reading
            * @returns the waveform at the current phase
            */
            float getValue(bool advancePhase = false);

            // Advance the phase by one sample
            void advancePhase();

            //---- Disable default ctor, copy and move semantics -----

            // No default ctor
            LFO() = delete;

            // No copy semantics
            LFO(const LFO&) = delete;
            const LFO& operator=(const LFO&) = delete;

            // No move semantics
            LFO(LFO&&) = delete;
            const LFO& operator=(LFO&&) = delete;
            //-------

        private:
            float sampleRate;       // Hz
            float frequency;        // Hz
            float phase{ 0.0f };    // [0…1)
            float phaseIncrement{ 0.0f };
            float amplitude{ 1.0f }; 
            float offset{ 0.0f };  
            Waveform waveform;

            void updatePhaseIncrement();
            float computeWaveform(float p) const;
    };
}