# BlindCard UI Redesign Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the existing BlindCard UI with a casino poker-themed design featuring dark/light themes, 3D chip buttons, animated poker cards, and responsive layout.

**Architecture:** Complete UI layer rewrite while preserving BlindCardManager core logic. New component hierarchy: BlindCardEditor → HeaderBar + ModeSelector + PokerTable + ControlPanel + ResultsPanel. Theme system via singleton ThemeManager broadcasting changes to all components.

**Tech Stack:** JUCE 7.x, C++17, CMake

**Design Reference:** `/Users/Mac/Projects/BlindCard/docs/plans/2026-01-19-blindcard-ui-redesign.md`

**Frontend Reference:** `/Users/Mac/Projects/blind-card-plugin-ui-ux` (React/Next.js design source)

---

## Phase 1: Theme System Foundation

### Task 1: Create ThemeColors.h

**Files:**
- Create: `Source/UI/Theme/ThemeColors.h`

**Step 1: Create directory structure**

```bash
mkdir -p Source/UI/Theme
```

**Step 2: Create ThemeColors.h with all color constants**

```cpp
// Source/UI/Theme/ThemeColors.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace BlindCard {
namespace Theme {

// =============================================================================
// Dark Theme Colors
// =============================================================================
namespace Dark {
    // Backgrounds
    inline const juce::Colour Background      { 0xFF0A0A0A };
    inline const juce::Colour Surface         { 0xFF1F1F1F };
    inline const juce::Colour SurfaceAlt      { 0xFF252525 };

    // Primary Colors
    inline const juce::Colour Primary         { 0xFFFF3B4E };
    inline const juce::Colour PrimaryHover    { 0xFFFF5A6A };
    inline const juce::Colour Accent          { 0xFFFFD700 };
    inline const juce::Colour AccentMuted     { 0xFFD4AF37 };

    // Text
    inline const juce::Colour TextPrimary     { 0xFFF5F5F0 };
    inline const juce::Colour TextSecondary   { 0xFF9CA3AF };
    inline const juce::Colour TextMuted       { 0xFF6B7280 };

    // Poker Table
    inline const juce::Colour TableFelt       { 0xFF1B8B6A };
    inline const juce::Colour TableFeltCenter { 0xFF22A77D };
    inline const juce::Colour TableWoodLight  { 0xFFD4A574 };
    inline const juce::Colour TableWoodDark   { 0xFF6B4F38 };
    inline const juce::Colour TableRail       { 0xFF8B5A3C };
    inline const juce::Colour NeonRed         { 0xFFFF3B4E };

    // Status
    inline const juce::Colour Success         { 0xFF22C55E };
    inline const juce::Colour Error           { 0xFFEF4444 };

    // Components
    inline const juce::Colour SliderTrack     { 0xFF333333 };
    inline const juce::Colour SliderFill      { 0xFFFF3B4E };
    inline const juce::Colour ToggleOff       { 0xFF333333 };
    inline const juce::Colour ToggleOn        { 0xFFFF3B4E };
    inline const juce::Colour CardFront       { 0xFFF5F5F0 };
    inline const juce::Colour StarEmpty       { 0xFF4A4A4A };
    inline const juce::Colour StarFilled      { 0xFFFFD700 };
}

// =============================================================================
// Light Theme Colors
// =============================================================================
namespace Light {
    // Backgrounds
    inline const juce::Colour Background      { 0xFFF5F0E8 };
    inline const juce::Colour Surface         { 0xFFFFFFFF };
    inline const juce::Colour SurfaceAlt      { 0xFFF0F0F0 };

    // Primary Colors
    inline const juce::Colour Primary         { 0xFFC41E3A };
    inline const juce::Colour PrimaryHover    { 0xFFD42E4A };
    inline const juce::Colour Accent          { 0xFFD4AF37 };
    inline const juce::Colour AccentMuted     { 0xFFB8962E };

    // Text
    inline const juce::Colour TextPrimary     { 0xFF1A1A1A };
    inline const juce::Colour TextSecondary   { 0xFF6B7280 };
    inline const juce::Colour TextMuted       { 0xFF9CA3AF };

    // Poker Table
    inline const juce::Colour TableFelt       { 0xFF2D9B7A };
    inline const juce::Colour TableFeltCenter { 0xFF3DAB8A };
    inline const juce::Colour TableWoodLight  { 0xFFD4A574 };
    inline const juce::Colour TableWoodDark   { 0xFF6B4F38 };
    inline const juce::Colour TableRail       { 0xFF8B5A3C };
    inline const juce::Colour NeonRed         { 0xFFC41E3A };

    // Status
    inline const juce::Colour Success         { 0xFF16A34A };
    inline const juce::Colour Error           { 0xFFDC2626 };

    // Components
    inline const juce::Colour SliderTrack     { 0xFFE5E5E5 };
    inline const juce::Colour SliderFill      { 0xFFC41E3A };
    inline const juce::Colour ToggleOff       { 0xFFD1D5DB };
    inline const juce::Colour ToggleOn        { 0xFFC41E3A };
    inline const juce::Colour CardFront       { 0xFFFFFFFF };
    inline const juce::Colour StarEmpty       { 0xFFD1D5DB };
    inline const juce::Colour StarFilled      { 0xFFD4AF37 };
}

} // namespace Theme
} // namespace BlindCard
```

