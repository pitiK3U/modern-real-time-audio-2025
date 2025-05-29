#pragma once

#include <utility>

namespace DSP
{
    enum EnvelopeState
    {
        OFF,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    class ADSREnvelopeGenerator
    {
        public:
            ADSREnvelopeGenerator(float sampleRate);
            ~ADSREnvelopeGenerator() = default;

            ADSREnvelopeGenerator(const ADSREnvelopeGenerator&) = delete;
            ADSREnvelopeGenerator(ADSREnvelopeGenerator&&) = delete;
            const ADSREnvelopeGenerator& operator=(const ADSREnvelopeGenerator&) = delete;
            const ADSREnvelopeGenerator& operator=(ADSREnvelopeGenerator&&) = delete;

            void prepare(double newSampleRate);
            
            // ----- SAMPLE-LEVEL API -----

            /**
            * Advance the envelope state by one sample.
            * Returns the current envelope value.
            * @param midiGateState  true if the gate is on, false if off
            */
            float getValue(bool midiGateState);

            EnvelopeState getCurrentState() const;
            float getCurrentStateTimer() const;
            
            bool isOff();

            void setAttackTime(float attackTimeMs);
            void setDecayTime(float decayTimeMs);
            void setSustainLevel(float sustainLevelLinear);
            void setReleaseTime(float releaseTimeMs);

            void setAttackCurveX(float x);
            void setAttackCurveY(float y);
            void setDecayCurveX(float x);
            void setDecayCurveY(float y);
            void setReleaseCurveX(float x);
            void setReleaseCurveY(float y);

        private:
            void updateEnvelopeState(bool midiGateState);

            float getOffCurveValue();
            float getAttackCurveValue();
            float getDecayCurveValue();
            float getSustainCurveValue();
            float getReleaseCurveValue();

            /**
            * Solve a single quadratic Bézier segment B(t) = (x(t), y(t)) for a given xQuery.
            * Returns the corresponding y, clamped to [y0, y1] if xQuery is outside [x0,x1].
            */
            static float solveQuadraticBezierAtX(double xQuery, double x0, double y0, double x1, double y1, double cx, double cy);

            float sampleRate; // Hz

            // ADSR parameters
            float attackTimeMs; // ms
            float decayTimeMs; // ms
            float sustainLevel; // 0.0 to 1.0
            float releaseTimeMs; // ms

            // Curvature coefficients
            std::pair<float, float> attackCurveCoefficients;
            std::pair<float, float> decayCurveCoefficients;
            std::pair<float, float> releaseCurveCoefficients;

            EnvelopeState currentState { OFF };
            float currentEnvelopeValue { 0.f }; // current envelope value
            float currentStateTimer { 0.f }; // in ms
            float currentStateStartEnvelopeValue { 0.f }; // value at the start of the current state (used for scaling)
    };
}