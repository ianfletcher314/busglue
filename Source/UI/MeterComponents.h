#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

// LED-style level meter (vertical)
class LevelMeter : public juce::Component
{
public:
    LevelMeter()
    {
        setOpaque(false);
    }

    void setLevel(float newLevel)
    {
        level = newLevel;
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // Background
        g.setColour(juce::Colour(0xff151515));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Convert to dB and normalize
        float db = juce::Decibels::gainToDecibels(level, -60.0f);
        float normalized = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);
        normalized = juce::jlimit(0.0f, 1.0f, normalized);

        // Draw segmented meter
        int numSegments = 16;
        float segmentHeight = (bounds.getHeight() - 2.0f) / numSegments;
        float segmentGap = 1.0f;
        int litSegments = static_cast<int>(normalized * numSegments);

        for (int i = 0; i < numSegments; ++i)
        {
            float segY = bounds.getBottom() - (i + 1) * segmentHeight;

            juce::Colour segColour;
            if (i < 10)
                segColour = Colors::meterGreen;
            else if (i < 14)
                segColour = Colors::meterYellow;
            else
                segColour = Colors::meterRed;

            bool isLit = i < litSegments;
            g.setColour(isLit ? segColour : segColour.withAlpha(0.12f));

            auto segBounds = juce::Rectangle<float>(bounds.getX() + 2.0f, segY,
                                                     bounds.getWidth() - 4.0f,
                                                     segmentHeight - segmentGap);
            g.fillRoundedRectangle(segBounds, 1.0f);
        }
    }

private:
    float level = 0.0f;
};

// LED-style gain reduction meter (red LEDs, shows from right to left or top to bottom)
class GainReductionMeter : public juce::Component
{
public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    GainReductionMeter()
    {
        setOpaque(false);
    }

    void setGainReduction(float grDb)
    {
        gainReduction = grDb;
    }

    void setOrientation(Orientation o)
    {
        orientation = o;
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // Background panel
        g.setColour(juce::Colour(0xff0f0f0f));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Normalize GR (0-20dB range)
        float normalizedGR = juce::jlimit(0.0f, 1.0f, gainReduction / 20.0f);

        int numLEDs = 10;

        if (orientation == Orientation::Horizontal)
        {
            float ledWidth = (bounds.getWidth() - 2.0f) / numLEDs;
            float ledGap = 2.0f;
            int litLEDs = static_cast<int>(normalizedGR * numLEDs);

            for (int i = 0; i < numLEDs; ++i)
            {
                // LEDs light from right to left (0dB on right, -20dB on left)
                float ledX = bounds.getRight() - (i + 1) * ledWidth;
                bool isLit = i < litLEDs;

                // Color gradient: green for little GR, amber for medium, red for heavy
                juce::Colour ledColour;
                if (i < 4)
                    ledColour = Colors::meterGreen;
                else if (i < 7)
                    ledColour = Colors::meterAmber;
                else
                    ledColour = Colors::meterRed;

                g.setColour(isLit ? ledColour : ledColour.withAlpha(0.1f));

                auto ledBounds = juce::Rectangle<float>(ledX + ledGap / 2.0f, bounds.getY() + 3.0f,
                                                         ledWidth - ledGap, bounds.getHeight() - 6.0f);
                g.fillRoundedRectangle(ledBounds, 2.0f);

                // LED highlight when lit
                if (isLit)
                {
                    g.setColour(juce::Colours::white.withAlpha(0.3f));
                    g.fillRoundedRectangle(ledBounds.removeFromTop(ledBounds.getHeight() * 0.3f), 2.0f);
                }
            }

            // Draw dB labels
            g.setColour(Colors::textDim);
            g.setFont(9.0f);
            g.drawText("0", bounds.removeFromRight(20), juce::Justification::centred);
            g.drawText("-20", bounds.removeFromLeft(25), juce::Justification::centred);
        }
        else // Vertical
        {
            float ledHeight = (bounds.getHeight() - 2.0f) / numLEDs;
            float ledGap = 2.0f;
            int litLEDs = static_cast<int>(normalizedGR * numLEDs);

            for (int i = 0; i < numLEDs; ++i)
            {
                float ledY = bounds.getY() + i * ledHeight;
                bool isLit = i < litLEDs;

                juce::Colour ledColour;
                if (i < 4)
                    ledColour = Colors::meterGreen;
                else if (i < 7)
                    ledColour = Colors::meterAmber;
                else
                    ledColour = Colors::meterRed;

                g.setColour(isLit ? ledColour : ledColour.withAlpha(0.1f));

                auto ledBounds = juce::Rectangle<float>(bounds.getX() + 3.0f, ledY + ledGap / 2.0f,
                                                         bounds.getWidth() - 6.0f, ledHeight - ledGap);
                g.fillRoundedRectangle(ledBounds, 2.0f);

                if (isLit)
                {
                    g.setColour(juce::Colours::white.withAlpha(0.3f));
                    g.fillRoundedRectangle(ledBounds.removeFromLeft(ledBounds.getWidth() * 0.3f), 2.0f);
                }
            }
        }
    }

private:
    float gainReduction = 0.0f;
    Orientation orientation = Orientation::Horizontal;
};