**Step 3: Verify file compiles**

```bash
# Add to CMakeLists.txt temporarily to verify syntax
# Or just check the header is valid C++
```

**Step 4: Commit**

```bash
git add Source/UI/Theme/ThemeColors.h
git commit -m "feat(theme): add ThemeColors.h with dark/light color constants"
```

---

### Task 2: Create ThemeManager

**Files:**
- Create: `Source/UI/Theme/ThemeManager.h`
- Create: `Source/UI/Theme/ThemeManager.cpp`

**Step 1: Create ThemeManager.h**

```cpp
// Source/UI/Theme/ThemeManager.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemeColors.h"

namespace BlindCard {

enum class ColourId {
    Background,
    Surface,
    SurfaceAlt,
    Primary,
    PrimaryHover,
    Accent,
    AccentMuted,
    TextPrimary,
    TextSecondary,
    TextMuted,
    TableFelt,
    TableFeltCenter,
    TableWoodLight,
    TableWoodDark,
    TableRail,
    NeonRed,
    Success,
    Error,
    SliderTrack,
    SliderFill,
    ToggleOff,
    ToggleOn,
    CardFront,
    StarEmpty,
    StarFilled
};

class ThemeManager : public juce::ChangeBroadcaster {
public:
    enum class Theme { Dark, Light };

    static ThemeManager& getInstance();

    void setTheme(Theme theme);
    Theme getTheme() const { return currentTheme; }
    void toggleTheme();
    bool isDark() const { return currentTheme == Theme::Dark; }

    juce::Colour getColour(ColourId id) const;

    // Prevent copying
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

private:
    ThemeManager() = default;
    Theme currentTheme = Theme::Dark;
};

} // namespace BlindCard
```

**Step 2: Create ThemeManager.cpp**

```cpp
// Source/UI/Theme/ThemeManager.cpp
#include "ThemeManager.h"

namespace BlindCard {

ThemeManager& ThemeManager::getInstance() {
    static ThemeManager instance;
    return instance;
}

void ThemeManager::setTheme(Theme theme) {
    if (currentTheme != theme) {
        currentTheme = theme;
        sendChangeMessage();
    }
}

void ThemeManager::toggleTheme() {
    setTheme(currentTheme == Theme::Dark ? Theme::Light : Theme::Dark);
}

juce::Colour ThemeManager::getColour(ColourId id) const {
    using namespace Theme;

    if (currentTheme == Theme::Dark) {
        switch (id) {
            case ColourId::Background:      return Dark::Background;
            case ColourId::Surface:         return Dark::Surface;
            case ColourId::SurfaceAlt:      return Dark::SurfaceAlt;
            case ColourId::Primary:         return Dark::Primary;
            case ColourId::PrimaryHover:    return Dark::PrimaryHover;
            case ColourId::Accent:          return Dark::Accent;
            case ColourId::AccentMuted:     return Dark::AccentMuted;
            case ColourId::TextPrimary:     return Dark::TextPrimary;
            case ColourId::TextSecondary:   return Dark::TextSecondary;
            case ColourId::TextMuted:       return Dark::TextMuted;
            case ColourId::TableFelt:       return Dark::TableFelt;
            case ColourId::TableFeltCenter: return Dark::TableFeltCenter;
            case ColourId::TableWoodLight:  return Dark::TableWoodLight;
            case ColourId::TableWoodDark:   return Dark::TableWoodDark;
            case ColourId::TableRail:       return Dark::TableRail;
            case ColourId::NeonRed:         return Dark::NeonRed;
            case ColourId::Success:         return Dark::Success;
            case ColourId::Error:           return Dark::Error;
            case ColourId::SliderTrack:     return Dark::SliderTrack;
            case ColourId::SliderFill:      return Dark::SliderFill;
            case ColourId::ToggleOff:       return Dark::ToggleOff;
            case ColourId::ToggleOn:        return Dark::ToggleOn;
            case ColourId::CardFront:       return Dark::CardFront;
            case ColourId::StarEmpty:       return Dark::StarEmpty;
            case ColourId::StarFilled:      return Dark::StarFilled;
        }
    } else {
        switch (id) {
            case ColourId::Background:      return Light::Background;
            case ColourId::Surface:         return Light::Surface;
            case ColourId::SurfaceAlt:      return Light::SurfaceAlt;
            case ColourId::Primary:         return Light::Primary;
            case ColourId::PrimaryHover:    return Light::PrimaryHover;
            case ColourId::Accent:          return Light::Accent;
            case ColourId::AccentMuted:     return Light::AccentMuted;
            case ColourId::TextPrimary:     return Light::TextPrimary;
            case ColourId::TextSecondary:   return Light::TextSecondary;
            case ColourId::TextMuted:       return Light::TextMuted;
            case ColourId::TableFelt:       return Light::TableFelt;
            case ColourId::TableFeltCenter: return Light::TableFeltCenter;
            case ColourId::TableWoodLight:  return Light::TableWoodLight;
            case ColourId::TableWoodDark:   return Light::TableWoodDark;
            case ColourId::TableRail:       return Light::TableRail;
            case ColourId::NeonRed:         return Light::NeonRed;
            case ColourId::Success:         return Light::Success;
            case ColourId::Error:           return Light::Error;
            case ColourId::SliderTrack:     return Light::SliderTrack;
            case ColourId::SliderFill:      return Light::SliderFill;
            case ColourId::ToggleOff:       return Light::ToggleOff;
            case ColourId::ToggleOn:        return Light::ToggleOn;
            case ColourId::CardFront:       return Light::CardFront;
            case ColourId::StarEmpty:       return Light::StarEmpty;
            case ColourId::StarFilled:      return Light::StarFilled;
        }
    }

    return juce::Colours::magenta; // Fallback - should never happen
}

} // namespace BlindCard
```

