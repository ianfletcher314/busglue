#pragma once

#include <JuceHeader.h>
#include "DSP/BusCompressor.h"

class BusGlueAudioProcessor : public juce::AudioProcessor
{
public:
    BusGlueAudioProcessor();
    ~BusGlueAudioProcessor() override;

    // Standard AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public API for editor access
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Metering accessors
    float getInputLevelL() const { return compressor.getInputLevelL(); }
    float getInputLevelR() const { return compressor.getInputLevelR(); }
    float getOutputLevelL() const { return compressor.getOutputLevelL(); }
    float getOutputLevelR() const { return compressor.getOutputLevelR(); }
    float getGainReduction() const { return compressor.getGainReduction(); }

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Module
    BusCompressor compressor;

    // Parameter pointers (cached for fast access)
    std::atomic<float>* bypass = nullptr;
    std::atomic<float>* threshold = nullptr;
    std::atomic<float>* ratioChoice = nullptr;
    std::atomic<float>* ratioContinuous = nullptr;
    std::atomic<float>* attackChoice = nullptr;
    std::atomic<float>* releaseChoice = nullptr;
    std::atomic<float>* knee = nullptr;
    std::atomic<float>* mix = nullptr;
    std::atomic<float>* makeupGain = nullptr;
    std::atomic<float>* autoMakeup = nullptr;
    std::atomic<float>* sidechainHPF = nullptr;
    std::atomic<float>* stereoLink = nullptr;
    std::atomic<float>* detectionMode = nullptr;
    std::atomic<float>* topology = nullptr;
    std::atomic<float>* character = nullptr;
    std::atomic<float>* useSteppedRatio = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BusGlueAudioProcessor)
};
