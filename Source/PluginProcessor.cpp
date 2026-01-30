#include "PluginProcessor.h"
#include "PluginEditor.h"

BusGlueAudioProcessor::BusGlueAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache parameter pointers
    bypass = apvts.getRawParameterValue("bypass");
    threshold = apvts.getRawParameterValue("threshold");
    ratioChoice = apvts.getRawParameterValue("ratioChoice");
    ratioContinuous = apvts.getRawParameterValue("ratioContinuous");
    attackChoice = apvts.getRawParameterValue("attackChoice");
    releaseChoice = apvts.getRawParameterValue("releaseChoice");
    knee = apvts.getRawParameterValue("knee");
    mix = apvts.getRawParameterValue("mix");
    makeupGain = apvts.getRawParameterValue("makeupGain");
    autoMakeup = apvts.getRawParameterValue("autoMakeup");
    sidechainHPF = apvts.getRawParameterValue("sidechainHPF");
    stereoLink = apvts.getRawParameterValue("stereoLink");
    detectionMode = apvts.getRawParameterValue("detectionMode");
    topology = apvts.getRawParameterValue("topology");
    character = apvts.getRawParameterValue("character");
    useSteppedRatio = apvts.getRawParameterValue("useSteppedRatio");
}

BusGlueAudioProcessor::~BusGlueAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout BusGlueAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 1), "Bypass", false));

    // Threshold (-40 to 0 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("threshold", 1), "Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f, 1.0f),
        -20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Ratio - Stepped choice (SSL-style: 2:1, 4:1, 10:1)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("ratioChoice", 1), "Ratio",
        juce::StringArray{ "2:1", "4:1", "10:1" }, 1));

    // Ratio - Continuous option
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("ratioContinuous", 1), "Ratio Continuous",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f),
        4.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    // Use stepped ratio toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("useSteppedRatio", 1), "Stepped Ratio", true));

    // Attack - Stepped choice (0.1, 0.3, 1, 3, 10, 30 ms)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("attackChoice", 1), "Attack",
        juce::StringArray{ "0.1 ms", "0.3 ms", "1 ms", "3 ms", "10 ms", "30 ms" }, 2));

    // Release - Stepped choice (0.1, 0.3, 0.6, 1.2s, Auto)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("releaseChoice", 1), "Release",
        juce::StringArray{ "100 ms", "300 ms", "600 ms", "1.2 s", "Auto" }, 2));

    // Knee (0 = hard, 12 = soft)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("knee", 1), "Knee",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Mix (parallel compression: 0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1), "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Makeup Gain (0-24 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("makeupGain", 1), "Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Auto Makeup
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("autoMakeup", 1), "Auto Makeup", false));

    // Sidechain HPF (20-300 Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sidechainHPF", 1), "SC HPF",
        juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.5f),
        20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Stereo Link (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stereoLink", 1), "Stereo Link",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Detection Mode (Peak/RMS)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("detectionMode", 1), "Detection",
        juce::StringArray{ "Peak", "RMS" }, 1));

    // Topology (Feed-Forward/Feed-Back)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("topology", 1), "Topology",
        juce::StringArray{ "Feed-Forward", "Feed-Back" }, 0));

    // Character Mode
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("character", 1), "Character",
        juce::StringArray{ "Clean", "Punch", "Warm", "Aggressive" }, 0));

    return { params.begin(), params.end() };
}

void BusGlueAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    compressor.prepare(sampleRate, samplesPerBlock);
}

void BusGlueAudioProcessor::releaseResources()
{
    compressor.reset();
}

bool BusGlueAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Main I/O must be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void BusGlueAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Check bypass
    if (bypass->load() > 0.5f)
        return;

    // Update compressor parameters
    compressor.setThreshold(threshold->load());

    // Ratio - stepped or continuous
    if (useSteppedRatio->load() > 0.5f)
        compressor.setRatioIndex(static_cast<int>(ratioChoice->load()));
    else
        compressor.setRatio(ratioContinuous->load());

    compressor.setAttackIndex(static_cast<int>(attackChoice->load()));
    compressor.setReleaseIndex(static_cast<int>(releaseChoice->load()));
    compressor.setKnee(knee->load());
    compressor.setMix(mix->load());
    compressor.setMakeupGain(makeupGain->load());
    compressor.setAutoMakeup(autoMakeup->load() > 0.5f);
    compressor.setSidechainHPF(sidechainHPF->load());
    compressor.setStereoLink(stereoLink->load());

    // Detection mode
    auto detMode = static_cast<int>(detectionMode->load());
    compressor.setDetectionMode(detMode == 0 ? EnvelopeFollower::DetectionMode::Peak
                                              : EnvelopeFollower::DetectionMode::RMS);

    // Topology
    auto topo = static_cast<int>(topology->load());
    compressor.setTopology(topo == 0 ? BusCompressor::Topology::FeedForward
                                      : BusCompressor::Topology::FeedBack);

    // Character
    auto charMode = static_cast<int>(character->load());
    compressor.setCharacterMode(static_cast<Saturator::CharacterMode>(charMode));

    // Check for external sidechain
    auto mainInputBus = getBus(true, 0);
    auto sidechainBus = getBus(true, 1);

    if (sidechainBus != nullptr && sidechainBus->isEnabled())
    {
        // External sidechain available
        juce::AudioBuffer<float> sidechainBuffer(buffer.getNumChannels(),
                                                  buffer.getNumSamples());

        // Copy sidechain data (channels 2-3 if present)
        for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
        {
            if (ch + 2 < buffer.getNumChannels())
                sidechainBuffer.copyFrom(ch, 0, buffer, ch + 2, 0, buffer.getNumSamples());
            else
                sidechainBuffer.copyFrom(ch, 0, buffer, ch, 0, buffer.getNumSamples());
        }

        // Create main buffer (just first 2 channels)
        juce::AudioBuffer<float> mainBuffer(buffer.getArrayOfWritePointers(), 2, buffer.getNumSamples());

        compressor.processSidechain(mainBuffer, sidechainBuffer);
    }
    else
    {
        // Internal sidechain
        compressor.process(buffer);
    }
}

juce::AudioProcessorEditor* BusGlueAudioProcessor::createEditor()
{
    return new BusGlueAudioProcessorEditor(*this);
}

bool BusGlueAudioProcessor::hasEditor() const { return true; }
const juce::String BusGlueAudioProcessor::getName() const { return JucePlugin_Name; }
bool BusGlueAudioProcessor::acceptsMidi() const { return false; }
bool BusGlueAudioProcessor::producesMidi() const { return false; }
bool BusGlueAudioProcessor::isMidiEffect() const { return false; }
double BusGlueAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int BusGlueAudioProcessor::getNumPrograms() { return 1; }
int BusGlueAudioProcessor::getCurrentProgram() { return 0; }
void BusGlueAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String BusGlueAudioProcessor::getProgramName(int index) { juce::ignoreUnused(index); return {}; }
void BusGlueAudioProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

void BusGlueAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BusGlueAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginInstance()
{
    return new BusGlueAudioProcessor();
}