**Step 3: Commit**

```bash
git add Source/UI/Theme/ThemeManager.h Source/UI/Theme/ThemeManager.cpp
git commit -m "feat(theme): add ThemeManager singleton with dark/light support"
```

---

### Task 3: Create AnimatedValue utility

**Files:**
- Create: `Source/UI/Animation/AnimatedValue.h`

**Step 1: Create directory and file**

```bash
mkdir -p Source/UI/Animation
```

**Step 2: Create AnimatedValue.h (extract from existing ColorUtils.h)**

```cpp
// Source/UI/Animation/AnimatedValue.h
#pragma once
#include <cmath>

namespace BlindCard {

enum class EasingType {
    Linear,
    EaseOut,
    EaseInOut
};

class AnimatedValue {
public:
    AnimatedValue(float initialValue = 0.0f)
        : current(initialValue), target(initialValue) {}

    void setTarget(float newTarget, float durationMs, EasingType easing = EasingType::EaseOut) {
        if (std::abs(target - newTarget) > 0.001f) {
            start = current;
            target = newTarget;
            duration = durationMs;
            elapsed = 0.0f;
            easingType = easing;
        }
    }

    void setImmediate(float value) {
        current = target = start = value;
        elapsed = duration;
    }

    void update(float deltaMs) {
        if (isComplete()) return;

        elapsed += deltaMs;
        float t = std::min(elapsed / duration, 1.0f);
        float easedT = applyEasing(t);
        current = start + (target - start) * easedT;
    }

    bool isComplete() const {
        return elapsed >= duration;
    }

    float getValue() const { return current; }
    float getTarget() const { return target; }

private:
    float applyEasing(float t) const {
        switch (easingType) {
            case EasingType::Linear:
                return t;
            case EasingType::EaseOut:
                return 1.0f - std::pow(1.0f - t, 3.0f);
            case EasingType::EaseInOut:
                return t < 0.5f
                    ? 4.0f * t * t * t
                    : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
        }
        return t;
    }

    float current = 0.0f;
    float target = 0.0f;
    float start = 0.0f;
    float duration = 1.0f;
    float elapsed = 0.0f;
    EasingType easingType = EasingType::EaseOut;
};

} // namespace BlindCard
```

**Step 3: Commit**

```bash
git add Source/UI/Animation/AnimatedValue.h
git commit -m "feat(animation): add AnimatedValue utility class with easing"
```

---

## Phase 2: Core Components

### Task 4: Create HeaderBar component

**Files:**
- Create: `Source/UI/Components/HeaderBar.h`
- Create: `Source/UI/Components/HeaderBar.cpp`

**Step 1: Create directory**

```bash
mkdir -p Source/UI/Components
```

**Step 2: Create HeaderBar.h**

```cpp
// Source/UI/Components/HeaderBar.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/ThemeManager.h"
#include "../Animation/AnimatedValue.h"

namespace BlindCard {

class HeaderBar : public juce::Component,
                  public juce::ChangeListener,
                  public juce::Timer {
public:
    HeaderBar();
    ~HeaderBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setConnected(bool connected);
    bool isConnected() const { return connected; }

    std::function<void()> onSettingsClicked;
    std::function<void()> onInfoClicked;
    std::function<void()> onThemeToggled;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;

    void drawLogo(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawConnectionStatus(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawThemeToggle(juce::Graphics& g, juce::Rectangle<float> bounds);

    bool connected = false;
    AnimatedValue pulseAnimation;
    float pulsePhase = 0.0f;

    juce::Rectangle<int> themeToggleBounds;
    juce::Rectangle<int> infoBounds;
    juce::Rectangle<int> settingsBounds;

    bool themeToggleHovered = false;
    bool infoHovered = false;
    bool settingsHovered = false;

    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};

} // namespace BlindCard
```

**Step 3: Create HeaderBar.cpp**

