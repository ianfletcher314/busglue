#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"

class EnvelopeFollower
{
public:
    enum class DetectionMode
    {
        Peak,
        RMS
    };

    EnvelopeFollower();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Process a single sample and return envelope value
    float processSample(float input);

    // Process stereo samples with link amount
    void processStereo(float inputL, float inputR, float linkAmount,
                       float& envL, float& envR);

    // Setters
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    void setDetectionMode(DetectionMode mode);
    void setAutoRelease(bool enabled);

    // For auto-release: inform the follower of the current gain reduction
    void setCurrentGainReduction(float grDb);

private:
    void updateCoefficients();

    // Parameters
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    DetectionMode detectionMode = DetectionMode::RMS;
    bool autoReleaseEnabled = false;

    // Derived coefficients
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Auto-release coefficients (fast and slow)
    float releaseFastCoeff = 0.0f;
    float releaseSlowCoeff = 0.0f;

    // RMS smoothing coefficient (precomputed for efficiency)
    float rmsCoeff = 0.0f;

    // State
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;

    // RMS smoothing state
    float rmsSquaredL = 0.0f;
    float rmsSquaredR = 0.0f;

    // Auto-release state
    float currentGainReduction = 0.0f;

    // Runtime
    double currentSampleRate = 44100.0;
};
