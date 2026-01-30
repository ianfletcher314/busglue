#include "BusCompressor.h"

// Static member definitions
constexpr float BusCompressor::ATTACK_VALUES[];
constexpr float BusCompressor::RELEASE_VALUES[];
constexpr float BusCompressor::RATIO_VALUES[];

BusCompressor::BusCompressor()
{
}

void BusCompressor::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    envelopeFollower.prepare(sampleRate, samplesPerBlock);
    saturator.prepare(sampleRate, samplesPerBlock);

    // Gain smoothing coefficient (1ms smoothing)
    gainSmoothCoeff = DSPUtils::calculateCoefficient(sampleRate, 1.0f);

    updateSidechainFilter();
    reset();
}

void BusCompressor::reset()
{
    envelopeFollower.reset();
    saturator.reset();

    hpfZ1L = hpfZ2L = 0.0f;
    hpfZ1R = hpfZ2R = 0.0f;

    feedbackGainL = feedbackGainR = 1.0f;
    smoothedGainL = smoothedGainR = 1.0f;

    currentGainReduction = 0.0f;
    inputLevelL = inputLevelR = 0.0f;
    outputLevelL = outputLevelR = 0.0f;
}

void BusCompressor::updateSidechainFilter()
{
    hpfCoeffs = DSPUtils::calcHighPass(currentSampleRate, sidechainHPFFreq, 0.707f);
}

float BusCompressor::processBiquad(float input, float& z1, float& z2, const DSPUtils::BiquadCoeffs& c)
{
    float output = c.b0 * input + z1;
    z1 = c.b1 * input - c.a1 * output + z2;
    z2 = c.b2 * input - c.a2 * output;
    return output;
}

float BusCompressor::computeGain(float inputDb)
{
    // Apply soft knee compression curve
    float outputDb = DSPUtils::computeSoftKnee(inputDb, threshold, ratio, kneeDb);
    float gainDb = outputDb - inputDb;

    return DSPUtils::decibelsToLinear(gainDb);
}

void BusCompressor::setThreshold(float thresholdDb)
{
    threshold = std::clamp(thresholdDb, -40.0f, 0.0f);
}

void BusCompressor::setRatio(float r)
{
    ratio = std::clamp(r, 1.0f, 20.0f);
}

void BusCompressor::setRatioIndex(int index)
{
    if (index >= 0 && index < NUM_RATIO_STEPS)
        ratio = RATIO_VALUES[index];
}

void BusCompressor::setAttackIndex(int index)
{
    if (index >= 0 && index < NUM_ATTACK_STEPS)
    {
        attackMs = ATTACK_VALUES[index];
        envelopeFollower.setAttack(attackMs);
    }
}

void BusCompressor::setAttackMs(float ms)
{
    attackMs = std::clamp(ms, 0.1f, 100.0f);
    envelopeFollower.setAttack(attackMs);
}

void BusCompressor::setReleaseIndex(int index)
{
    if (index >= 0 && index < NUM_RELEASE_STEPS)
    {
        if (index == 4)  // Auto
        {
            autoReleaseEnabled = true;
            envelopeFollower.setAutoRelease(true);
        }
        else
        {
            autoReleaseEnabled = false;
            releaseMs = RELEASE_VALUES[index] * 1000.0f;  // Convert to ms
            envelopeFollower.setRelease(releaseMs);
            envelopeFollower.setAutoRelease(false);
        }
    }
}

void BusCompressor::setReleaseMs(float ms)
{
    releaseMs = std::clamp(ms, 10.0f, 2000.0f);
    envelopeFollower.setRelease(releaseMs);
}

void BusCompressor::setAutoRelease(bool enabled)
{
    autoReleaseEnabled = enabled;
    envelopeFollower.setAutoRelease(enabled);
}

void BusCompressor::setKnee(float knee)
{
    kneeDb = std::clamp(knee, 0.0f, 12.0f);
}

void BusCompressor::setMix(float mixPercent)
{
    mix = std::clamp(mixPercent, 0.0f, 100.0f);
}

void BusCompressor::setMakeupGain(float gainDb)
{
    makeupGain = std::clamp(gainDb, 0.0f, 24.0f);
}