```cpp
// Source/UI/Components/HeaderBar.cpp
#include "HeaderBar.h"

namespace BlindCard {

HeaderBar::HeaderBar() {
    ThemeManager::getInstance().addChangeListener(this);
    startTimerHz(60);
}

HeaderBar::~HeaderBar() {
    ThemeManager::getInstance().removeChangeListener(this);
    stopTimer();
}

void HeaderBar::changeListenerCallback(juce::ChangeBroadcaster*) {
    repaint();
}

void HeaderBar::timerCallback() {
    pulsePhase += 0.04f;
    if (pulsePhase > 1.0f) pulsePhase -= 1.0f;

    if (connected) {
        repaint();
    }
}

void HeaderBar::setConnected(bool isConnected) {
    connected = isConnected;
    repaint();
}

void HeaderBar::paint(juce::Graphics& g) {
    auto& tm = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(tm.getColour(ColourId::Background));
    g.fillRect(bounds);

    // Bottom border
    g.setColour(tm.getColour(ColourId::Surface));
    g.fillRect(bounds.removeFromBottom(1.0f));

    // Logo area (left)
    auto logoArea = bounds.removeFromLeft(200.0f);
    drawLogo(g, logoArea.reduced(16.0f, 8.0f));

    // Right side buttons
    auto rightArea = bounds.removeFromRight(150.0f).reduced(16.0f, 12.0f);

    // Settings button (rightmost)
    auto settingsArea = rightArea.removeFromRight(32.0f);
    settingsBounds = settingsArea.toNearestInt();
    g.setColour(settingsHovered ? tm.getColour(ColourId::TextPrimary) : tm.getColour(ColourId::TextSecondary));
    g.drawText(juce::String::fromUTF8("⚙"), settingsArea, juce::Justification::centred);

    rightArea.removeFromRight(8.0f);

    // Info button
    auto infoArea = rightArea.removeFromRight(32.0f);
    infoBounds = infoArea.toNearestInt();
    g.setColour(infoHovered ? tm.getColour(ColourId::TextPrimary) : tm.getColour(ColourId::TextSecondary));
    g.drawText(juce::String::fromUTF8("ℹ"), infoArea, juce::Justification::centred);

    rightArea.removeFromRight(8.0f);

    // Theme toggle
    auto toggleArea = rightArea.removeFromRight(44.0f);
    themeToggleBounds = toggleArea.toNearestInt();
    drawThemeToggle(g, toggleArea);

    // Connection status (center)
    auto centerArea = bounds.withSizeKeepingCentre(140.0f, 32.0f);
    drawConnectionStatus(g, centerArea);
}

void HeaderBar::drawLogo(juce::Graphics& g, juce::Rectangle<float> bounds) {
    auto& tm = ThemeManager::getInstance();

    // Logo icon (red rounded rect with spade)
    auto iconBounds = bounds.removeFromLeft(40.0f).reduced(0.0f, 4.0f);

    g.setGradientFill(juce::ColourGradient(
        tm.getColour(ColourId::Primary).brighter(0.1f),
        iconBounds.getTopLeft(),
        tm.getColour(ColourId::Primary).darker(0.2f),
        iconBounds.getBottomRight(),
        false
    ));
    g.fillRoundedRectangle(iconBounds, 8.0f);

    // Spade symbol
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f));
    g.drawText(juce::String::fromUTF8("♠"), iconBounds, juce::Justification::centred);

    bounds.removeFromLeft(12.0f);

    // Title
    g.setColour(tm.getColour(ColourId::Primary));
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("BlindCard", bounds.removeFromTop(24.0f), juce::Justification::centredLeft);

    // Subtitle
    g.setColour(tm.getColour(ColourId::TextMuted));
    g.setFont(juce::Font(10.0f));
    g.drawText("BY SUGOI AUDIO", bounds, juce::Justification::centredLeft);
}

void HeaderBar::drawConnectionStatus(juce::Graphics& g, juce::Rectangle<float> bounds) {
    auto& tm = ThemeManager::getInstance();

    // Capsule background
    g.setColour(tm.getColour(ColourId::Surface));
    g.fillRoundedRectangle(bounds, bounds.getHeight() / 2.0f);

    // Content
    auto contentBounds = bounds.reduced(12.0f, 6.0f);

    // Headphone icon
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(14.0f));
    g.drawText(juce::String::fromUTF8("🎧"), contentBounds.removeFromLeft(20.0f), juce::Justification::centred);

    contentBounds.removeFromLeft(6.0f);

    // Text
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(12.0f));
    g.drawText(connected ? "Connected" : "No Connection",
               contentBounds.removeFromLeft(80.0f),
               juce::Justification::centredLeft);

    // Status dot with pulse
    auto dotBounds = contentBounds.removeFromRight(12.0f).withSizeKeepingCentre(8.0f, 8.0f);

    if (connected) {
        // Pulse effect
        float pulse = 0.5f + 0.5f * std::sin(pulsePhase * juce::MathConstants<float>::twoPi);
        auto glowColour = tm.getColour(ColourId::Accent).withAlpha(0.3f * pulse);
        g.setColour(glowColour);
        g.fillEllipse(dotBounds.expanded(4.0f * pulse));

        g.setColour(tm.getColour(ColourId::Accent));
    } else {
        g.setColour(tm.getColour(ColourId::TextMuted));
    }
    g.fillEllipse(dotBounds);
}

void HeaderBar::drawThemeToggle(juce::Graphics& g, juce::Rectangle<float> bounds) {
    auto& tm = ThemeManager::getInstance();
    bool isDark = tm.isDark();

    // Track
    auto trackBounds = bounds.withSizeKeepingCentre(44.0f, 24.0f);
    g.setColour(isDark ? tm.getColour(ColourId::ToggleOn) : tm.getColour(ColourId::ToggleOff));
    g.fillRoundedRectangle(trackBounds, 12.0f);

    // Glow effect when on (dark mode)
    if (isDark) {
        g.setColour(tm.getColour(ColourId::Primary).withAlpha(0.3f));
        g.drawRoundedRectangle(trackBounds.expanded(2.0f), 14.0f, 2.0f);
    }

    // Knob
    float knobX = isDark ? trackBounds.getRight() - 22.0f : trackBounds.getX() + 2.0f;
    auto knobBounds = juce::Rectangle<float>(knobX, trackBounds.getY() + 2.0f, 20.0f, 20.0f);

    g.setColour(juce::Colours::white);
    g.fillEllipse(knobBounds);

    // Icon
    g.setColour(isDark ? tm.getColour(ColourId::Primary) : tm.getColour(ColourId::Accent));
    g.setFont(juce::Font(12.0f));
    g.drawText(isDark ? juce::String::fromUTF8("🌙") : juce::String::fromUTF8("☀"),
               knobBounds, juce::Justification::centred);
}

void HeaderBar::resized() {
    // Bounds calculated in paint()
}

void HeaderBar::mouseMove(const juce::MouseEvent& e) {
    bool needsRepaint = false;

    bool newThemeHover = themeToggleBounds.contains(e.getPosition());
    if (newThemeHover != themeToggleHovered) {
        themeToggleHovered = newThemeHover;
        needsRepaint = true;
    }

    bool newInfoHover = infoBounds.contains(e.getPosition());
    if (newInfoHover != infoHovered) {
        infoHovered = newInfoHover;
        needsRepaint = true;
    }

    bool newSettingsHover = settingsBounds.contains(e.getPosition());
    if (newSettingsHover != settingsHovered) {
        settingsHovered = newSettingsHover;
        needsRepaint = true;
    }

    if (needsRepaint) repaint();
}

void HeaderBar::mouseDown(const juce::MouseEvent& e) {
    if (themeToggleBounds.contains(e.getPosition())) {
        ThemeManager::getInstance().toggleTheme();
        if (onThemeToggled) onThemeToggled();
    }
    else if (infoBounds.contains(e.getPosition())) {
        if (onInfoClicked) onInfoClicked();
    }
    else if (settingsBounds.contains(e.getPosition())) {
        if (onSettingsClicked) onSettingsClicked();
    }
}

void HeaderBar::mouseExit(const juce::MouseEvent&) {
    themeToggleHovered = false;
    infoHovered = false;
    settingsHovered = false;
    repaint();
}

} // namespace BlindCard
```

