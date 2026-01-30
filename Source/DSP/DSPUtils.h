#pragma once

#include <cmath>
#include <algorithm>

namespace DSPUtils
{
    // Level conversion
    inline float linearToDecibels(float linear)
    {
        return linear > 0.0f ? 20.0f * std::log10(linear) : -100.0f;
    }

    inline float decibelsToLinear(float dB)
    {
        return std::pow(10.0f, dB / 20.0f);
    }

    // Range mapping
    inline float mapRange(float value, float inMin, float inMax, float outMin, float outMax)
    {
        return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
    }

    // Clipping functions
    inline float softClip(float sample)
    {
        return std::tanh(sample);
    }

    inline float hardClip(float sample, float threshold = 1.0f)
    {
        return std::clamp(sample, -threshold, threshold);
    }

    // One-pole filter coefficient
    inline float calculateCoefficient(double sampleRate, float timeMs)
    {
        if (timeMs <= 0.0f) return 1.0f;
        return 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * timeMs * 0.001f));
    }

    // Biquad coefficient structure
    struct BiquadCoeffs
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    // High-pass filter coefficients (for sidechain filtering)
    inline BiquadCoeffs calcHighPass(double sampleRate, float freq, float q = 0.707f)
    {
        BiquadCoeffs c;
        float w0 = 2.0f * 3.14159265358979323846f * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f + cosw0) / 2.0f) / a0;
        c.b1 = (-(1.0f + cosw0)) / a0;
        c.b2 = ((1.0f + cosw0) / 2.0f) / a0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha) / a0;

        return c;
    }

    // Low-pass filter coefficients
    inline BiquadCoeffs calcLowPass(double sampleRate, float freq, float q = 0.707f)
    {
        BiquadCoeffs c;
        float w0 = 2.0f * 3.14159265358979323846f * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f - cosw0) / 2.0f) / a0;
        c.b1 = (1.0f - cosw0) / a0;
        c.b2 = ((1.0f - cosw0) / 2.0f) / a0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha) / a0;

        return c;
    }

    // Soft knee computation
    inline float computeSoftKnee(float inputDb, float thresholdDb, float ratio, float kneeWidth)
    {
        if (kneeWidth <= 0.0f)
        {
            // Hard knee
            if (inputDb <= thresholdDb)
                return inputDb;
            return thresholdDb + (inputDb - thresholdDb) / ratio;
        }

        // Soft knee
        float halfKnee = kneeWidth / 2.0f;
        float kneeStart = thresholdDb - halfKnee;
        float kneeEnd = thresholdDb + halfKnee;

        if (inputDb <= kneeStart)
            return inputDb;
        if (inputDb >= kneeEnd)
            return thresholdDb + (inputDb - thresholdDb) / ratio;

        // In the knee region - smooth transition
        float kneeProgress = (inputDb - kneeStart) / kneeWidth;
        float effectiveRatio = 1.0f + (ratio - 1.0f) * kneeProgress;
        float kneeGain = inputDb - kneeStart;
        return kneeStart + kneeGain / effectiveRatio;
    }

    // Analog-style saturation curves
    inline float warmSaturate(float sample, float drive)
    {
        float x = sample * (1.0f + drive);
        return std::tanh(x * 0.9f) / 0.9f;
    }

    inline float punchSaturate(float sample, float drive)
    {
        // Asymmetric soft clipping for punch
        float x = sample * (1.0f + drive * 0.5f);
        if (x > 0.0f)
            return std::tanh(x * 1.2f) / 1.1f;
        else
            return std::tanh(x * 0.8f) / 0.9f;
    }

    inline float aggressiveSaturate(float sample, float drive)
    {
        // Harder clipping with harmonics
        float x = sample * (1.0f + drive);
        float y = std::tanh(x * 1.5f);
        // Add some 3rd harmonic
        return y + 0.1f * drive * (y * y * y);
    }
}
