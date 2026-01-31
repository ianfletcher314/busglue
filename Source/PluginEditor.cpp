#include "PluginEditor.h"

BusGlueAudioProcessorEditor::BusGlueAudioProcessorEditor(BusGlueAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    DBG("BusGlue: Editor constructor ENTRY - processor=" << (void*)&p);

    // Initialize LookAndFeel
    DBG("BusGlue: Editor - creating LookAndFeel");
    lookAndFeel = std::make_unique<BusGlueLookAndFeel>();
    steppedLookAndFeel = std::make_unique<SteppedKnobLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    DBG("BusGlue: Editor - LookAndFeel created OK");

    // Title label - compact
    DBG("BusGlue: Editor - creating title label");
    titleLabel.setText("BUS GLUE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Colors::accent);  // Green title
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // GR label
    grLabel.setText("GR", juce::dontSendNotification);
    grLabel.setFont(juce::Font(10.0f));
    grLabel.setColour(juce::Label::textColourId, Colors::textSecondary);
    grLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(grLabel);
    DBG("BusGlue: Editor - labels created");

    // Setup main knobs
    DBG("BusGlue: Editor - creating sliders");
    setupSlider(thresholdSlider, thresholdLabel, "THRESHOLD");
    setupSlider(kneeSlider, kneeLabel, "KNEE");
    setupSlider(mixSlider, mixLabel, "MIX");
    setupSlider(makeupSlider, makeupLabel, "MAKEUP");
    setupSlider(sidechainHPFSlider, scHPFLabel, "SC HPF");
    setupSlider(stereoLinkSlider, stereoLinkLabel, "LINK");
    setupSlider(ratioContinuousSlider, ratioLabel, "RATIO");

    // Configure ratio slider for continuous mode
    ratioContinuousSlider.setVisible(false);
    DBG("BusGlue: Editor - sliders created");

    // Setup combo boxes
    DBG("BusGlue: Editor - creating combo boxes");
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
    DBG("BusGlue: Editor - combo boxes created");

    // Toggle buttons
    DBG("BusGlue: Editor - creating buttons");
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
    DBG("BusGlue: Editor - buttons created");

    // Meters
    DBG("BusGlue: Editor - creating meters");
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    grMeter.setOrientation(GainReductionMeter::Orientation::Horizontal);
    addAndMakeVisible(grMeter);

    addAndMakeVisible(vuMeter);
    DBG("BusGlue: Editor - meters created");

    // Create APVTS attachments
    DBG("BusGlue: Editor - creating APVTS attachments");
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
    DBG("BusGlue: Editor - APVTS attachments created");

    // Initialize visibility based on stepped ratio state
    DBG("BusGlue: Editor - initializing visibility");
    bool stepped = steppedRatioButton.getToggleState();
    ratioSelector.setVisible(stepped);
    ratioContinuousSlider.setVisible(!stepped);

    // Load background image
    juce::File imageFile("/Users/ianfletcher/busglue/Source/background.png");
    if (imageFile.existsAsFile())
        backgroundImage = juce::ImageFileFormat::loadFrom(imageFile);
    DBG("BusGlue: Background image loaded: " << (backgroundImage.isValid() ? "yes" : "no"));

    // Set size and start timer - compact but room for VU
    DBG("BusGlue: Editor - setting size 650x260");
    setSize(650, 260);

    // Defer timer start to avoid potential deadlock with host's message thread during plugin loading.
    // Starting a timer synchronously in the constructor can block if the host's message loop
    // is not fully ready (particularly in Logic Pro during AU validation).
    DBG("BusGlue: Editor - scheduling async timer start");
    juce::MessageManager::callAsync([this]() {
        DBG("BusGlue: Editor - async callback, isShowing=" << isShowing());
        if (isShowing())  // Safety check - only start if editor is still visible
            startTimerHz(30);
    });
    DBG("BusGlue: Editor constructor EXIT - SUCCESS");
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(10.0f));
    label.setColour(juce::Label::textColourId, Colors::textSecondary);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void BusGlueAudioProcessorEditor::setupComboBox(juce::ComboBox& box, juce::Label& label,
                                                 const juce::String& labelText)
{
    addAndMakeVisible(box);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(10.0f));
    label.setColour(juce::Label::textColourId, Colors::textSecondary);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void BusGlueAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background - dark green
    g.fillAll(Colors::background);

    // Header area
    g.setColour(Colors::panelBg);
    g.fillRoundedRectangle(4.0f, 2.0f, getWidth() - 8.0f, 24.0f, 4.0f);

    // Main panel area
    auto mainPanelBounds = juce::Rectangle<float>(4.0f, 28.0f, getWidth() - 8.0f, getHeight() - 32.0f);

    // Draw background image if available
    if (backgroundImage.isValid())
    {
        // Draw image scaled to fit, with dark overlay for readability
        g.saveState();

        // Clip to rounded rectangle
        juce::Path clipPath;
        clipPath.addRoundedRectangle(mainPanelBounds, 4.0f);
        g.reduceClipRegion(clipPath);

        // Draw the image scaled to fill
        g.drawImage(backgroundImage, mainPanelBounds,
                    juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);

        // Dark overlay for contrast (semi-transparent)
        g.setColour(Colors::background.withAlpha(0.7f));
        g.fillRect(mainPanelBounds);

        g.restoreState();
    }
    else
    {
        // Fallback to solid color
        g.setColour(Colors::panelBg);
        g.fillRoundedRectangle(mainPanelBounds, 4.0f);
    }

    // Section dividers
    g.setColour(Colors::panelBorder.withAlpha(0.4f));

    float dividerY = 30.0f;
    float dividerHeight = getHeight() - 34.0f;

    g.drawLine(103.0f, dividerY, 103.0f, dividerY + dividerHeight, 1.0f);
    g.drawLine(282.0f, dividerY, 282.0f, dividerY + dividerHeight, 1.0f);
    g.drawLine(486.0f, dividerY, 486.0f, dividerY + dividerHeight, 1.0f);
}

