/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    ModeSelector.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the ModeSelector component.

  ==============================================================================
*/

#include "ModeSelector.h"
#include "../Theme/FontManager.h"

namespace BlindCard
{

//==============================================================================
// Layout constants (larger AirCheck-inspired design)
namespace Layout
{
    // Container
    constexpr float containerCornerRadius = 12.0f;  // Larger corner radius
    constexpr float borderWidth = 1.0f;
    constexpr float containerPadding = 6.0f;        // More padding
    constexpr float optionGap = 8.0f;               // Gap between options

    // Mode options
    constexpr int numModes = 3;
    constexpr float optionCornerRadius = 10.0f;     // Rounder buttons
    constexpr float optionPaddingH = 20.0f;         // More horizontal padding
    constexpr float optionPaddingV = 12.0f;         // More vertical padding
    constexpr float iconSpacing = 10.0f;            // More spacing between icon and label

    // Typography
    constexpr float labelFontSize = 17.0f;          // Larger font
    constexpr float iconFontSize = 20.0f;           // Larger icons

    // Lock icon
    constexpr float lockIconSize = 14.0f;           // Larger lock icon
}

// Colors (AirCheck-inspired vibrant design with theme support)
namespace Colors
{
    // Dark theme
    namespace Dark
    {
        const juce::Colour containerBg      { 0xFF1A1A1A };  // Dark background
        const juce::Colour containerBorder  { 0xFF333333 };  // Subtle border
        const juce::Colour inactiveText     { 0xFF9CA3AF };  // Gray text
        const juce::Colour inactiveBg       { 0xFF2D2D2D };  // Subtle dark bg
        const juce::Colour hoverBg          { 0xFF3D3D3D };  // Lighter on hover
        const juce::Colour hoverText        { 0xFFFFFFFF };  // White on hover
    }

    // Light theme
    namespace Light
    {
        const juce::Colour containerBg      { 0xFFE8E4DF };  // Light beige background
        const juce::Colour containerBorder  { 0xFFD4CFC8 };  // Subtle border
        const juce::Colour inactiveText     { 0xFF6B7280 };  // Gray text
        const juce::Colour inactiveBg       { 0xFFF5F3F0 };  // Light bg
        const juce::Colour hoverBg          { 0xFFE0DCD6 };  // Darker on hover
        const juce::Colour hoverText        { 0xFF1F2937 };  // Dark text on hover
    }

    // Mode-specific colors (same for both themes)
    const juce::Colour starsActiveBg    { 0xFFDC2626 };  // Red
    const juce::Colour starsActiveBorder{ 0xFFEF4444 };

    const juce::Colour guessActiveBg    { 0xFFFACC15 };  // Bright Yellow (Tailwind yellow-400)
    const juce::Colour guessActiveBorder{ 0xFFFDE047 };  // Lighter yellow
    const juce::Colour guessActiveText  { 0xFF1C1917 };  // Dark text for yellow (better contrast)

    const juce::Colour qaActiveBg       { 0xFF2563EB };  // Blue
    const juce::Colour qaActiveBorder   { 0xFF3B82F6 };

    const juce::Colour activeText       { 0xFFFFFFFF };  // White text for red and blue
}

// Mode icons (Unicode characters matching Lucide icons style)
namespace Icons
{
    // Stars: ☆ outline star for inactive, ★ filled for active
    const juce::String StarsOutline = juce::String::charToString(0x2606);  // White star outline (☆)
    const juce::String StarsFilled  = juce::String::charToString(0x2605);  // Black star filled (★)
    const juce::String Stars = StarsFilled;

    // Guess: ? question mark
    const juce::String Guess = "?";  // Simple question mark for better rendering

    // Q&A: Use speech bubbles or Q symbol
    const juce::String QA = juce::String::charToString(0x2754);  // White question mark ornament (❔)

    // Lock icon
    const juce::String Lock  = juce::String::charToString(0x1F512); // Lock 🔒
}

//==============================================================================
ModeSelector::ModeSelector()
{
    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);