**Step 4: Commit**

```bash
git add Source/UI/Components/HeaderBar.h Source/UI/Components/HeaderBar.cpp
git commit -m "feat(ui): add HeaderBar component with logo, connection status, theme toggle"
```

---

### Task 5: Create ModeSelector component

**Files:**
- Create: `Source/UI/Components/ModeSelector.h`
- Create: `Source/UI/Components/ModeSelector.cpp`

**Step 1: Create ModeSelector.h**

```cpp
// Source/UI/Components/ModeSelector.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/ThemeManager.h"
#include "../../Core/Types.h"

namespace BlindCard {

class ModeSelector : public juce::Component,
                     public juce::ChangeListener {
public:
    ModeSelector();
    ~ModeSelector() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMode(RatingMode mode);
    RatingMode getMode() const { return currentMode; }

    void setLocked(bool locked);
    bool isLocked() const { return isLockedState; }

    std::function<void(RatingMode)> onModeChanged;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    RatingMode currentMode = RatingMode::Stars;
    bool isLockedState = false;
    int hoveredIndex = -1;

    struct ModeOption {
        RatingMode mode;
        juce::String icon;
        juce::String label;
        juce::Rectangle<int> bounds;
    };

    std::vector<ModeOption> options;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelector)
};

} // namespace BlindCard
```

**Step 2: Create ModeSelector.cpp**

