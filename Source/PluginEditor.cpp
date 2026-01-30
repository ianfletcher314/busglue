#include "PluginEditor.h"

BusGlueAudioProcessorEditor::BusGlueAudioProcessorEditor(BusGlueAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Initialize LookAndFeel
    lookAndFeel = std::make_unique<BusGlueLookAndFeel>();
    steppedLookAndFeel = std::make_unique<SteppedKnobLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    // Title label
    titleLabel.setText("BUS GLUE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Colors::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // GR label
    grLabel.setText("GR", juce::dontSendNotification);
    grLabel.setFont(juce::Font(11.0f));
    grLabel.setColour(juce::Label::textColourId, Colors::textSecondary);
    grLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(grLabel);

    // Setup main knobs
    setupSlider(thresholdSlider, thresholdLabel, "THRESHOLD");
    setupSlider(kneeSlider, kneeLabel, "KNEE");
    setupSlider(mixSlider, mixLabel, "MIX");
    setupSlider(makeupSlider, makeupLabel, "MAKEUP");
    setupSlider(sidechainHPFSlider, scHPFLabel, "SC HPF");
    setupSlider(stereoLinkSlider, stereoLinkLabel, "LINK");
    setupSlider(ratioContinuousSlider, ratioLabel, "RATIO");

    // Configure ratio slider for continuous mode
    ratioContinuousSlider.setVisible(false);

    // Setup combo boxes
    setupComboBox(ratioSelector, ratioLabel, "RATIO");
    ratioSelector.addItemList(juce::StringArray{ "2:1", "4:1", "10:1" }, 1);

    setupComboBox(attackSelector, attackLabel, "ATTACK");
    attackSelector.addItemList(juce::StringArray{ "0.1 ms", "0.3 ms", "1 ms", "3 ms", "10 ms", "30 ms" }, 1);

    setupComboBox(releaseSelector, releaseLabel, "RELEASE");
    releaseSelector.addItemList(juce::StringArray{ "100 ms", "300 ms", "600 ms", "1.2 s", "Auto" }, 1);

    setupComboBox(detectionSelector, detectionLabel, "DETECTION");
    detectionSelector.addItemList(juce::StringArray{ "Peak", "RMS" }, 1);

    setupComboBox(topologySelector, topologyLabel, "TOPOLOGY");
    topologySelector.addItemList(juce::StringArray{ "FF", "FB" }, 1);

    setupComboBox(characterSelector, characterLabel, "CHARACTER");
    characterSelector.addItemList(juce::StringArray{ "Clean", "Punch", "Warm", "Aggressive" }, 1);

    // Toggle buttons
    bypassButton.setButtonText("Bypass");
    addAndMakeVisible(bypassButton);

    autoMakeupButton.setButtonText("Auto");
    addAndMakeVisible(autoMakeupButton);

    steppedRatioButton.setButtonText("Stepped");
    steppedRatioButton.onClick = [this]()
    {
        bool stepped = steppedRatioButton.getToggleState();
        ratioSelector.setVisible(stepped);
        ratioContinuousSlider.setVisible(!stepped);
    };
    addAndMakeVisible(steppedRatioButton);

    // Meters
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    grMeter.setOrientation(GainReductionMeter::Orientation::Horizontal);
    addAndMakeVisible(grMeter);

    addAndMakeVisible(vuMeter);

    // Create APVTS attachments
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "threshold", thresholdSlider);
    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "knee", kneeSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "mix", mixSlider);
    makeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "makeupGain", makeupSlider);
    sidechainHPFAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "sidechainHPF", sidechainHPFSlider);
    stereoLinkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "stereoLink", stereoLinkSlider);
    ratioContinuousAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "ratioContinuous", ratioContinuousSlider);

    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "ratioChoice", ratioSelector);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "attackChoice", attackSelector);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "releaseChoice", releaseSelector);
    detectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "detectionMode", detectionSelector);
    topologyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "topology", topologySelector);
    characterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "character", characterSelector);

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "bypass", bypassButton);
    autoMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "autoMakeup", autoMakeupButton);
    steppedRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "useSteppedRatio", steppedRatioButton);

    // Initialize visibility based on stepped ratio state
    bool stepped = steppedRatioButton.getToggleState();
    ratioSelector.setVisible(stepped);
    ratioContinuousSlider.setVisible(!stepped);

    // Set size and start timer
    setSize(700, 450);
    startTimerHz(30);
}