    // Enable mouse tracking for hover effects
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

ModeSelector::~ModeSelector()
{
    ThemeManager::getInstance().removeChangeListener(this);
}

//==============================================================================
void ModeSelector::setMode(blindcard::RatingMode mode)
{
    if (currentMode != mode && !lockedState)
    {
        currentMode = mode;
        repaint();

        if (onModeChanged)
            onModeChanged(mode);
    }
}

void ModeSelector::setLocked(bool locked)
{
    if (lockedState != locked)
    {
        lockedState = locked;

        // Update cursor based on lock state
        setMouseCursor(locked
            ? juce::MouseCursor::NormalCursor
            : juce::MouseCursor::PointingHandCursor);

        repaint();
    }
}

//==============================================================================
void ModeSelector::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    bool isDark = ThemeManager::getInstance().isDark();

    // Theme-aware container colors
    auto containerBg = isDark ? Colors::Dark::containerBg : Colors::Light::containerBg;
    auto containerBorder = isDark ? Colors::Dark::containerBorder : Colors::Light::containerBorder;

    // Outer container background
    g.setColour(containerBg);
    g.fillRoundedRectangle(bounds, Layout::containerCornerRadius);

    // Border
    g.setColour(containerBorder);
    g.drawRoundedRectangle(bounds.reduced(Layout::borderWidth / 2.0f),
                           Layout::containerCornerRadius,
                           Layout::borderWidth);

    // Draw mode options with padding
    auto contentBounds = bounds.reduced(Layout::containerPadding);
    float optionWidth = (contentBounds.getWidth() - Layout::optionGap * 2.0f) / Layout::numModes;

    auto starsOptionBounds = juce::Rectangle<float>(
        contentBounds.getX(), contentBounds.getY(),
        optionWidth, contentBounds.getHeight());

    auto guessOptionBounds = juce::Rectangle<float>(
        starsOptionBounds.getRight() + Layout::optionGap, contentBounds.getY(),
        optionWidth, contentBounds.getHeight());

    auto qaOptionBounds = juce::Rectangle<float>(
        guessOptionBounds.getRight() + Layout::optionGap, contentBounds.getY(),
        contentBounds.getRight() - guessOptionBounds.getRight() - Layout::optionGap, contentBounds.getHeight());

    drawModeOption(g, starsOptionBounds, Icons::Stars, "Stars",
                   currentMode == blindcard::RatingMode::Stars,
                   currentHover == HoverState::Stars,
                   blindcard::RatingMode::Stars, isDark);

    drawModeOption(g, guessOptionBounds, Icons::Guess, "Guess",
                   currentMode == blindcard::RatingMode::Guess,
                   currentHover == HoverState::Guess,
                   blindcard::RatingMode::Guess, isDark);

    drawModeOption(g, qaOptionBounds, Icons::QA, "Q&A",
                   currentMode == blindcard::RatingMode::QA,
                   currentHover == HoverState::QA,
                   blindcard::RatingMode::QA, isDark);

    // Draw lock overlay if locked
    if (lockedState)
    {
        drawLockOverlay(g, bounds);
    }
}

void ModeSelector::resized()
{
    auto bounds = getLocalBounds();
    int optionWidth = bounds.getWidth() / Layout::numModes;

    starsBounds = bounds.removeFromLeft(optionWidth);
    guessBounds = bounds.removeFromLeft(optionWidth);
    qaBounds = bounds; // Remainder goes to Q&A
}

//==============================================================================
void ModeSelector::mouseMove(const juce::MouseEvent& event)
{
    if (lockedState)
        return;

    auto newHover = getHoverStateAt(event.getPosition());
    if (newHover != currentHover)
    {
        currentHover = newHover;
        repaint();
    }
}

void ModeSelector::mouseExit(const juce::MouseEvent&)
{
    if (currentHover != HoverState::None)
    {
        currentHover = HoverState::None;
        repaint();
    }
}

void ModeSelector::mouseDown(const juce::MouseEvent& event)
{
    if (lockedState)
        return;

    auto clickState = getHoverStateAt(event.getPosition());

    if (clickState != HoverState::None)
    {
        auto newMode = hoverStateToMode(clickState);
        if (newMode != currentMode)
        {
            currentMode = newMode;
            repaint();

            if (onModeChanged)
                onModeChanged(newMode);
        }
    }
}

