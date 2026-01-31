#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/MeterComponents.h"

class BusGlueAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::Timer
{
public:
    BusGlueAudioProcessorEditor(BusGlueAudioProcessor&);
    ~BusGlueAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    BusGlueAudioProcessor& audioProcessor;

    // Custom LookAndFeel
    std::unique_ptr<BusGlueLookAndFeel> lookAndFeel;
    std::unique_ptr<SteppedKnobLookAndFeel> steppedLookAndFeel;

    // Main controls
    juce::Slider thresholdSlider;
    juce::Slider kneeSlider;
    juce::Slider mixSlider;
    juce::Slider makeupSlider;
    juce::Slider sidechainHPFSlider;
    juce::Slider stereoLinkSlider;
    juce::Slider ratioContinuousSlider;

    // Stepped controls (displayed as rotary but snapped)
    juce::ComboBox ratioSelector;
    juce::ComboBox attackSelector;
    juce::ComboBox releaseSelector;
    juce::ComboBox detectionSelector;
    juce::ComboBox topologySelector;
    juce::ComboBox characterSelector;

    // Toggle buttons
    juce::ToggleButton bypassButton;
    juce::ToggleButton autoMakeupButton;
    juce::ToggleButton steppedRatioButton;

    // Labels
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label kneeLabel;
    juce::Label mixLabel;
    juce::Label makeupLabel;
    juce::Label scHPFLabel;
    juce::Label stereoLinkLabel;
    juce::Label detectionLabel;
    juce::Label topologyLabel;
    juce::Label characterLabel;
    juce::Label titleLabel;
    juce::Label grLabel;

    // Meters
    StereoMeter inputMeter;
    StereoMeter outputMeter;
    GainReductionMeter grMeter;
    VUMeter vuMeter;

    // Smoothed metering values
    float smoothedInputL = 0.0f;
    float smoothedInputR = 0.0f;
    float smoothedOutputL = 0.0f;
    float smoothedOutputR = 0.0f;
    float smoothedGR = 0.0f;

    // Background image
    juce::Image backgroundImage;

    // APVTS Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sidechainHPFAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoLinkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioContinuousAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> detectionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> topologyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> characterAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoMakeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> steppedRatioAttachment;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void setupComboBox(juce::ComboBox& box, juce::Label& label, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BusGlueAudioProcessorEditor)
};
