#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"
#include "EnvelopeFollower.h"
#include "Saturator.h"

class BusCompressor
{
public:
    // Stepped attack times (like SSL hardware)
    static constexpr float ATTACK_VALUES[] = { 0.1f, 0.3f, 1.0f, 3.0f, 10.0f, 30.0f };
    static constexpr int NUM_ATTACK_STEPS = 6;

    // Stepped release times
    static constexpr float RELEASE_VALUES[] = { 0.1f, 0.3f, 0.6f, 1.2f, -1.0f };  // -1 = Auto
    static constexpr int NUM_RELEASE_STEPS = 5;

    // Fixed ratio options (like SSL)
    static constexpr float RATIO_VALUES[] = { 2.0f, 4.0f, 10.0f };
    static constexpr int NUM_RATIO_STEPS = 3;

    enum class Topology
    {
        FeedForward,
        FeedBack
    };

    BusCompressor();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void processSidechain(juce::AudioBuffer<float>& mainBuffer,
                          const juce::AudioBuffer<float>& sidechainBuffer);
    void reset();

    // Setters
    void setThreshold(float thresholdDb);           // -40 to 0 dB
    void setRatio(float ratio);                     // Continuous: 1 to 20, or use index
    void setRatioIndex(int index);                  // Stepped: 0-2 for 2:1, 4:1, 10:1
    void setAttackIndex(int index);                 // Stepped: 0-5
    void setAttackMs(float ms);                     // Continuous
    void setReleaseIndex(int index);                // Stepped: 0-4 (4 = auto)
    void setReleaseMs(float ms);                    // Continuous
    void setAutoRelease(bool enabled);
    void setKnee(float kneeDb);                     // 0 (hard) to 12 dB (soft)
    void setMix(float mixPercent);                  // 0-100% parallel compression
    void setMakeupGain(float gainDb);               // 0 to 24 dB
    void setAutoMakeup(bool enabled);
    void setSidechainHPF(float freqHz);             // 20-300 Hz
    void setStereoLink(float linkPercent);          // 0-100%
    void setDetectionMode(EnvelopeFollower::DetectionMode mode);
    void setTopology(Topology topology);
    void setCharacterMode(Saturator::CharacterMode mode);
    void setBypass(bool shouldBypass);

    // Getters for metering
    float getGainReduction() const { return currentGainReduction; }
    float getInputLevelL() const { return inputLevelL; }
    float getInputLevelR() const { return inputLevelR; }
    float getOutputLevelL() const { return outputLevelL; }
    float getOutputLevelR() const { return outputLevelR; }

private:
    float computeGain(float inputDb);
    void updateSidechainFilter();
    float processBiquad(float input, float& z1, float& z2, const DSPUtils::BiquadCoeffs& c);

    // Parameters
    float threshold = -20.0f;
    float ratio = 4.0f;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    bool autoReleaseEnabled = false;
    float kneeDb = 0.0f;
    float mix = 100.0f;
    float makeupGain = 0.0f;
    bool autoMakeupEnabled = false;
    float sidechainHPFFreq = 20.0f;
    float stereoLink = 100.0f;
    Topology topology = Topology::FeedForward;
    bool bypassed = false;

    // DSP Modules
    EnvelopeFollower envelopeFollower;
    Saturator saturator;

    // Sidechain HPF state
    DSPUtils::BiquadCoeffs hpfCoeffs;
    float hpfZ1L = 0.0f, hpfZ2L = 0.0f;
    float hpfZ1R = 0.0f, hpfZ2R = 0.0f;

    // Feedback topology state
    float feedbackGainL = 1.0f;
    float feedbackGainR = 1.0f;

    // Metering
    float currentGainReduction = 0.0f;
    float inputLevelL = 0.0f;
    float inputLevelR = 0.0f;
    float outputLevelL = 0.0f;
    float outputLevelR = 0.0f;

    // Gain smoothing
    float smoothedGainL = 1.0f;
    float smoothedGainR = 1.0f;
    float gainSmoothCoeff = 0.0f;

    // Runtime
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