//==============================================================================
void ModeSelector::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        repaint();
    }
}

//==============================================================================
void ModeSelector::drawModeOption(juce::Graphics& g,
                                   juce::Rectangle<float> bounds,
                                   const juce::String& icon,
                                   const juce::String& label,
                                   bool isSelected,
                                   bool isHovered,
                                   blindcard::RatingMode mode,
                                   bool isDarkTheme)
{
    auto& fonts = FontManager::getInstance();

    // Get theme-specific inactive colors
    juce::Colour inactiveBg = isDarkTheme ? Colors::Dark::inactiveBg : Colors::Light::inactiveBg;
    juce::Colour inactiveText = isDarkTheme ? Colors::Dark::inactiveText : Colors::Light::inactiveText;
    juce::Colour hoverBg = isDarkTheme ? Colors::Dark::hoverBg : Colors::Light::hoverBg;
    juce::Colour hoverText = isDarkTheme ? Colors::Dark::hoverText : Colors::Light::hoverText;

    // Get mode-specific active colors
    juce::Colour activeBg, activeBorder, activeTextColour;
    switch (mode)
    {
        case blindcard::RatingMode::Stars:
            activeBg = Colors::starsActiveBg;
            activeBorder = Colors::starsActiveBorder;
            activeTextColour = Colors::activeText;
            break;
        case blindcard::RatingMode::Guess:
            activeBg = Colors::guessActiveBg;
            activeBorder = Colors::guessActiveBorder;
            activeTextColour = Colors::guessActiveText;  // Dark text for yellow
            break;
        case blindcard::RatingMode::QA:
            activeBg = Colors::qaActiveBg;
            activeBorder = Colors::qaActiveBorder;
            activeTextColour = Colors::activeText;
            break;
    }

    // Determine colors based on state
    juce::Colour bgColour = inactiveBg;
    juce::Colour textColour = inactiveText;

    if (isSelected)
    {
        // Solid vibrant fill for selected state
        bgColour = activeBg;
        textColour = activeTextColour;
    }
    else if (isHovered && !lockedState)
    {
        // Lighter/darker background on hover
        bgColour = hoverBg;
        textColour = hoverText;
    }

    // Draw background with rounded corners - always draw for all states
    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, Layout::optionCornerRadius);

    // Draw subtle glow border for selected state
    if (isSelected)
    {
        // Inner glow effect - slightly lighter border
        g.setColour(activeBorder.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), Layout::optionCornerRadius, 2.0f);
    }

    // Calculate content layout - Clean readable style
    juce::Font labelFont = fonts.getMedium(Layout::labelFontSize);

    // Larger icon size
    float iconSize = 18.0f;
    float labelWidth = labelFont.getStringWidthFloat(label);
    float totalContentWidth = iconSize + Layout::iconSpacing + labelWidth;

    // Center the content horizontally
    float contentStartX = bounds.getX() + (bounds.getWidth() - totalContentWidth) / 2.0f;
    float iconCenterX = contentStartX + iconSize / 2.0f;
    float iconCenterY = bounds.getCentreY();

    // Draw icon as path (more reliable than Unicode fonts)
    g.setColour(textColour);

    if (label == "Stars")
    {
        // Draw 5-pointed star
        juce::Path star;
        float outerR = iconSize / 2.0f;
        float innerR = outerR * 0.4f;
        for (int i = 0; i < 10; ++i)
        {
            float angle = static_cast<float>(i) * juce::MathConstants<float>::pi / 5.0f - juce::MathConstants<float>::halfPi;
            float r = (i % 2 == 0) ? outerR : innerR;
            float x = iconCenterX + r * std::cos(angle);
            float y = iconCenterY + r * std::sin(angle);
            if (i == 0)
                star.startNewSubPath(x, y);
            else
                star.lineTo(x, y);
        }
        star.closeSubPath();
        g.fillPath(star);
    }
    else if (label == "Guess")
    {
        // Draw question mark circle
        float circleR = iconSize / 2.0f - 1.0f;
        g.drawEllipse(iconCenterX - circleR, iconCenterY - circleR, circleR * 2.0f, circleR * 2.0f, 1.5f);
        g.setFont(fonts.getMedium(14.0f));
        g.drawText("?", juce::Rectangle<float>(contentStartX, bounds.getY(), iconSize, bounds.getHeight()),
                   juce::Justification::centred);
    }
    else if (label == "Q&A")
    {
        // Draw speech bubble
        juce::Path bubble;
        float bw = iconSize - 2.0f;
        float bh = iconSize * 0.7f;
        float bx = iconCenterX - bw / 2.0f;
        float by = iconCenterY - bh / 2.0f - 1.0f;
        bubble.addRoundedRectangle(bx, by, bw, bh, 3.0f);
        // Add tail
        bubble.startNewSubPath(bx + bw * 0.3f, by + bh);
        bubble.lineTo(bx + bw * 0.2f, by + bh + 4.0f);
        bubble.lineTo(bx + bw * 0.5f, by + bh);
        bubble.closeSubPath();
        g.fillPath(bubble);
    }

    // Draw label (font-medium = bold in JUCE terms)
    g.setFont(labelFont);
    g.drawText(label,
               juce::Rectangle<float>(contentStartX + iconSize + Layout::iconSpacing,
                                       bounds.getY(),
                                       labelWidth,
                                       bounds.getHeight()),
               juce::Justification::centred);

    // Draw lock icon if locked and selected
    if (lockedState && isSelected)
    {
        juce::Font lockFont = fonts.getRegular(Layout::lockIconSize);
        float lockWidth = lockFont.getStringWidthFloat(Icons::Lock);
        g.setColour(textColour.withAlpha(0.5f));  // Original: text-[#FF3B4E]/50
        g.setFont(lockFont);
        g.drawText(Icons::Lock,
                   juce::Rectangle<float>(bounds.getRight() - lockWidth - 8.0f,
                                           bounds.getY(),
                                           lockWidth,
                                           bounds.getHeight()),
                   juce::Justification::centred);
    }
}