```cpp
// Source/UI/Components/ModeSelector.cpp
#include "ModeSelector.h"

namespace BlindCard {

ModeSelector::ModeSelector() {
    options = {
        { RatingMode::Stars, juce::String::fromUTF8("☆"), "Stars", {} },
        { RatingMode::Guess, juce::String::fromUTF8("◎"), "Guess", {} },
        { RatingMode::QA,    juce::String::fromUTF8("✎"), "Q&A",   {} }
    };

    ThemeManager::getInstance().addChangeListener(this);
}

ModeSelector::~ModeSelector() {
    ThemeManager::getInstance().removeChangeListener(this);
}

void ModeSelector::changeListenerCallback(juce::ChangeBroadcaster*) {
    repaint();
}

void ModeSelector::setMode(RatingMode mode) {
    if (currentMode != mode && !isLockedState) {
        currentMode = mode;
        repaint();
        if (onModeChanged) onModeChanged(mode);
    }
}

void ModeSelector::setLocked(bool locked) {
    isLockedState = locked;
    repaint();
}

void ModeSelector::paint(juce::Graphics& g) {
    auto& tm = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Outer container
    g.setColour(tm.getColour(ColourId::Surface));
    g.fillRoundedRectangle(bounds, 20.0f);

    g.setColour(tm.getColour(ColourId::SurfaceAlt));
    g.drawRoundedRectangle(bounds, 20.0f, 1.0f);

    // Draw each option
    for (size_t i = 0; i < options.size(); ++i) {
        auto& opt = options[i];
        auto optBounds = opt.bounds.toFloat();

        bool isSelected = (opt.mode == currentMode);
        bool isHovered = (static_cast<int>(i) == hoveredIndex) && !isLockedState;

        // Background
        if (isSelected) {
            g.setColour(tm.getColour(ColourId::Primary).withAlpha(0.15f));
            g.fillRoundedRectangle(optBounds.reduced(2.0f), 16.0f);

            g.setColour(tm.getColour(ColourId::Primary));
            g.drawRoundedRectangle(optBounds.reduced(2.0f), 16.0f, 1.0f);
        } else if (isHovered) {
            g.setColour(juce::Colours::white.withAlpha(0.05f));
            g.fillRoundedRectangle(optBounds.reduced(2.0f), 16.0f);
        }

        // Icon and text
        auto contentBounds = optBounds.reduced(12.0f, 0.0f);

        g.setColour(isSelected ? tm.getColour(ColourId::Primary) : tm.getColour(ColourId::TextSecondary));
        g.setFont(juce::Font(14.0f));

        auto iconBounds = contentBounds.removeFromLeft(20.0f);
        g.drawText(opt.icon, iconBounds, juce::Justification::centred);

        contentBounds.removeFromLeft(4.0f);
        g.setFont(juce::Font(13.0f, juce::Font::FontStyleFlags::plain));
        g.drawText(opt.label, contentBounds, juce::Justification::centredLeft);

        // Lock icon if locked and not selected
        if (isLockedState && !isSelected) {
            g.setColour(tm.getColour(ColourId::TextMuted));
            g.setFont(juce::Font(10.0f));
            g.drawText(juce::String::fromUTF8("🔒"),
                       optBounds.removeFromRight(24.0f),
                       juce::Justification::centred);
        }
    }
}

void ModeSelector::resized() {
    auto bounds = getLocalBounds();
    int optionWidth = bounds.getWidth() / static_cast<int>(options.size());

    for (size_t i = 0; i < options.size(); ++i) {
        options[i].bounds = juce::Rectangle<int>(
            bounds.getX() + static_cast<int>(i) * optionWidth,
            bounds.getY(),
            optionWidth,
            bounds.getHeight()
        );
    }
}

void ModeSelector::mouseMove(const juce::MouseEvent& e) {
    int newHovered = -1;

    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].bounds.contains(e.getPosition())) {
            newHovered = static_cast<int>(i);
            break;
        }
    }

    if (newHovered != hoveredIndex) {
        hoveredIndex = newHovered;
        repaint();
    }
}

void ModeSelector::mouseDown(const juce::MouseEvent& e) {
    if (isLockedState) return;

    for (auto& opt : options) {
        if (opt.bounds.contains(e.getPosition())) {
            setMode(opt.mode);
            break;
        }
    }
}

void ModeSelector::mouseExit(const juce::MouseEvent&) {
    hoveredIndex = -1;
    repaint();
}

} // namespace BlindCard
```

**Step 3: Commit**

```bash
git add Source/UI/Components/ModeSelector.h Source/UI/Components/ModeSelector.cpp
git commit -m "feat(ui): add ModeSelector component with Stars/Guess/Q&A modes"
```

---

### Task 6: Create ChipButton component

**Files:**
- Create: `Source/UI/Components/ChipButton.h`
- Create: `Source/UI/Components/ChipButton.cpp`

**Step 1: Create ChipButton.h**

```cpp
// Source/UI/Components/ChipButton.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/ThemeManager.h"
#include "../Animation/AnimatedValue.h"

namespace BlindCard {

class ChipButton : public juce::Component,
                   public juce::ChangeListener,
                   public juce::Timer {
public:
    enum class Variant { Red, Gold, Black, Blue };

    ChipButton(const juce::String& label, Variant variant = Variant::Red);
    ~ChipButton() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setEnabled(bool enabled);
    void setLabel(const juce::String& label);

    std::function<void()> onClick;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    juce::Colour getBaseColour() const;
    juce::Colour getBaseColourDark() const;

    juce::String labelText;
    Variant chipVariant;
    bool isEnabled = true;
    bool isHovered = false;
    bool isPressed = false;

    AnimatedValue hoverAnimation;
    AnimatedValue pressAnimation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChipButton)
};

} // namespace BlindCard
```

**Step 2: Create ChipButton.cpp**