void BusCompressor::setAutoMakeup(bool enabled)
{
    autoMakeupEnabled = enabled;
}

void BusCompressor::setSidechainHPF(float freqHz)
{
    sidechainHPFFreq = std::clamp(freqHz, 20.0f, 300.0f);
    updateSidechainFilter();
}

void BusCompressor::setStereoLink(float linkPercent)
{
    stereoLink = std::clamp(linkPercent, 0.0f, 100.0f);
}

void BusCompressor::setDetectionMode(EnvelopeFollower::DetectionMode mode)
{
    envelopeFollower.setDetectionMode(mode);
}

void BusCompressor::setTopology(Topology topo)
{
    topology = topo;
}

void BusCompressor::setCharacterMode(Saturator::CharacterMode mode)
{
    saturator.setCharacterMode(mode);
    // Set drive based on character - subtle saturation
    switch (mode)
    {
        case Saturator::CharacterMode::Clean:
            saturator.setDrive(0.0f);
            break;
        case Saturator::CharacterMode::Punch:
            saturator.setDrive(0.2f);
            break;
        case Saturator::CharacterMode::Warm:
            saturator.setDrive(0.3f);
            break;
        case Saturator::CharacterMode::Aggressive:
            saturator.setDrive(0.5f);
            break;
    }
}

void BusCompressor::setBypass(bool shouldBypass)
{
    bypassed = shouldBypass;
}

void BusCompressor::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels < 2)
        return;  // Stereo only

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    // Mix factor
    float wetMix = mix / 100.0f;
    float dryMix = 1.0f - wetMix;

    // Auto makeup calculation (approximate)
    float autoMakeupDb = 0.0f;
    if (autoMakeupEnabled)
    {
        // Estimate makeup based on threshold and ratio
        float avgGR = (threshold < 0.0f) ? -threshold * (1.0f - 1.0f / ratio) * 0.5f : 0.0f;
        autoMakeupDb = avgGR * 0.5f;  // Conservative estimate
    }

    float totalMakeup = DSPUtils::decibelsToLinear(makeupGain + autoMakeupDb);

    // Track peak levels for metering
    float peakInL = 0.0f, peakInR = 0.0f;
    float peakOutL = 0.0f, peakOutR = 0.0f;
    float peakGR = 0.0f;

    float linkAmount = stereoLink / 100.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = leftChannel[i];
        float dryR = rightChannel[i];

        // Track input levels
        peakInL = std::max(peakInL, std::abs(dryL));
        peakInR = std::max(peakInR, std::abs(dryR));

        // Sidechain signal (filtered)
        float scL, scR;

        if (topology == Topology::FeedBack)
        {
            // Feedback: use output of previous sample
            scL = dryL * feedbackGainL;
            scR = dryR * feedbackGainR;
        }
        else
        {
            // Feedforward: use input directly
            scL = dryL;
            scR = dryR;
        }

        // Apply sidechain HPF
        scL = processBiquad(scL, hpfZ1L, hpfZ2L, hpfCoeffs);
        scR = processBiquad(scR, hpfZ1R, hpfZ2R, hpfCoeffs);

        // Envelope detection with stereo link
        float envL, envR;
        envelopeFollower.processStereo(scL, scR, linkAmount, envL, envR);

        // Convert to dB
        float envDbL = DSPUtils::linearToDecibels(envL);
        float envDbR = DSPUtils::linearToDecibels(envR);

        // Compute gain reduction
        float gainL = computeGain(envDbL);
        float gainR = computeGain(envDbR);

        // Smooth gain changes
        smoothedGainL += gainSmoothCoeff * (gainL - smoothedGainL);
        smoothedGainR += gainSmoothCoeff * (gainR - smoothedGainR);

        // Store for feedback topology
        feedbackGainL = smoothedGainL;
        feedbackGainR = smoothedGainR;

        // Update envelope follower with current GR for auto-release
        float grDb = DSPUtils::linearToDecibels(std::min(smoothedGainL, smoothedGainR));
        grDb = std::abs(grDb);
        envelopeFollower.setCurrentGainReduction(grDb);
        peakGR = std::max(peakGR, grDb);

        // Apply compression
        float wetL = dryL * smoothedGainL * totalMakeup;
        float wetR = dryR * smoothedGainR * totalMakeup;

        // Mix dry/wet
        float outL = dryL * dryMix + wetL * wetMix;
        float outR = dryR * dryMix + wetR * wetMix;

        leftChannel[i] = outL;
        rightChannel[i] = outR;

        // Track output levels
        peakOutL = std::max(peakOutL, std::abs(outL));
        peakOutR = std::max(peakOutR, std::abs(outR));
    }

    // Apply saturation based on character mode
    saturator.process(buffer);

    // Update metering values
    inputLevelL = peakInL;
    inputLevelR = peakInR;
    outputLevelL = peakOutL;
    outputLevelR = peakOutR;
    currentGainReduction = peakGR;
}

