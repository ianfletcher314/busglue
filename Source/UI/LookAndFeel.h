#pragma once

#include <JuceHeader.h>

namespace Colors
{
    const juce::Colour background     = juce::Colour(0xff1a1a1a);
    const juce::Colour panelBg        = juce::Colour(0xff252525);
    const juce::Colour panelBorder    = juce::Colour(0xff353535);
    const juce::Colour accent         = juce::Colour(0xff4a9eff);
    const juce::Colour accentWarm     = juce::Colour(0xffff8844);
    const juce::Colour textPrimary    = juce::Colour(0xfff0f0f0);
    const juce::Colour textSecondary  = juce::Colour(0xff909090);
    const juce::Colour textDim        = juce::Colour(0xff606060);
    const juce::Colour meterGreen     = juce::Colour(0xff22c55e);
    const juce::Colour meterYellow    = juce::Colour(0xffeab308);
    const juce::Colour meterRed       = juce::Colour(0xffef4444);
    const juce::Colour meterAmber     = juce::Colour(0xffff9933);
    const juce::Colour knobBody       = juce::Colour(0xff404040);
    const juce::Colour knobRing       = juce::Colour(0xff505050);
    const juce::Colour ledOff         = juce::Colour(0xff331111);
    const juce::Colour ledOn          = juce::Colour(0xffff3333);
    const juce::Colour ledAmberOff    = juce::Colour(0xff332211);
    const juce::Colour ledAmberOn     = juce::Colour(0xffffaa33);
}

class BusGlueLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BusGlueLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, Colors::textPrimary);
        setColour(juce::Slider::textBoxBackgroundColourId, Colors::panelBg);
        setColour(juce::Slider::textBoxOutlineColourId, Colors::panelBorder);
        setColour(juce::Label::textColourId, Colors::textPrimary);
        setColour(juce::ComboBox::backgroundColourId, Colors::panelBg);
        setColour(juce::ComboBox::textColourId, Colors::textPrimary);
        setColour(juce::ComboBox::outlineColourId, Colors::panelBorder);
        setColour(juce::ComboBox::arrowColourId, Colors::textSecondary);
        setColour(juce::PopupMenu::backgroundColourId, Colors::panelBg);
        setColour(juce::PopupMenu::textColourId, Colors::textPrimary);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::accent);
        setColour(juce::ToggleButton::textColourId, Colors::textPrimary);
        setColour(juce::ToggleButton::tickColourId, Colors::accent);
    }

    void setAccentColour(juce::Colour c) { accentColour = c; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float, float,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(2.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

        // Outer ring (knurled edge)
        g.setColour(juce::Colour(0xff303030));
        g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        // Knurl pattern
        g.setColour(juce::Colour(0xff404040));
        int numKnurls = 24;
        for (int i = 0; i < numKnurls; ++i)
        {
            float angle = i * juce::MathConstants<float>::twoPi / numKnurls;
            float x1 = cx + (radius - 1.0f) * std::cos(angle);
            float y1 = cy + (radius - 1.0f) * std::sin(angle);
            float x2 = cx + (radius - 4.0f) * std::cos(angle);
            float y2 = cy + (radius - 4.0f) * std::sin(angle);
            g.drawLine(x1, y1, x2, y2, 1.5f);
        }

        // Main knob body with gradient
        float innerRadius = radius * 0.78f;
        juce::ColourGradient knobGradient(juce::Colour(0xff555555), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
                                           juce::Colour(0xff252525), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Ring
        g.setColour(juce::Colour(0xff606060));
        g.drawEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

        // Indicator line (7 o'clock to 5 o'clock range)
        float indicatorAngle = juce::jmap(sliderPosProportional, 0.0f, 1.0f, -2.356f, 2.356f) + juce::MathConstants<float>::pi;
        float indicatorLength = innerRadius * 0.65f;
        float ix1 = cx + (innerRadius * 0.2f) * std::cos(indicatorAngle);
        float iy1 = cy + (innerRadius * 0.2f) * std::sin(indicatorAngle);
        float ix2 = cx + indicatorLength * std::cos(indicatorAngle);
        float iy2 = cy + indicatorLength * std::sin(indicatorAngle);
        g.setColour(accentColour);
        g.drawLine(ix1, iy1, ix2, iy2, 3.0f);

        // Center cap
        float capRadius = innerRadius * 0.25f;
        g.setColour(juce::Colour(0xff404040));
        g.fillEllipse(cx - capRadius, cy - capRadius, capRadius * 2.0f, capRadius * 2.0f);

        // Draw tick marks for stepped sliders
        if (slider.getInterval() > 0)
        {
            drawStepMarkers(g, cx, cy, radius, slider);
        }
    }

    void drawStepMarkers(juce::Graphics& g, float cx, float cy, float radius,
                         juce::Slider& slider)
    {
        auto range = slider.getRange();
        double step = slider.getInterval();
        if (step <= 0) return;

        int numSteps = static_cast<int>((range.getEnd() - range.getStart()) / step) + 1;

        g.setColour(Colors::textDim);

        for (int i = 0; i < numSteps; ++i)
        {
            float normalized = static_cast<float>(i) / static_cast<float>(numSteps - 1);
            float angle = juce::jmap(normalized, 0.0f, 1.0f, -2.356f, 2.356f) + juce::MathConstants<float>::pi;

            float outerR = radius + 4.0f;
            float innerR = radius + 8.0f;

            float x1 = cx + outerR * std::cos(angle);
            float y1 = cy + outerR * std::sin(angle);
            float x2 = cx + innerR * std::cos(angle);
            float y2 = cy + innerR * std::sin(angle);

            g.drawLine(x1, y1, x2, y2, 1.5f);
        }
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          const juce::Slider::SliderStyle style, juce::Slider&) override
    {
        if (style == juce::Slider::LinearHorizontal)
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
            float trackHeight = 4.0f;
            float trackY = bounds.getCentreY() - trackHeight / 2.0f;

            // Track background
            g.setColour(Colors::panelBg);
            g.fillRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackHeight, 2.0f);

            // Filled portion
            float fillWidth = sliderPos - bounds.getX();
            g.setColour(accentColour);
            g.fillRoundedRectangle(bounds.getX(), trackY, fillWidth, trackHeight, 2.0f);

            // Thumb
            float thumbSize = 14.0f;
            g.setColour(Colors::textPrimary);
            g.fillEllipse(sliderPos - thumbSize / 2.0f, bounds.getCentreY() - thumbSize / 2.0f,
                          thumbSize, thumbSize);
        }
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

        g.setColour(Colors::panelBg);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(Colors::panelBorder);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

        // Arrow
        auto arrowZone = juce::Rectangle<float>(width - 20.0f, 0.0f, 15.0f, (float)height);
        juce::Path arrow;
        arrow.startNewSubPath(arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f);
        arrow.lineTo(arrowZone.getCentreX(), arrowZone.getCentreY() + 3.0f);
        arrow.lineTo(arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);

        g.setColour(box.isEnabled() ? Colors::textSecondary : Colors::textDim);
        g.strokePath(arrow, juce::PathStrokeType(2.0f));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // LED style toggle
        float ledSize = 12.0f;
        float ledX = 4.0f;
        float ledY = bounds.getCentreY() - ledSize / 2.0f;

        bool isOn = button.getToggleState();

        // LED glow
        if (isOn)
        {
            g.setColour(accentColour.withAlpha(0.3f));
            g.fillEllipse(ledX - 2, ledY - 2, ledSize + 4, ledSize + 4);
        }

        // LED body
        g.setColour(isOn ? accentColour : Colors::ledOff);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);

        // LED highlight
        g.setColour(juce::Colours::white.withAlpha(isOn ? 0.4f : 0.1f));
        g.fillEllipse(ledX + 2, ledY + 2, 4, 4);

        // Text
        g.setColour(shouldDrawButtonAsHighlighted ? Colors::textPrimary : Colors::textSecondary);
        g.setFont(12.0f);

        auto textBounds = bounds.withTrimmedLeft(ledSize + 10);
        g.drawText(button.getButtonText(), textBounds, juce::Justification::centredLeft);

        juce::ignoreUnused(shouldDrawButtonAsDown);
    }

private:
    juce::Colour accentColour = Colors::accent;
};

// Specialized look and feel for stepped knobs
class SteppedKnobLookAndFeel : public BusGlueLookAndFeel
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        // Call parent for main drawing
        BusGlueLookAndFeel::drawRotarySlider(g, x, y, width, height,
                                              sliderPosProportional, rotaryStartAngle,
                                              rotaryEndAngle, slider);

        // Draw detent marks for stepped controls
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(2.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

        // Highlight current step position with brighter tick
        float currentAngle = juce::jmap(sliderPosProportional, 0.0f, 1.0f, -2.356f, 2.356f) + juce::MathConstants<float>::pi;
        float tickR1 = radius + 6.0f;
        float tickR2 = radius + 12.0f;

        g.setColour(Colors::accent);
        float tx1 = cx + tickR1 * std::cos(currentAngle);
        float ty1 = cy + tickR1 * std::sin(currentAngle);
        float tx2 = cx + tickR2 * std::cos(currentAngle);
        float ty2 = cy + tickR2 * std::sin(currentAngle);
        g.drawLine(tx1, ty1, tx2, ty2, 2.0f);
    }
};
