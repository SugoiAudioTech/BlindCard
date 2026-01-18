/*
  ==============================================================================

    HeaderBar.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the HeaderBar component.

  ==============================================================================
*/

#include "HeaderBar.h"

namespace BlindCard
{

//==============================================================================
// Layout constants
namespace Layout
{
    // Logo area
    constexpr int logoSize = 40;
    constexpr int logoLeftMargin = 16;
    constexpr float logoCornerRadius = 8.0f;

    // Brand text
    constexpr int brandLeftMargin = 12;
    constexpr float brandNameFontSize = 18.0f;
    constexpr float sugoiFontSize = 10.0f;

    // Connection status capsule
    constexpr int capsuleWidth = 130;
    constexpr int capsuleHeight = 28;
    constexpr float capsuleCornerRadius = 14.0f;
    constexpr int statusDotSize = 8;

    // Right controls
    constexpr int controlRightMargin = 16;
    constexpr int controlSpacing = 8;

    // Theme toggle (pill shape)
    constexpr int toggleWidth = 44;
    constexpr int toggleHeight = 24;
    constexpr float toggleCornerRadius = 12.0f;

    // Icon buttons
    constexpr int iconButtonSize = 32;
    constexpr float iconButtonCornerRadius = 6.0f;
}

//==============================================================================
HeaderBar::HeaderBar()
{
    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);

    // Enable mouse tracking for hover effects
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

HeaderBar::~HeaderBar()
{
    ThemeManager::getInstance().removeChangeListener(this);
    stopTimer();
}

//==============================================================================
void HeaderBar::setConnected(bool connected)
{
    if (connectionStatus != connected)
    {
        connectionStatus = connected;

        if (connected)
        {
            // Start pulse animation
            pulsePhase = 0.0f;
            startTimer(kTimerIntervalMs);
        }
        else
        {
            // Stop pulse animation
            stopTimer();
        }

        repaint();
    }
}

//==============================================================================
void HeaderBar::paint(juce::Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(theme.getColour(ColourId::Surface));
    g.fillRect(bounds);

    // Subtle bottom border
    g.setColour(theme.getColour(ColourId::SurfaceAlt));
    g.fillRect(bounds.removeFromBottom(1.0f));

    // Draw components
    float logoX = static_cast<float>(Layout::logoLeftMargin);
    float logoY = (bounds.getHeight() - Layout::logoSize) / 2.0f;
    drawLogo(g, { logoX, logoY, static_cast<float>(Layout::logoSize), static_cast<float>(Layout::logoSize) });

    // Brand text (to the right of logo)
    float brandX = logoX + Layout::logoSize + Layout::brandLeftMargin;
    float brandY = logoY;
    drawBrandText(g, { brandX, brandY, 120.0f, static_cast<float>(Layout::logoSize) });

    // Connection status (centered)
    drawConnectionStatus(g, connectionStatusBounds.toFloat());

    // Right controls
    drawThemeToggle(g, themeToggleBounds.toFloat(), currentHover == HoverState::ThemeToggle);
    drawInfoButton(g, infoButtonBounds.toFloat(), currentHover == HoverState::InfoButton);
    drawSettingsButton(g, settingsButtonBounds.toFloat(), currentHover == HoverState::SettingsButton);
}

//==============================================================================
void HeaderBar::resized()
{
    auto bounds = getLocalBounds();
    auto centerY = bounds.getCentreY();

    // Calculate right-side button positions (from right to left)
    int rightEdge = bounds.getWidth() - Layout::controlRightMargin;

    // Settings button
    settingsButtonBounds = {
        rightEdge - Layout::iconButtonSize,
        centerY - Layout::iconButtonSize / 2,
        Layout::iconButtonSize,
        Layout::iconButtonSize
    };
    rightEdge = settingsButtonBounds.getX() - Layout::controlSpacing;

    // Info button
    infoButtonBounds = {
        rightEdge - Layout::iconButtonSize,
        centerY - Layout::iconButtonSize / 2,
        Layout::iconButtonSize,
        Layout::iconButtonSize
    };
    rightEdge = infoButtonBounds.getX() - Layout::controlSpacing;

    // Theme toggle
    themeToggleBounds = {
        rightEdge - Layout::toggleWidth,
        centerY - Layout::toggleHeight / 2,
        Layout::toggleWidth,
        Layout::toggleHeight
    };
    rightEdge = themeToggleBounds.getX() - Layout::controlSpacing * 2;

    // Connection status (positioned between brand and controls)
    connectionStatusBounds = {
        rightEdge - Layout::capsuleWidth,
        centerY - Layout::capsuleHeight / 2,
        Layout::capsuleWidth,
        Layout::capsuleHeight
    };
}

//==============================================================================
void HeaderBar::mouseMove(const juce::MouseEvent& event)
{
    auto newHover = getHoverStateAt(event.getPosition());
    if (newHover != currentHover)
    {
        currentHover = newHover;
        repaint();
    }
}

void HeaderBar::mouseExit(const juce::MouseEvent&)
{
    if (currentHover != HoverState::None)
    {
        currentHover = HoverState::None;
        repaint();
    }
}

void HeaderBar::mouseDown(const juce::MouseEvent& event)
{
    auto clickState = getHoverStateAt(event.getPosition());

    switch (clickState)
    {
        case HoverState::ThemeToggle:
            if (onThemeToggled)
                onThemeToggled();
            break;

        case HoverState::InfoButton:
            if (onInfoClicked)
                onInfoClicked();
            break;

        case HoverState::SettingsButton:
            if (onSettingsClicked)
                onSettingsClicked();
            break;

        default:
            break;
    }
}

//==============================================================================
void HeaderBar::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        repaint();
    }
}