```cpp
// Source/UI/Components/ChipButton.cpp
#include "ChipButton.h"

namespace BlindCard {

ChipButton::ChipButton(const juce::String& label, Variant variant)
    : labelText(label), chipVariant(variant)
{
    ThemeManager::getInstance().addChangeListener(this);
}

ChipButton::~ChipButton() {
    ThemeManager::getInstance().removeChangeListener(this);
    stopTimer();
}

void ChipButton::changeListenerCallback(juce::ChangeBroadcaster*) {
    repaint();
}

void ChipButton::timerCallback() {
    hoverAnimation.update(16.67f);
    pressAnimation.update(16.67f);

    repaint();

    if (hoverAnimation.isComplete() && pressAnimation.isComplete()) {
        stopTimer();
    }
}

void ChipButton::setEnabled(bool enabled) {
    isEnabled = enabled;
    repaint();
}

void ChipButton::setLabel(const juce::String& label) {
    labelText = label;
    repaint();
}

juce::Colour ChipButton::getBaseColour() const {
    switch (chipVariant) {
        case Variant::Red:   return juce::Colour(0xFFCC0000);
        case Variant::Gold:  return juce::Colour(0xFFDAA520);
        case Variant::Black: return juce::Colour(0xFF333333);
        case Variant::Blue:  return juce::Colour(0xFF2E6EB0);
    }
    return juce::Colour(0xFFCC0000);
}

juce::Colour ChipButton::getBaseColourDark() const {
    switch (chipVariant) {
        case Variant::Red:   return juce::Colour(0xFF8B0000);
        case Variant::Gold:  return juce::Colour(0xFF8B7300);
        case Variant::Black: return juce::Colour(0xFF1A1A1A);
        case Variant::Blue:  return juce::Colour(0xFF1A4A7A);
    }
    return juce::Colour(0xFF8B0000);
}

void ChipButton::paint(juce::Graphics& g) {
    auto& tm = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Calculate chip area (square, centered)
    float chipSize = std::min(bounds.getWidth(), bounds.getHeight() - 20.0f);
    auto chipBounds = bounds.removeFromTop(chipSize).withSizeKeepingCentre(chipSize, chipSize);

    float alpha = isEnabled ? 1.0f : 0.4f;
    float hoverValue = hoverAnimation.getValue();
    float pressValue = pressAnimation.getValue();

    // Hover glow
    if (hoverValue > 0.0f && isEnabled) {
        auto glowColour = getBaseColour().withAlpha(0.4f * hoverValue);
        for (int i = 3; i >= 0; --i) {
            g.setColour(glowColour.withMultipliedAlpha(1.0f - i * 0.25f));
            g.fillEllipse(chipBounds.expanded(4.0f * (i + 1) * hoverValue));
        }
    }

    // Scale for press
    float scale = 1.0f - 0.05f * pressValue;
    auto scaledChipBounds = chipBounds.withSizeKeepingCentre(chipSize * scale, chipSize * scale);

    // Translate up for hover
    scaledChipBounds.translate(0.0f, -2.0f * hoverValue);

    // Outer ring gradient
    g.setGradientFill(juce::ColourGradient(
        getBaseColour().withAlpha(alpha),
        scaledChipBounds.getTopLeft(),
        getBaseColourDark().withAlpha(alpha),
        scaledChipBounds.getBottomRight(),
        false
    ));
    g.fillEllipse(scaledChipBounds);

    // Stripe decorations (12 stripes)
    g.setColour(juce::Colour(0xFFF5F5DC).withAlpha(alpha * 0.9f));
    auto center = scaledChipBounds.getCentre();
    float outerRadius = scaledChipBounds.getWidth() / 2.0f;
    float innerRadius = outerRadius * 0.8f;

    for (int i = 0; i < 12; ++i) {
        float angle = i * juce::MathConstants<float>::twoPi / 12.0f;
        float stripeWidth = 4.0f * scale;

        juce::Path stripe;
        float cos_a = std::cos(angle);
        float sin_a = std::sin(angle);

        stripe.addRectangle(-stripeWidth / 2.0f, -outerRadius, stripeWidth, outerRadius - innerRadius);
        stripe.applyTransform(juce::AffineTransform::rotation(angle).translated(center.x, center.y));

        g.fillPath(stripe);
    }

    // Middle black ring
    float middleRadius = outerRadius * 0.75f;
    auto middleBounds = scaledChipBounds.withSizeKeepingCentre(middleRadius * 2.0f, middleRadius * 2.0f);

    g.setColour(juce::Colours::black.withAlpha(alpha));
    g.fillEllipse(middleBounds);

    // Gold border on middle ring
    g.setColour(tm.getColour(ColourId::Accent).withAlpha(alpha));
    g.drawEllipse(middleBounds, 1.5f);

    // Inner circle
    float innerCircleRadius = outerRadius * 0.5f;
    auto innerBounds = scaledChipBounds.withSizeKeepingCentre(innerCircleRadius * 2.0f, innerCircleRadius * 2.0f);

    g.setGradientFill(juce::ColourGradient(
        getBaseColourDark().brighter(0.2f).withAlpha(alpha),
        innerBounds.getTopLeft(),
        getBaseColourDark().withAlpha(alpha),
        innerBounds.getBottomRight(),
        false
    ));
    g.fillEllipse(innerBounds);

    // Icon (placeholder - would use actual icons)
    g.setColour(juce::Colours::white.withAlpha(alpha));
    g.setFont(juce::Font(20.0f * scale));

    juce::String icon;
    if (labelText == "SHUFFLE") icon = juce::String::fromUTF8("⟳");
    else if (labelText == "RESET") icon = juce::String::fromUTF8("↺");
    else if (labelText == "REVEAL") icon = juce::String::fromUTF8("👁");
    else if (labelText == "NEXT") icon = juce::String::fromUTF8("→");
    else icon = labelText.substring(0, 1);

    g.drawText(icon, innerBounds, juce::Justification::centred);

    // Label below chip
    auto labelBounds = getLocalBounds().toFloat().removeFromBottom(20.0f);
    g.setColour(tm.getColour(ColourId::TextSecondary).withAlpha(alpha));
    g.setFont(juce::Font(11.0f));
    g.drawText(labelText, labelBounds, juce::Justification::centred);
}

void ChipButton::resized() {
    // Nothing specific needed
}

void ChipButton::mouseEnter(const juce::MouseEvent&) {
    if (!isEnabled) return;
    isHovered = true;
    hoverAnimation.setTarget(1.0f, 150.0f);
    startTimerHz(60);
}

void ChipButton::mouseExit(const juce::MouseEvent&) {
    isHovered = false;
    hoverAnimation.setTarget(0.0f, 150.0f);
    startTimerHz(60);
}

void ChipButton::mouseDown(const juce::MouseEvent&) {
    if (!isEnabled) return;
    isPressed = true;
    pressAnimation.setTarget(1.0f, 100.0f);
    startTimerHz(60);
}

void ChipButton::mouseUp(const juce::MouseEvent& e) {
    if (!isEnabled) return;
    isPressed = false;
    pressAnimation.setTarget(0.0f, 100.0f);
    startTimerHz(60);

    if (getLocalBounds().contains(e.getPosition()) && onClick) {
        onClick();
    }
}

} // namespace BlindCard
```

