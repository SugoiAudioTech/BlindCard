/*
  ==============================================================================

    ModeSelector.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the ModeSelector component.

  ==============================================================================
*/

#include "ModeSelector.h"

namespace BlindCard
{

//==============================================================================
// Layout constants
namespace Layout
{
    // Container
    constexpr float containerCornerRadius = 20.0f;
    constexpr float borderWidth = 1.0f;

    // Mode options
    constexpr int numModes = 3;
    constexpr float optionPaddingH = 12.0f;
    constexpr float iconSpacing = 6.0f;

    // Typography
    constexpr float labelFontSize = 13.0f;
    constexpr float iconFontSize = 14.0f;

    // Colors for selected state
    constexpr float selectedBgAlpha = 0.15f;  // rgba(255,59,78,0.15)

    // Lock icon
    constexpr float lockIconSize = 12.0f;
}

// Mode icons (Unicode characters)
namespace Icons
{
    const juce::String Stars = juce::String::charToString(0x2606);  // White star
    const juce::String Guess = juce::String::charToString(0x25CE);  // Bullseye
    const juce::String QA    = juce::String::charToString(0x270E);  // Pencil
    const juce::String Lock  = juce::String::charToString(0x1F512); // Lock
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
    auto& theme = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Outer container background
    g.setColour(theme.getColour(ColourId::Surface));
    g.fillRoundedRectangle(bounds, Layout::containerCornerRadius);

    // Border
    g.setColour(theme.getColour(ColourId::SurfaceAlt));
    g.drawRoundedRectangle(bounds.reduced(Layout::borderWidth / 2.0f),
                           Layout::containerCornerRadius,
                           Layout::borderWidth);

    // Draw mode options
    drawModeOption(g, starsBounds.toFloat(), Icons::Stars, "Stars",
                   currentMode == blindcard::RatingMode::Stars,
                   currentHover == HoverState::Stars);

    drawModeOption(g, guessBounds.toFloat(), Icons::Guess, "Guess",
                   currentMode == blindcard::RatingMode::Guess,
                   currentHover == HoverState::Guess);

    drawModeOption(g, qaBounds.toFloat(), Icons::QA, "Q&A",
                   currentMode == blindcard::RatingMode::QA,
                   currentHover == HoverState::QA);

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
                                   bool isHovered)
{
    auto& theme = ThemeManager::getInstance();

    // Determine colors based on state
    juce::Colour bgColour = juce::Colours::transparentBlack;
    juce::Colour textColour = theme.getColour(ColourId::TextSecondary); // #9CA3AF equivalent

    if (isSelected)
    {
        // Selected: red tint background, red text
        bgColour = theme.getColour(ColourId::Primary).withAlpha(Layout::selectedBgAlpha);
        textColour = theme.getColour(ColourId::Primary); // #FF3B4E equivalent
    }
    else if (isHovered && !lockedState)
    {
        // Hover: slight white overlay, lighter text
        bgColour = juce::Colours::white.withAlpha(0.05f);
        textColour = theme.getColour(ColourId::TextPrimary).interpolatedWith(
            theme.getColour(ColourId::TextSecondary), 0.3f);
    }

    // Draw background if not transparent
    if (!bgColour.isTransparent())
    {
        // For selected state, apply rounded corners at edges
        float cornerRadius = 0.0f;
        bool isLeftEdge = (bounds.getX() < 2.0f);
        bool isRightEdge = (bounds.getRight() > getWidth() - 2);

        if (isSelected)
        {
            juce::Path bgPath;
            if (isLeftEdge)
            {
                // Left option: rounded left corners
                bgPath.addRoundedRectangle(bounds.getX(), bounds.getY(),
                                            bounds.getWidth(), bounds.getHeight(),
                                            Layout::containerCornerRadius, Layout::containerCornerRadius,
                                            true, false, true, false);
            }
            else if (isRightEdge)
            {
                // Right option: rounded right corners
                bgPath.addRoundedRectangle(bounds.getX(), bounds.getY(),
                                            bounds.getWidth(), bounds.getHeight(),
                                            Layout::containerCornerRadius, Layout::containerCornerRadius,
                                            false, true, false, true);
            }
            else
            {
                // Middle option: no rounded corners
                bgPath.addRectangle(bounds);
            }

            g.setColour(bgColour);
            g.fillPath(bgPath);

            // Draw selected border accent on bottom
            g.setColour(theme.getColour(ColourId::Primary));
            g.fillRect(bounds.getX() + Layout::optionPaddingH,
                       bounds.getBottom() - 2.0f,
                       bounds.getWidth() - Layout::optionPaddingH * 2.0f,
                       2.0f);
        }
        else
        {
            g.setColour(bgColour);
            g.fillRect(bounds);
        }
    }

    // Calculate content layout
    auto contentBounds = bounds.reduced(Layout::optionPaddingH, 0.0f);

    // Measure text widths
    juce::Font iconFont(Layout::iconFontSize);
    juce::Font labelFont(Layout::labelFontSize);

    float iconWidth = iconFont.getStringWidthFloat(icon);
    float labelWidth = labelFont.getStringWidthFloat(label);
    float totalContentWidth = iconWidth + Layout::iconSpacing + labelWidth;

    // Center the content
    float contentStartX = contentBounds.getX() + (contentBounds.getWidth() - totalContentWidth) / 2.0f;

    // Draw icon
    g.setColour(textColour);
    g.setFont(iconFont);
    g.drawText(icon,
               juce::Rectangle<float>(contentStartX, bounds.getY(), iconWidth, bounds.getHeight()),
               juce::Justification::centred);

    // Draw label
    g.setFont(labelFont);
    g.drawText(label,
               juce::Rectangle<float>(contentStartX + iconWidth + Layout::iconSpacing,
                                       bounds.getY(),
                                       labelWidth,
                                       bounds.getHeight()),
               juce::Justification::centred);
}

void ModeSelector::drawLockOverlay(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    // Semi-transparent overlay
    g.setColour(theme.getColour(ColourId::Surface).withAlpha(0.6f));
    g.fillRoundedRectangle(bounds, Layout::containerCornerRadius);

    // Lock icon in center
    g.setColour(theme.getColour(ColourId::TextMuted));
    g.setFont(juce::Font(16.0f));
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