BusGlueAudioProcessorEditor::~BusGlueAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void BusGlueAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label,
                                               const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(11.0f));
    label.setColour(juce::Label::textColourId, Colors::textSecondary);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void BusGlueAudioProcessorEditor::setupComboBox(juce::ComboBox& box, juce::Label& label,
                                                 const juce::String& labelText)
{
    addAndMakeVisible(box);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(11.0f));
    label.setColour(juce::Label::textColourId, Colors::textSecondary);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void BusGlueAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(Colors::background);

    // Header area
    g.setColour(Colors::panelBg);
    g.fillRoundedRectangle(10.0f, 10.0f, getWidth() - 20.0f, 50.0f, 6.0f);

    // Main panel
    g.fillRoundedRectangle(10.0f, 70.0f, getWidth() - 20.0f, getHeight() - 80.0f, 6.0f);

    // Section dividers
    g.setColour(Colors::panelBorder);

    // Vertical dividers
    float dividerY = 90.0f;
    float dividerHeight = getHeight() - 120.0f;

    g.drawLine(180.0f, dividerY, 180.0f, dividerY + dividerHeight, 1.0f);
    g.drawLine(400.0f, dividerY, 400.0f, dividerY + dividerHeight, 1.0f);
    g.drawLine(580.0f, dividerY, 580.0f, dividerY + dividerHeight, 1.0f);

    // Section labels
    g.setColour(Colors::textDim);
    g.setFont(10.0f);
    g.drawText("INPUT", 20, 75, 150, 15, juce::Justification::centred);
    g.drawText("DYNAMICS", 190, 75, 200, 15, juce::Justification::centred);
    g.drawText("CHARACTER", 410, 75, 160, 15, juce::Justification::centred);
    g.drawText("OUTPUT", 590, 75, 90, 15, juce::Justification::centred);
}

void BusGlueAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Header
    auto headerArea = bounds.removeFromTop(60);
    titleLabel.setBounds(25, 20, 150, 30);
    bypassButton.setBounds(headerArea.getRight() - 100, 20, 80, 25);

    // GR meter in header
    grMeter.setBounds(200, 18, 250, 30);
    grLabel.setBounds(455, 18, 30, 30);

    // Main area
    bounds.removeFromTop(20);  // Spacing
    bounds.reduce(20, 10);

    // Input section (left)
    auto inputSection = bounds.removeFromLeft(160);
    inputMeter.setBounds(inputSection.removeFromLeft(40).reduced(5, 30));

    auto threshArea = inputSection.removeFromTop(inputSection.getHeight() / 2);
    thresholdLabel.setBounds(threshArea.removeFromTop(18));
    thresholdSlider.setBounds(threshArea.reduced(5));

    auto scArea = inputSection;
    scHPFLabel.setBounds(scArea.removeFromTop(18));
    sidechainHPFSlider.setBounds(scArea.reduced(5));

    bounds.removeFromLeft(10);

    // Dynamics section (center)
    auto dynamicsSection = bounds.removeFromLeft(210);

    // Top row: Ratio, Attack
    auto topRow = dynamicsSection.removeFromTop(130);
    auto ratioArea = topRow.removeFromLeft(100);
    ratioLabel.setBounds(ratioArea.removeFromTop(18));
    ratioSelector.setBounds(ratioArea.removeFromTop(25).reduced(5, 0));
    ratioContinuousSlider.setBounds(ratioArea.removeFromTop(70).reduced(5));
    steppedRatioButton.setBounds(ratioArea.removeFromTop(20).reduced(5, 0));

    auto attackArea = topRow;
    attackLabel.setBounds(attackArea.removeFromTop(18));
    attackSelector.setBounds(attackArea.removeFromTop(25).reduced(5, 0));

    // Bottom row: Release, Knee
    auto bottomRow = dynamicsSection.removeFromTop(130);
    auto releaseArea = bottomRow.removeFromLeft(100);
    releaseLabel.setBounds(releaseArea.removeFromTop(18));
    releaseSelector.setBounds(releaseArea.removeFromTop(25).reduced(5, 0));

    auto kneeArea = bottomRow;
    kneeLabel.setBounds(kneeArea.removeFromTop(18));
    kneeSlider.setBounds(kneeArea.reduced(5));

    // Detection and topology row
    auto optionsRow = dynamicsSection;
    auto detArea = optionsRow.removeFromLeft(100);
    detectionLabel.setBounds(detArea.removeFromTop(18));
    detectionSelector.setBounds(detArea.removeFromTop(25).reduced(5, 0));

    auto topoArea = optionsRow;
    topologyLabel.setBounds(topoArea.removeFromTop(18));
    topologySelector.setBounds(topoArea.removeFromTop(25).reduced(5, 0));

    bounds.removeFromLeft(10);

    // Character section
    auto characterSection = bounds.removeFromLeft(170);

    // VU Meter
    vuMeter.setBounds(characterSection.removeFromTop(100).reduced(20, 10));

    // Character selector
    characterLabel.setBounds(characterSection.removeFromTop(18));
    characterSelector.setBounds(characterSection.removeFromTop(28).reduced(10, 0));

    characterSection.removeFromTop(10);

    // Mix and Stereo Link
    auto mixArea = characterSection.removeFromTop(80);
    mixLabel.setBounds(mixArea.removeFromTop(18));
    mixSlider.setBounds(mixArea.reduced(10));

    auto linkArea = characterSection.removeFromTop(80);
    stereoLinkLabel.setBounds(linkArea.removeFromTop(18));
    stereoLinkSlider.setBounds(linkArea.reduced(10));

    bounds.removeFromLeft(10);

    // Output section (right)
    auto outputSection = bounds;

    // Output meter
    outputMeter.setBounds(outputSection.removeFromRight(40).reduced(5, 30));

    auto makeupArea = outputSection.removeFromTop(outputSection.getHeight() / 2);
    makeupLabel.setBounds(makeupArea.removeFromTop(18));
    makeupSlider.setBounds(makeupArea.reduced(5));
    autoMakeupButton.setBounds(outputSection.removeFromTop(25).reduced(10, 0));
}

void BusGlueAudioProcessorEditor::timerCallback()
{
    // Smooth level metering
    float targetInL = audioProcessor.getInputLevelL();
    float targetInR = audioProcessor.getInputLevelR();
    float targetOutL = audioProcessor.getOutputLevelL();
    float targetOutR = audioProcessor.getOutputLevelR();
    float targetGR = audioProcessor.getGainReduction();

    // Smooth attack, faster decay
    smoothedInputL = (targetInL > smoothedInputL)
                         ? smoothedInputL + 0.3f * (targetInL - smoothedInputL)
                         : smoothedInputL * 0.85f;
    smoothedInputR = (targetInR > smoothedInputR)
                         ? smoothedInputR + 0.3f * (targetInR - smoothedInputR)
                         : smoothedInputR * 0.85f;
    smoothedOutputL = (targetOutL > smoothedOutputL)
                          ? smoothedOutputL + 0.3f * (targetOutL - smoothedOutputL)
                          : smoothedOutputL * 0.85f;
    smoothedOutputR = (targetOutR > smoothedOutputR)
                          ? smoothedOutputR + 0.3f * (targetOutR - smoothedOutputR)
                          : smoothedOutputR * 0.85f;

    // GR meter - fast attack, slow release
    smoothedGR = (targetGR > smoothedGR)
                     ? smoothedGR + 0.4f * (targetGR - smoothedGR)
                     : smoothedGR * 0.9f;

    // Update meters
    inputMeter.setLevels(smoothedInputL, smoothedInputR);
    outputMeter.setLevels(smoothedOutputL, smoothedOutputR);
    grMeter.setGainReduction(smoothedGR);
    vuMeter.setGainReduction(smoothedGR);
    vuMeter.updateAnimation();

    // Repaint
    inputMeter.repaintMeters();
    outputMeter.repaintMeters();
    grMeter.repaint();
    vuMeter.repaint();
}
