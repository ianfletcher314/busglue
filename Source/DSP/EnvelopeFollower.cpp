#include "EnvelopeFollower.h"

EnvelopeFollower::EnvelopeFollower()
{
}

void EnvelopeFollower::prepare(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    currentSampleRate = sampleRate;
    updateCoefficients();
    reset();
}

void EnvelopeFollower::reset()
{
    envelopeL = 0.0f;
    envelopeR = 0.0f;
    rmsSquaredL = 0.0f;
    rmsSquaredR = 0.0f;
    currentGainReduction = 0.0f;
}

void EnvelopeFollower::updateCoefficients()
{
    attackCoeff = DSPUtils::calculateCoefficient(currentSampleRate, attackMs);
    releaseCoeff = DSPUtils::calculateCoefficient(currentSampleRate, releaseMs);

    // Auto-release: fast = 50ms, slow = 2000ms (program dependent)
    releaseFastCoeff = DSPUtils::calculateCoefficient(currentSampleRate, 50.0f);
    releaseSlowCoeff = DSPUtils::calculateCoefficient(currentSampleRate, 2000.0f);

    // RMS smoothing coefficient (10ms window)
    rmsCoeff = DSPUtils::calculateCoefficient(currentSampleRate, 10.0f);
}

void EnvelopeFollower::setAttack(float ms)
{
    attackMs = ms;
    attackCoeff = DSPUtils::calculateCoefficient(currentSampleRate, attackMs);
}

void EnvelopeFollower::setRelease(float ms)
{
    releaseMs = ms;
    releaseCoeff = DSPUtils::calculateCoefficient(currentSampleRate, releaseMs);
}

void EnvelopeFollower::setDetectionMode(DetectionMode mode)
{
    detectionMode = mode;
}

void EnvelopeFollower::setAutoRelease(bool enabled)
{
    autoReleaseEnabled = enabled;
}

void EnvelopeFollower::setCurrentGainReduction(float grDb)
{
    currentGainReduction = grDb;
}

float EnvelopeFollower::processSample(float input)
{
    float rectified;

    if (detectionMode == DetectionMode::Peak)
    {
        rectified = std::abs(input);
    }
    else // RMS
    {
        // Smooth the squared value, then take sqrt
        float squared = input * input;
        rmsSquaredL += rmsCoeff * (squared - rmsSquaredL);
        rectified = std::sqrt(rmsSquaredL);
    }

    // Determine release coefficient (possibly auto)
    float effectiveReleaseCoeff = releaseCoeff;

    if (autoReleaseEnabled)
    {
        // Program-dependent release: more GR = faster release
        // Blend between slow and fast based on gain reduction amount
        float grFactor = std::min(1.0f, currentGainReduction / 12.0f);
        effectiveReleaseCoeff = releaseSlowCoeff + grFactor * (releaseFastCoeff - releaseSlowCoeff);
    }

    // Attack/release envelope
    if (rectified > envelopeL)
    {
        envelopeL += attackCoeff * (rectified - envelopeL);
    }
    else
    {
        envelopeL += effectiveReleaseCoeff * (rectified - envelopeL);
    }

    return envelopeL;
}

void EnvelopeFollower::processStereo(float inputL, float inputR, float linkAmount,
                                      float& envL, float& envR)
{
    float rectL, rectR;

    if (detectionMode == DetectionMode::Peak)
    {
        rectL = std::abs(inputL);
        rectR = std::abs(inputR);
    }
    else // RMS
    {
        float squaredL = inputL * inputL;
        float squaredR = inputR * inputR;
        rmsSquaredL += rmsCoeff * (squaredL - rmsSquaredL);
        rmsSquaredR += rmsCoeff * (squaredR - rmsSquaredR);
        rectL = std::sqrt(rmsSquaredL);
        rectR = std::sqrt(rmsSquaredR);
    }

    // Stereo linking: blend between independent and linked detection
    float linked = std::max(rectL, rectR);
    rectL = rectL + linkAmount * (linked - rectL);
    rectR = rectR + linkAmount * (linked - rectR);

    // Determine release coefficient
    float effectiveReleaseCoeff = releaseCoeff;

    if (autoReleaseEnabled)
    {
        float grFactor = std::min(1.0f, currentGainReduction / 12.0f);
        effectiveReleaseCoeff = releaseSlowCoeff + grFactor * (releaseFastCoeff - releaseSlowCoeff);
    }

    // Left channel
    if (rectL > envelopeL)
        envelopeL += attackCoeff * (rectL - envelopeL);
    else
        envelopeL += effectiveReleaseCoeff * (rectL - envelopeL);

    // Right channel
    if (rectR > envelopeR)
        envelopeR += attackCoeff * (rectR - envelopeR);
    else
        envelopeR += effectiveReleaseCoeff * (rectR - envelopeR);

    envL = envelopeL;
    envR = envelopeR;
}
