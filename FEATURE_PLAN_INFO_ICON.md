# Implementation Plan: Info Icon Feature for BusGlue Plugin

## Overview

Add a small info icon next to the bypass button that shows a frosted overlay with plugin description when clicked.

### User Interaction Flow
1. User sees info icon in header (right side, next to bypass button)
2. User clicks the icon
3. Frosted overlay fades in, covering the plugin UI
4. User reads the information
5. User clicks "X" button or clicks outside the overlay panel
6. Overlay fades out

---

## UI Design

### Icon Placement
- Position: 8 pixels left of the bypass button
- Location: `(getWidth() - 98, 6, 18, 18)`
- Size: 18x18 pixels (matches bypass button height)
- Style: Circle with "i" character, using accent color

### Overlay Window Design
- Size: 420x230 pixels (centered within plugin)
- Background: `Colors::panelBg` with alpha 0.97
- Border: `Colors::panelBorder`, 1.5px, rounded corners (8px)
- Close button: "X" in top-right corner

### Frosted Glass Effect
JUCE doesn't have native blur, so we simulate:
1. Semi-transparent dark overlay (`Colors::background.withAlpha(0.75f)`)
2. Info panel with slight transparency
3. Subtle shadow for depth

---

## Implementation Steps

### Step 1: Create InfoButton.h

**File:** `Source/UI/InfoButton.h`

```cpp
#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class InfoButton : public juce::Button
{
public:
    InfoButton() : juce::Button("Info")
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 1.0f;

        // Background on hover/press
        if (isMouseOver || isButtonDown)
        {
            g.setColour(Colors::accent.withAlpha(isButtonDown ? 0.4f : 0.25f));
            g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        }

        // Circle outline
        g.setColour(Colors::accent);
        g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        // "i" character
        g.setColour(Colors::textPrimary);
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText("i", bounds, juce::Justification::centred);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoButton)
};
```

### Step 2: Create InfoOverlay.h

**File:** `Source/UI/InfoOverlay.h`

```cpp
#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class InfoOverlay : public juce::Component
{
public:
    std::function<void()> onClose;

    InfoOverlay()
    {
        setOpaque(false);
        setWantsKeyboardFocus(true);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Semi-transparent background (simulated frosted glass)
        g.setColour(Colors::background.withAlpha(0.75f));
        g.fillRect(bounds);

        // Info panel dimensions
        float panelWidth = 420.0f;
        float panelHeight = 230.0f;
        auto panelBounds = juce::Rectangle<float>(
            (bounds.getWidth() - panelWidth) / 2.0f,
            (bounds.getHeight() - panelHeight) / 2.0f,
            panelWidth,
            panelHeight
        );

        // Panel shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(panelBounds.translated(3.0f, 3.0f), 8.0f);

        // Panel background
        g.setColour(Colors::panelBg.withAlpha(0.97f));
        g.fillRoundedRectangle(panelBounds, 8.0f);

        // Panel border
        g.setColour(Colors::panelBorder);
        g.drawRoundedRectangle(panelBounds, 8.0f, 1.5f);

        // Close button area (top-right of panel)
        auto closeRect = juce::Rectangle<float>(
            panelBounds.getRight() - 28.0f,
            panelBounds.getY() + 4.0f,
            24.0f, 24.0f
        );
        closeButtonBounds = closeRect.toNearestInt();

        g.setColour(Colors::textDim);
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText("×", closeRect, juce::Justification::centred);

        // Title
        g.setColour(Colors::accent);
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText("BUS GLUE", panelBounds.removeFromTop(32.0f).reduced(16.0f, 6.0f),
                   juce::Justification::centredLeft);

        // Content area
        auto contentArea = panelBounds.reduced(16.0f, 4.0f);

        g.setColour(Colors::textPrimary);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText("SSL/API-Style Bus Compressor", contentArea.removeFromTop(18.0f),
                   juce::Justification::centredLeft);

        contentArea.removeFromTop(8.0f);

        g.setColour(Colors::textSecondary);
        g.setFont(juce::Font(11.0f));

        juce::String description =
            "Bus Glue is a professional bus compressor designed to add cohesion "
            "and punch to your mix bus, drum groups, and instrument buses.\n\n"
            "Features:\n"
            "  • Stepped attack/release like classic SSL hardware\n"
            "  • Feed-forward and feedback topology options\n"
            "  • Four character modes: Clean, Punch, Warm, Aggressive\n"
            "  • Sidechain high-pass filter (20-300 Hz)\n"
            "  • Variable stereo linking (0-100%)\n"
            "  • Parallel compression mix control\n\n"
            "Tip: Start with 4:1 ratio, -10dB threshold, and adjust to taste.";

        g.drawFittedText(description, contentArea.toNearestInt(),
                         juce::Justification::topLeft, 12);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // Check if clicked on close button
        if (closeButtonBounds.contains(e.getPosition()))
        {
            if (onClose) onClose();
            return;
        }

        // Check if clicked outside panel (dismiss)
        float panelWidth = 420.0f;
        float panelHeight = 230.0f;
        auto panelBounds = juce::Rectangle<float>(
            (getWidth() - panelWidth) / 2.0f,
            (getHeight() - panelHeight) / 2.0f,
            panelWidth, panelHeight
        );

        if (!panelBounds.contains(e.getPosition().toFloat()))
        {
            if (onClose) onClose();
        }
    }

    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey)
        {
            if (onClose) onClose();
            return true;
        }
        return false;
    }

private:
    juce::Rectangle<int> closeButtonBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoOverlay)
};
```

### Step 3: Modify PluginEditor.h

Add includes and members:

```cpp
#include "UI/InfoButton.h"
#include "UI/InfoOverlay.h"

// In private section:
InfoButton infoButton;
std::unique_ptr<InfoOverlay> infoOverlay;
```

### Step 4: Modify PluginEditor.cpp Constructor

After bypass button setup:

```cpp
// Info button
infoButton.onClick = [this]()
{
    if (!infoOverlay)
    {
        infoOverlay = std::make_unique<InfoOverlay>();
        infoOverlay->onClose = [this]() { infoOverlay.reset(); repaint(); };
        addAndMakeVisible(*infoOverlay);
        infoOverlay->setBounds(getLocalBounds());
    }
};
addAndMakeVisible(infoButton);
```

### Step 5: Modify PluginEditor.cpp resized()

After `bypassButton.setBounds(...)`:

```cpp
infoButton.setBounds(getWidth() - 98, 6, 18, 18);

if (infoOverlay)
    infoOverlay->setBounds(getLocalBounds());
```

---

## Testing Checklist

### Visual Tests
- [ ] Info icon visible in header, positioned correctly
- [ ] Info icon has hover state (subtle green fill)
- [ ] Overlay covers full plugin area when open
- [ ] Panel is centered with proper shadow and border
- [ ] Close button (X) visible in top-right

### Interaction Tests
- [ ] Clicking info icon opens overlay
- [ ] Clicking close button dismisses overlay
- [ ] Clicking outside panel dismisses overlay
- [ ] Pressing Escape dismisses overlay
- [ ] Plugin remains functional after dismissing

### Build Verification
- [ ] Standalone builds without errors
- [ ] AU builds without errors
- [ ] VST3 builds without errors
