#include "Saturator.h"

Saturator::Saturator()
{
}

void Saturator::prepare(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    currentSampleRate = sampleRate;

    // DC blocker coefficient - adjust for sample rate
    dcBlockerCoeff = 1.0f - (20.0f / static_cast<float>(sampleRate));
    dcBlockerCoeff = std::clamp(dcBlockerCoeff, 0.9f, 0.9999f);

    reset();
}

void Saturator::reset()
{
    dcBlockerStateL = 0.0f;
    dcBlockerStateR = 0.0f;
}

void Saturator::setCharacterMode(CharacterMode mode)
{
    characterMode = mode;
}

void Saturator::setDrive(float driveAmount)
{
    drive = std::clamp(driveAmount, 0.0f, 1.0f);
}

void Saturator::setBypass(bool shouldBypass)
{
    bypassed = shouldBypass;
}

float Saturator::processSample(float input)
{
    if (bypassed || characterMode == CharacterMode::Clean)
        return input;

    float output;

    switch (characterMode)
    {
        case CharacterMode::Punch:
            output = DSPUtils::punchSaturate(input, drive);
            break;

        case CharacterMode::Warm:
            output = DSPUtils::warmSaturate(input, drive);
            break;

        case CharacterMode::Aggressive:
            output = DSPUtils::aggressiveSaturate(input, drive);
            break;

        default:
            output = input;
            break;
    }

    return output;
}

void Saturator::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed || characterMode == CharacterMode::Clean)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);
        float& dcState = (ch == 0) ? dcBlockerStateL : dcBlockerStateR;

        for (int i = 0; i < numSamples; ++i)
        {
            float input = channelData[i];
            float saturated = processSample(input);

            // DC blocker (high-pass at ~20Hz)
            float dcBlocked = saturated - dcState;
            dcState = saturated - dcBlocked * dcBlockerCoeff;

            channelData[i] = dcBlocked;
        }
    }
}