void HeaderBar::timerCallback()
{
    // Update pulse animation
    pulsePhase += static_cast<float>(kTimerIntervalMs) / kPulsePeriodMs;
    if (pulsePhase >= 1.0f)
        pulsePhase -= 1.0f;

    // Only repaint the connection status area for efficiency
    repaint(connectionStatusBounds);
}

//==============================================================================
void HeaderBar::drawLogo(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    // Red gradient background
    juce::Colour gradientTop = theme.getColour(ColourId::Primary);
    juce::Colour gradientBottom = gradientTop.darker(0.2f);

    juce::ColourGradient gradient(
        gradientTop, bounds.getX(), bounds.getY(),
        gradientBottom, bounds.getX(), bounds.getBottom(),
        false);

    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, Layout::logoCornerRadius);

    // White spade symbol
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f));

    // Draw spade symbol centered
    g.drawText(juce::String::charToString(0x2660), // Unicode spade
               bounds.toNearestInt(),
               juce::Justification::centred);
}

void HeaderBar::drawBrandText(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    // "BlindCard" in red, bold
    g.setColour(theme.getColour(ColourId::Primary));
    g.setFont(juce::Font(Layout::brandNameFontSize, juce::Font::bold));

    auto nameArea = bounds.removeFromTop(bounds.getHeight() * 0.6f);
    g.drawText("BlindCard", nameArea, juce::Justification::centredLeft);

    // "BY SUGOI AUDIO" in gray
    g.setColour(theme.getColour(ColourId::TextMuted));
    g.setFont(juce::Font(Layout::sugoiFontSize));
    g.drawText("BY SUGOI AUDIO", bounds, juce::Justification::centredLeft);
}

void HeaderBar::drawConnectionStatus(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    // Dark capsule background
    g.setColour(theme.isDark() ? juce::Colour(0xFF1A1A1A) : juce::Colour(0xFFE8E8E8));
    g.fillRoundedRectangle(bounds, Layout::capsuleCornerRadius);

    // Content area with padding
    auto contentArea = bounds.reduced(12.0f, 0.0f);

    // Headphone icon (using a simple shape)
    auto iconArea = contentArea.removeFromLeft(16.0f);
    g.setColour(theme.getColour(ColourId::TextSecondary));

    // Draw simplified headphone icon
    float iconCenterX = iconArea.getCentreX();
    float iconCenterY = iconArea.getCentreY();
    float iconRadius = 6.0f;

    // Headband arc
    juce::Path headband;
    headband.addCentredArc(iconCenterX, iconCenterY + 2.0f,
                           iconRadius, iconRadius,
                           0.0f,
                           -juce::MathConstants<float>::pi,
                           0.0f,
                           true);
    g.strokePath(headband, juce::PathStrokeType(1.5f));

    // Ear cups
    g.fillEllipse(iconCenterX - iconRadius - 2.0f, iconCenterY, 4.0f, 6.0f);
    g.fillEllipse(iconCenterX + iconRadius - 2.0f, iconCenterY, 4.0f, 6.0f);

    // Status text
    contentArea.removeFromLeft(6.0f); // Spacing
    auto textArea = contentArea.removeFromLeft(70.0f);

    g.setFont(juce::Font(12.0f));
    g.setColour(theme.getColour(ColourId::TextSecondary));
    g.drawText(connectionStatus ? "Connected" : "No Connection",
               textArea,
               juce::Justification::centredLeft);

    // Status dot
    if (connectionStatus)
    {
        // Calculate pulse alpha (sine wave: 0.4 to 1.0)
        float pulseAlpha = 0.4f + 0.6f * (0.5f + 0.5f * std::sin(pulsePhase * juce::MathConstants<float>::twoPi));

        // Gold dot with pulse
        juce::Colour goldColour = theme.getColour(ColourId::Accent);
        g.setColour(goldColour.withAlpha(pulseAlpha));

        float dotX = contentArea.getRight() - Layout::statusDotSize - 4.0f;
        float dotY = bounds.getCentreY() - Layout::statusDotSize / 2.0f;

        g.fillEllipse(dotX, dotY,
                      static_cast<float>(Layout::statusDotSize),
                      static_cast<float>(Layout::statusDotSize));

        // Glow effect when at peak brightness
        if (pulseAlpha > 0.8f)
        {
            g.setColour(goldColour.withAlpha((pulseAlpha - 0.8f) * 0.5f));
            g.fillEllipse(dotX - 2.0f, dotY - 2.0f,
                          static_cast<float>(Layout::statusDotSize + 4),
                          static_cast<float>(Layout::statusDotSize + 4));
        }
    }
    else
    {
        // Gray dot when disconnected
        g.setColour(theme.getColour(ColourId::TextMuted));
        float dotX = contentArea.getRight() - Layout::statusDotSize - 4.0f;
        float dotY = bounds.getCentreY() - Layout::statusDotSize / 2.0f;
        g.fillEllipse(dotX, dotY,
                      static_cast<float>(Layout::statusDotSize),
                      static_cast<float>(Layout::statusDotSize));
    }
}