void BusGlueAudioProcessorEditor::resized()
{
    // Layout constants - compact
    const int knobSize = 60;
    const int knobTotal = 78;
    const int bigKnobSize = 72;
    const int bigKnobTotal = 90;
    const int labelHeight = 12;
    const int comboHeight = 18;
    const int padding = 4;

    auto bounds = getLocalBounds();

    // Header
    titleLabel.setBounds(14, 6, 100, 20);
    bypassButton.setBounds(getWidth() - 72, 6, 60, 18);
    grMeter.setBounds(120, 4, 180, 22);
    grLabel.setBounds(305, 4, 22, 22);

    // Main area after header
    bounds.removeFromTop(28);
    bounds.reduce(6, 2);

    int topY = bounds.getY();

    // Bottom alignment line - everything aligns here with margin
    int alignBottom = bounds.getBottom() - 8;  // Small margin from bottom
    int bottomRowY = alignBottom - comboHeight;  // For combos/buttons
    int bottomKnobY = alignBottom - knobTotal;   // For knobs

    // Section widths
    int inputW = 95;
    int dynamicsW = 175;
    int characterW = 200;

    // ===== INPUT SECTION =====
    auto inputSection = bounds.removeFromLeft(inputW);
    bounds.removeFromLeft(padding);

    // Input meter - shorter, aligned top to bottom
    auto meterArea = inputSection.removeFromLeft(22);
    inputMeter.setBounds(meterArea.getX(), topY, 20, alignBottom - topY);
    inputSection.removeFromLeft(2);

    // Threshold - top
    int knobX = inputSection.getX() + (inputSection.getWidth() - knobSize) / 2;
    thresholdLabel.setBounds(inputSection.getX(), topY, inputSection.getWidth(), labelHeight);
    thresholdSlider.setBounds(knobX, topY + labelHeight, knobSize, knobTotal);

    // SC HPF - aligned at bottom
    scHPFLabel.setBounds(inputSection.getX(), bottomKnobY - labelHeight - 2, inputSection.getWidth(), labelHeight);
    sidechainHPFSlider.setBounds(knobX, bottomKnobY, knobSize, knobTotal);

    // ===== DYNAMICS SECTION =====
    auto dynamicsSection = bounds.removeFromLeft(dynamicsW);
    bounds.removeFromLeft(padding);

    int col1W = 80;
    int col2W = dynamicsSection.getWidth() - col1W;

    // Top row: Ratio, Attack
    ratioLabel.setBounds(dynamicsSection.getX(), topY, col1W, labelHeight);
    ratioSelector.setBounds(dynamicsSection.getX() + 2, topY + labelHeight, col1W - 4, comboHeight);
    ratioContinuousSlider.setBounds(dynamicsSection.getX() + 6, topY + labelHeight, knobSize - 10, knobSize - 10);
    steppedRatioButton.setBounds(dynamicsSection.getX() + 2, topY + labelHeight + comboHeight + 2, col1W - 4, 14);

    attackLabel.setBounds(dynamicsSection.getX() + col1W, topY, col2W, labelHeight);
    attackSelector.setBounds(dynamicsSection.getX() + col1W + 2, topY + labelHeight, col2W - 4, comboHeight);

    // Second row: Release, Knee
    int row2Y = topY + labelHeight + comboHeight + 16;
    releaseLabel.setBounds(dynamicsSection.getX(), row2Y, col1W, labelHeight);
    releaseSelector.setBounds(dynamicsSection.getX() + 2, row2Y + labelHeight, col1W - 4, comboHeight);

    kneeLabel.setBounds(dynamicsSection.getX() + col1W, row2Y, col2W, labelHeight);
    knobX = dynamicsSection.getX() + col1W + (col2W - knobSize) / 2;
    kneeSlider.setBounds(knobX, row2Y + labelHeight, knobSize, knobTotal);

    // Bottom row: Detection, Topology - aligned at bottom
    detectionLabel.setBounds(dynamicsSection.getX(), bottomRowY - labelHeight - 2, col1W, labelHeight);
    detectionSelector.setBounds(dynamicsSection.getX() + 2, bottomRowY, col1W - 4, comboHeight);

    topologyLabel.setBounds(dynamicsSection.getX() + col1W, bottomRowY - labelHeight - 2, col2W, labelHeight);
    topologySelector.setBounds(dynamicsSection.getX() + col1W + 2, bottomRowY, col2W - 4, comboHeight);

    // ===== CHARACTER SECTION =====
    auto characterSection = bounds.removeFromLeft(characterW);
    bounds.removeFromLeft(padding);

    // Calculate space above Mix/Link
    int spaceAboveMixLink = bottomKnobY - labelHeight - 4 - topY;

    // VU Meter - bigger, room for all numbers including 10
    int vuHeight = 82;
    vuMeter.setBounds(characterSection.getX() + 15, topY - 2, characterSection.getWidth() - 30, vuHeight);

    // Character selector - below VU meter
    int charY = topY + vuHeight + 4;
    int charComboHeight = 22;
    characterLabel.setBounds(characterSection.getX(), charY, characterSection.getWidth(), labelHeight);
    characterSelector.setBounds(characterSection.getX() + 15, charY + labelHeight + 2, characterSection.getWidth() - 30, charComboHeight);

    // Mix and Link side by side - aligned at bottom (unchanged)
    int halfW = characterSection.getWidth() / 2;
    mixLabel.setBounds(characterSection.getX(), bottomKnobY - labelHeight - 2, halfW, labelHeight);
    knobX = characterSection.getX() + (halfW - knobSize) / 2;
    mixSlider.setBounds(knobX, bottomKnobY, knobSize, knobTotal);

    stereoLinkLabel.setBounds(characterSection.getX() + halfW, bottomKnobY - labelHeight - 2, halfW, labelHeight);
    knobX = characterSection.getX() + halfW + (halfW - knobSize) / 2;
    stereoLinkSlider.setBounds(knobX, bottomKnobY, knobSize, knobTotal);

    // ===== OUTPUT SECTION =====
    auto outputSection = bounds;

    // Output meter - aligned
    auto outMeterArea = outputSection.removeFromRight(22);
    outputMeter.setBounds(outMeterArea.getX(), topY, 20, alignBottom - topY);
    outputSection.removeFromRight(2);

    // Makeup - top, bigger
    makeupLabel.setBounds(outputSection.getX(), topY, outputSection.getWidth(), labelHeight);
    knobX = outputSection.getX() + (outputSection.getWidth() - bigKnobSize) / 2;
    makeupSlider.setBounds(knobX, topY + labelHeight, bigKnobSize, bigKnobTotal);

    // Auto button - aligned at bottom
    autoMakeupButton.setBounds(outputSection.getX() + 2, bottomRowY, outputSection.getWidth() - 4, comboHeight);
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