void BusCompressor::processSidechain(juce::AudioBuffer<float>& mainBuffer,
                                      const juce::AudioBuffer<float>& sidechainBuffer)
{
    if (bypassed)
        return;

    const int numSamples = mainBuffer.getNumSamples();
    const int numChannels = mainBuffer.getNumChannels();
    const int scChannels = sidechainBuffer.getNumChannels();

    if (numChannels < 2 || scChannels < 1)
        return;

    float* leftChannel = mainBuffer.getWritePointer(0);
    float* rightChannel = mainBuffer.getWritePointer(1);
    const float* scLeft = sidechainBuffer.getReadPointer(0);
    const float* scRight = (scChannels > 1) ? sidechainBuffer.getReadPointer(1) : scLeft;

    float wetMix = mix / 100.0f;
    float dryMix = 1.0f - wetMix;

    float autoMakeupDb = 0.0f;
    if (autoMakeupEnabled)
    {
        float avgGR = (threshold < 0.0f) ? -threshold * (1.0f - 1.0f / ratio) * 0.5f : 0.0f;
        autoMakeupDb = avgGR * 0.5f;
    }

    float totalMakeup = DSPUtils::decibelsToLinear(makeupGain + autoMakeupDb);

    float peakInL = 0.0f, peakInR = 0.0f;
    float peakOutL = 0.0f, peakOutR = 0.0f;
    float peakGR = 0.0f;

    float linkAmount = stereoLink / 100.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = leftChannel[i];
        float dryR = rightChannel[i];

        peakInL = std::max(peakInL, std::abs(dryL));
        peakInR = std::max(peakInR, std::abs(dryR));

        // Use external sidechain
        float scL = scLeft[i];
        float scR = scRight[i];

        // Apply sidechain HPF
        scL = processBiquad(scL, hpfZ1L, hpfZ2L, hpfCoeffs);
        scR = processBiquad(scR, hpfZ1R, hpfZ2R, hpfCoeffs);

        // Envelope detection
        float envL, envR;
        envelopeFollower.processStereo(scL, scR, linkAmount, envL, envR);

        float envDbL = DSPUtils::linearToDecibels(envL);
        float envDbR = DSPUtils::linearToDecibels(envR);

        float gainL = computeGain(envDbL);
        float gainR = computeGain(envDbR);

        smoothedGainL += gainSmoothCoeff * (gainL - smoothedGainL);
        smoothedGainR += gainSmoothCoeff * (gainR - smoothedGainR);

        float grDb = DSPUtils::linearToDecibels(std::min(smoothedGainL, smoothedGainR));
        grDb = std::abs(grDb);
        envelopeFollower.setCurrentGainReduction(grDb);
        peakGR = std::max(peakGR, grDb);

        float wetL = dryL * smoothedGainL * totalMakeup;
        float wetR = dryR * smoothedGainR * totalMakeup;

        float outL = dryL * dryMix + wetL * wetMix;
        float outR = dryR * dryMix + wetR * wetMix;

        leftChannel[i] = outL;
        rightChannel[i] = outR;

        peakOutL = std::max(peakOutL, std::abs(outL));
        peakOutR = std::max(peakOutR, std::abs(outR));
    }

    saturator.process(mainBuffer);

    inputLevelL = peakInL;
    inputLevelR = peakInR;
    outputLevelL = peakOutL;
    outputLevelR = peakOutR;
    currentGainReduction = peakGR;
}