void HeaderBar::drawThemeToggle(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered)
{
    auto& theme = ThemeManager::getInstance();
    bool isDark = theme.isDark();

    // Toggle background
    juce::Colour bgColour = isDark
        ? juce::Colour(0xFF2A2A2A)
        : juce::Colour(0xFFE0E0E0);

    if (isHovered)
        bgColour = bgColour.brighter(0.1f);

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, Layout::toggleCornerRadius);

    // Knob position (left for light mode, right for dark mode)
    float knobDiameter = bounds.getHeight() - 4.0f;
    float knobX = isDark
        ? bounds.getRight() - knobDiameter - 2.0f
        : bounds.getX() + 2.0f;
    float knobY = bounds.getY() + 2.0f;

    // Knob with glow when in dark mode
    juce::Rectangle<float> knobBounds(knobX, knobY, knobDiameter, knobDiameter);

    if (isDark)
    {
        // Red glow effect
        juce::Colour glowColour = theme.getColour(ColourId::Primary).withAlpha(0.4f);
        g.setColour(glowColour);
        g.fillEllipse(knobBounds.expanded(3.0f));
    }

    // Knob fill
    g.setColour(isDark ? theme.getColour(ColourId::Primary) : juce::Colours::white);
    g.fillEllipse(knobBounds);

    // Icon inside knob
    g.setColour(isDark ? juce::Colours::white : theme.getColour(ColourId::TextMuted));
    g.setFont(juce::Font(12.0f));

    // Moon (dark mode) or Sun (light mode)
    juce::String icon = isDark
        ? juce::String::charToString(0x263D)   // Crescent moon
        : juce::String::charToString(0x2600);  // Sun

    g.drawText(icon, knobBounds.toNearestInt(), juce::Justification::centred);
}

void HeaderBar::drawInfoButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered)
{
    auto& theme = ThemeManager::getInstance();

    // Button background
    juce::Colour bgColour = theme.isDark()
        ? juce::Colour(0xFF2A2A2A)
        : juce::Colour(0xFFE8E8E8);

    if (isHovered)
        bgColour = bgColour.brighter(0.15f);

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, Layout::iconButtonCornerRadius);

    // Info icon (circle with 'i')
    g.setColour(isHovered
        ? theme.getColour(ColourId::Primary)
        : theme.getColour(ColourId::TextSecondary));

    // Draw circle outline
    float iconSize = 18.0f;
    auto iconBounds = bounds.withSizeKeepingCentre(iconSize, iconSize);
    g.drawEllipse(iconBounds, 1.5f);

    // Draw 'i' character
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("i", iconBounds.toNearestInt(), juce::Justification::centred);
}

void HeaderBar::drawSettingsButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered)
{
    auto& theme = ThemeManager::getInstance();

    // Button background
    juce::Colour bgColour = theme.isDark()
        ? juce::Colour(0xFF2A2A2A)
        : juce::Colour(0xFFE8E8E8);

    if (isHovered)
        bgColour = bgColour.brighter(0.15f);

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, Layout::iconButtonCornerRadius);

    // Gear icon
    g.setColour(isHovered
        ? theme.getColour(ColourId::Primary)
        : theme.getColour(ColourId::TextSecondary));

    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();
    float outerRadius = 9.0f;
    float innerRadius = 4.0f;
    int numTeeth = 8;

    // Draw gear shape
    juce::Path gear;
    for (int i = 0; i < numTeeth * 2; ++i)
    {
        float angle = static_cast<float>(i) * juce::MathConstants<float>::pi / static_cast<float>(numTeeth);
        float radius = (i % 2 == 0) ? outerRadius : outerRadius - 2.5f;

        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);

        if (i == 0)
            gear.startNewSubPath(x, y);
        else
            gear.lineTo(x, y);
    }
    gear.closeSubPath();

    // Cut out center circle
    gear.addEllipse(centerX - innerRadius, centerY - innerRadius,
                    innerRadius * 2.0f, innerRadius * 2.0f);

    g.fillPath(gear, juce::AffineTransform(),
               juce::FillType(g.getCurrentColour()).withEvenOddFill());
}

//==============================================================================
HeaderBar::HoverState HeaderBar::getHoverStateAt(juce::Point<int> position) const
{
    if (themeToggleBounds.contains(position))
        return HoverState::ThemeToggle;

    if (infoButtonBounds.contains(position))
        return HoverState::InfoButton;

    if (settingsButtonBounds.contains(position))
        return HoverState::SettingsButton;

    return HoverState::None;
}

} // namespace BlindCard