// VU-style needle meter for gain reduction
class VUMeter : public juce::Component
{
public:
    VUMeter()
    {
        setOpaque(false);
    }

    void setGainReduction(float grDb)
    {
        targetGR = grDb;
    }

    void updateAnimation()
    {
        // Smooth needle movement
        float diff = targetGR - currentGR;
        currentGR += diff * 0.15f;  // Ballistic response
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(4.0f);

        // Meter face background
        g.setColour(juce::Colour(0xff1a1a18));
        g.fillRoundedRectangle(bounds, 6.0f);

        // Scale arc
        float centerX = bounds.getCentreX();
        float centerY = bounds.getBottom() + 20.0f;
        float arcRadius = bounds.getWidth() * 0.45f;

        // Draw scale markings
        g.setColour(Colors::textDim);
        g.setFont(9.0f);

        const float startAngle = juce::MathConstants<float>::pi * 0.85f;
        const float endAngle = juce::MathConstants<float>::pi * 0.15f;

        // Scale marks: 0, -3, -6, -10, -15, -20
        const float dbValues[] = { 0.0f, 3.0f, 6.0f, 10.0f, 15.0f, 20.0f };
        const char* labels[] = { "0", "3", "6", "10", "15", "20" };

        for (int i = 0; i < 6; ++i)
        {
            float normalized = dbValues[i] / 20.0f;
            float angle = startAngle - normalized * (startAngle - endAngle);

            float tickInner = arcRadius - 8.0f;
            float tickOuter = arcRadius;

            float x1 = centerX + tickInner * std::cos(angle);
            float y1 = centerY - tickInner * std::sin(angle);
            float x2 = centerX + tickOuter * std::cos(angle);
            float y2 = centerY - tickOuter * std::sin(angle);

            g.drawLine(x1, y1, x2, y2, 1.0f);

            // Label
            float labelR = arcRadius + 12.0f;
            float lx = centerX + labelR * std::cos(angle);
            float ly = centerY - labelR * std::sin(angle);
            g.drawText(labels[i], juce::Rectangle<float>(lx - 12, ly - 6, 24, 12),
                       juce::Justification::centred);
        }

        // Needle
        float normalizedGR = juce::jlimit(0.0f, 1.0f, currentGR / 20.0f);
        float needleAngle = startAngle - normalizedGR * (startAngle - endAngle);
        float needleLength = arcRadius - 15.0f;

        float needleX = centerX + needleLength * std::cos(needleAngle);
        float needleY = centerY - needleLength * std::sin(needleAngle);

        // Needle shadow
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawLine(centerX + 1, centerY + 1, needleX + 1, needleY + 1, 3.0f);

        // Needle
        g.setColour(Colors::meterRed);
        g.drawLine(centerX, centerY, needleX, needleY, 2.0f);

        // Pivot point
        g.setColour(juce::Colour(0xff404040));
        g.fillEllipse(centerX - 5, centerY - 5, 10, 10);
    }

private:
    float targetGR = 0.0f;
    float currentGR = 0.0f;
};

// Stereo level meter pair
class StereoMeter : public juce::Component
{
public:
    StereoMeter()
    {
        addAndMakeVisible(leftMeter);
        addAndMakeVisible(rightMeter);
    }

    void setLevels(float left, float right)
    {
        leftMeter.setLevel(left);
        rightMeter.setLevel(right);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        int meterWidth = (bounds.getWidth() - 4) / 2;

        leftMeter.setBounds(bounds.removeFromLeft(meterWidth));
        bounds.removeFromLeft(4);
        rightMeter.setBounds(bounds);
    }

    void repaintMeters()
    {
        leftMeter.repaint();
        rightMeter.repaint();
    }

private:
    LevelMeter leftMeter;
    LevelMeter rightMeter;
};