void ModeSelector::drawLockOverlay(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& fonts = FontManager::getInstance();
    bool isDark = ThemeManager::getInstance().isDark();

    auto containerBg = isDark ? Colors::Dark::containerBg : Colors::Light::containerBg;
    auto inactiveText = isDark ? Colors::Dark::inactiveText : Colors::Light::inactiveText;

    // Semi-transparent overlay using container color
    g.setColour(containerBg.withAlpha(0.6f));
    g.fillRoundedRectangle(bounds, Layout::containerCornerRadius);

    // Lock icon in center
    g.setColour(inactiveText);
    g.setFont(fonts.getRegular(18.0f));
    g.drawText(Icons::Lock, bounds.toNearestInt(), juce::Justification::centred);
}

//==============================================================================
ModeSelector::HoverState ModeSelector::getHoverStateAt(juce::Point<int> position) const
{
    if (starsBounds.contains(position))
        return HoverState::Stars;

    if (guessBounds.contains(position))
        return HoverState::Guess;

    if (qaBounds.contains(position))
        return HoverState::QA;

    return HoverState::None;
}

blindcard::RatingMode ModeSelector::hoverStateToMode(HoverState state) const
{
    switch (state)
    {
        case HoverState::Stars: return blindcard::RatingMode::Stars;
        case HoverState::Guess: return blindcard::RatingMode::Guess;
        case HoverState::QA:    return blindcard::RatingMode::QA;
        default:                return currentMode;
    }
}

juce::Rectangle<int> ModeSelector::getModeButtonBounds(blindcard::RatingMode mode) const
{
    switch (mode)
    {
        case blindcard::RatingMode::Stars: return starsBounds;
        case blindcard::RatingMode::Guess: return guessBounds;
        case blindcard::RatingMode::QA:    return qaBounds;
        default:                            return starsBounds;
    }
}

} // namespace BlindCard