**Step 3: Commit**

```bash
git add Source/UI/Components/ChipButton.h Source/UI/Components/ChipButton.cpp
git commit -m "feat(ui): add ChipButton component with 3D poker chip design"
```

---

## Phase 3: Continue with remaining components...

> **Note:** The remaining tasks follow the same pattern. For brevity, the full implementation code is provided in summary form. Each task should be implemented and committed separately.

### Task 7: Create StarRating component
- Files: `Source/UI/Components/StarRating.h/.cpp`
- 5 clickable stars with fill animation
- Gold (#FFD700) filled, gray (#4A4A4A) empty

### Task 8: Create GuessDropdown component
- Files: `Source/UI/Components/GuessDropdown.h/.cpp`
- Dropdown with track list
- Dark theme styling

### Task 9: Create PokerCard component
- Files: `Source/UI/Components/PokerCard.h/.cpp`
- Card front/back rendering
- Flip animation (400ms, Y-axis rotation)
- Integrates StarRating and GuessDropdown

### Task 10: Create ChipStack component
- Files: `Source/UI/Components/ChipStack.h/.cpp`
- Decorative stacked chips (5 left, 4 right)
- Red/black alternating with gold rings

### Task 11: Create PokerTable component
- Files: `Source/UI/Components/PokerTable.h/.cpp`
- Wood frame + neon border + felt surface
- Contains ChipStacks and PokerCards
- Card layout: 4 single row, 5-8 double row

### Task 12: Create QuestionBanner component
- Files: `Source/UI/Components/QuestionBanner.h/.cpp`
- "Which card is [Plugin Name]?" display
- Progress indicator (1/4)
- Correct/wrong feedback colors

### Task 13: Create ControlPanel component
- Files: `Source/UI/Components/ControlPanel.h/.cpp`
- Tracks display (read-only)
- Rounds slider
- Auto Gain toggle
- ChipButtons (Shuffle/Reveal/Reset/Next)

### Task 14: Create ResultsPanel component
- Files: `Source/UI/Components/ResultsPanel.h/.cpp`
- Stars mode: rating list sorted by score
- Guess mode: card list with submit button
- Q&A mode: correct/wrong results

### Task 15: Create new BlindCardEditor
- Files: `Source/UI/BlindCardEditor.h/.cpp`
- Compose all components
- Connect to BlindCardManager
- Handle window resizing (900×600 to 1400×900)

### Task 16: Update CMakeLists.txt
- Add all new source files
- Remove old PluginEditor references

### Task 17: Copy card back asset
- Copy `04-card-back.png` from frontend project to `Source/Assets/`
- Add to CMake as binary resource

### Task 18: Integration testing
- Build and run in DAW
- Test all three modes
- Test theme switching
- Test window resizing

### Task 19: Remove old UI files
- Delete old `PluginEditor.h/.cpp`
- Delete old `Source/UI/CardComponent.h/.cpp`
- Delete old `Source/UI/ControlBar/` folder

### Task 20: Final polish
- Verify all animations are smooth (60fps)
- Cross-platform build verification
- AU/VST3 validation

---

## Verification Checklist

After completing all tasks, verify:

- [ ] All 3 modes work: Stars, Guess, Q&A
- [ ] Theme toggle works (Dark ↔ Light)
- [ ] Window resizes correctly (900×600 to 1400×900)
- [ ] 2-8 cards display correctly
- [ ] Card flip animation is smooth
- [ ] Chip buttons have hover/press effects
- [ ] Q&A shows "Which card is [Plugin]?" prompt
- [ ] Results panel shows correct data per mode
- [ ] Connected status indicator works
- [ ] AU validation passes
- [ ] VST3 validation passes
- [ ] macOS build succeeds
- [ ] Windows build succeeds

---

*Plan created: 2026-01-19*
