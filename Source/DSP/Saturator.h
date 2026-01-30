#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"

class Saturator
{
public:
    enum class CharacterMode
    {
        Clean,      // No saturation
        Punch,      // Asymmetric, transient enhancement
        Warm,       // Even harmonics, smooth
        Aggressive  // Harder clipping, more harmonics
    };

    Saturator();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Process a single sample
    float processSample(float input);

    // Process stereo buffer
    void process(juce::AudioBuffer<float>& buffer);

    // Setters
    void setCharacterMode(CharacterMode mode);
    void setDrive(float driveAmount);  // 0.0 - 1.0
    void setBypass(bool shouldBypass);

private:
    // DC blocker filter state
    float dcBlockerStateL = 0.0f;
    float dcBlockerStateR = 0.0f;
    float dcBlockerCoeff = 0.995f;

    // Parameters
    CharacterMode characterMode = CharacterMode::Clean;
    float drive = 0.0f;
    bool bypassed = false;

    // Runtime
    double currentSampleRate = 44100.0;
};
