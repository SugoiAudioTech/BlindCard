/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    TransportBar.cpp
    Created: 2025-01-21
    Author:  BlindCard

  ==============================================================================
*/

#include "TransportBar.h"
#include "../Theme/FontManager.h"
#include <cmath>

namespace BlindCard
{

namespace
{
    constexpr double kPositionEpsilon = 1.0e-6;
}

//==============================================================================
TransportBar::TransportBar()
{
    ThemeManager::getInstance().addChangeListener(this);
}

TransportBar::~TransportBar()
{
    ThemeManager::getInstance().removeChangeListener(this);
}

//==============================================================================
void TransportBar::setPlaying(bool playing)
{
    if (isPlaying != playing)
    {
        isPlaying = playing;
        repaint();
    }
}

void TransportBar::setPosition(double positionSeconds)
{
    if (!isDraggingProgressBar
        && std::abs(currentPositionSeconds - positionSeconds) > kPositionEpsilon)
    {
        currentPositionSeconds = positionSeconds;
        repaint();
    }
}

void TransportBar::setDuration(double durationSeconds)
{
    if (std::abs(totalDurationSeconds - durationSeconds) > kPositionEpsilon)
    {
        totalDurationSeconds = durationSeconds;
        repaint();
    }
}

void TransportBar::setEnabled(bool enabled)
{
    if (isEnabled != enabled)
    {
        isEnabled = enabled;
        repaint();
    }
}

void TransportBar::setLooping(bool looping)
{
    if (isLooping != looping)
    {
        isLooping = looping;
        repaint();
    }
}

//==============================================================================
void TransportBar::paint(juce::Graphics& g)
{
    auto& theme = ThemeManager::getInstance();

    // Draw loop button (with accent color when active)
    {
        auto lb = loopBounds.toFloat();
        bool loopHovered = currentHover == HoverState::Loop;

        // Background
        if (isLooping && isEnabled)
        {
            g.setColour(theme.getColour(ColourId::Accent).withAlpha(0.25f));
            g.fillRoundedRectangle(lb, 4.0f);
        }
        else if (loopHovered && isEnabled)
        {
            g.setColour(theme.getColour(ColourId::Accent).withAlpha(0.2f));
            g.fillRoundedRectangle(lb, 4.0f);
        }

        // Text
        juce::Colour loopColor;
        if (!isEnabled)
            loopColor = theme.isDark() ? juce::Colour(0xFFA4ABA4) : juce::Colour(0xFF706C66);
        else if (isLooping)
            loopColor = theme.isDark() ? juce::Colour(0xFFE4868B) : juce::Colour(0xFFB8414D);
        else if (loopHovered)
            loopColor = theme.isDark() ? juce::Colour(0xFFE4868B) : juce::Colour(0xFFB8414D);
        else
            loopColor = theme.getColour(ColourId::TextPrimary);

        g.setColour(loopColor);

        // Draw loop icon: rounded-rect loop with two arrowheads
        // Standard repeat/loop symbol like media players use
        auto iconBounds = lb.reduced(4.0f);
        float cx = iconBounds.getCentreX();
        float cy = iconBounds.getCentreY();
        float hw = iconBounds.getWidth() * 0.38f;   // half width
        float hh = iconBounds.getHeight() * 0.25f;  // half height
        float cr = hh;                                // corner radius

        // Top path: left-to-right with rounded ends
        juce::Path topPath;
        topPath.startNewSubPath(cx - hw + cr, cy - hh);
        topPath.lineTo(cx + hw - cr, cy - hh);
        topPath.addArc(cx + hw - cr * 2, cy - hh, cr * 2, cr * 2,
                        -juce::MathConstants<float>::halfPi, 0.0f, true);

        // Bottom path: right-to-left with rounded ends
        juce::Path bottomPath;
        bottomPath.startNewSubPath(cx + hw - cr, cy + hh);
        bottomPath.lineTo(cx - hw + cr, cy + hh);
        bottomPath.addArc(cx - hw, cy + hh - cr * 2, cr * 2, cr * 2,
                           juce::MathConstants<float>::halfPi, juce::MathConstants<float>::pi, true);

        g.strokePath(topPath, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved));
        g.strokePath(bottomPath, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved));

        // Right arrowhead (pointing right, at top-right)
        float ax = cx + hw;
        float ay = cy - hh;
        juce::Path arrowR;
        arrowR.addTriangle(ax - 1.0f, ay - 3.5f,
                           ax + 3.5f, ay,
                           ax - 1.0f, ay + 3.5f);
        g.fillPath(arrowR);

        // Left arrowhead (pointing left, at bottom-left)
        float ax2 = cx - hw;
        float ay2 = cy + hh;
        juce::Path arrowL;
        arrowL.addTriangle(ax2 + 1.0f, ay2 - 3.5f,
                           ax2 - 3.5f, ay2,
                           ax2 + 1.0f, ay2 + 3.5f);
        g.fillPath(arrowL);
    }

    // The playback symbols are drawn as vectors, independent of text fonts.
    drawButton(g, skipBackBounds.toFloat(), ButtonIcon::SkipBack,
               currentHover == HoverState::SkipBack, isEnabled);

    drawButton(g, playPauseBounds.toFloat(), isPlaying ? ButtonIcon::Pause : ButtonIcon::Play,
               currentHover == HoverState::PlayPause, isEnabled);

    drawButton(g, skipForwardBounds.toFloat(), ButtonIcon::SkipForward,
               currentHover == HoverState::SkipForward, isEnabled);

    // Draw time label
    drawTimeLabel(g, timeLabelBounds.toFloat());

    // Draw progress bar
    drawProgressBar(g, progressBarBounds.toFloat());
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds();

    const int buttonSize = 24;
    const int buttonSpacing = 4;
    const int timeLabelWidth = 85;
    const int gap = 16;

    // Layout buttons (left-aligned)
    int x = 0;
    int btnY = (bounds.getHeight() - buttonSize) / 2;

    loopBounds = juce::Rectangle<int>(x, btnY, buttonSize, buttonSize);
    x += buttonSize + buttonSpacing;

    skipBackBounds = juce::Rectangle<int>(x, btnY, buttonSize, buttonSize);
    x += buttonSize + buttonSpacing;

    playPauseBounds = juce::Rectangle<int>(x, btnY, buttonSize, buttonSize);
    x += buttonSize + buttonSpacing;

    skipForwardBounds = juce::Rectangle<int>(x, btnY, buttonSize, buttonSize);
    x += buttonSize + gap;

    // Time label
    timeLabelBounds = juce::Rectangle<int>(x, 0, timeLabelWidth, bounds.getHeight());
    x += timeLabelWidth + gap;

    // Progress bar (fills remaining width)
    int progressBarWidth = std::max(50, bounds.getWidth() - x);
    progressBarBounds = juce::Rectangle<int>(x, (bounds.getHeight() - 8) / 2,
                                              progressBarWidth, 8);
}

//==============================================================================
void TransportBar::mouseDown(const juce::MouseEvent& event)
{
    if (!isEnabled)
        return;

    auto pos = event.getPosition();

    if (loopBounds.contains(pos))
    {
        if (onLoopClicked)
            onLoopClicked();
    }
    else if (skipBackBounds.contains(pos))
    {
        if (onSkipBackward)
            onSkipBackward();
    }
    else if (playPauseBounds.contains(pos))
    {
        if (onPlayPauseClicked)
            onPlayPauseClicked();
    }
    else if (skipForwardBounds.contains(pos))
    {
        if (onSkipForward)
            onSkipForward();
    }
    else if (progressBarBounds.contains(pos))
    {
        isDraggingProgressBar = true;
        dragPosition = getProgressBarPosition(pos.x);
        repaint();
    }
}

void TransportBar::mouseDrag(const juce::MouseEvent& event)
{
    if (isDraggingProgressBar)
    {
        dragPosition = getProgressBarPosition(event.getPosition().x);
        repaint();
    }
}

void TransportBar::mouseUp(const juce::MouseEvent& /*event*/)
{
    if (isDraggingProgressBar)
    {
        isDraggingProgressBar = false;

        if (onSeek)
            onSeek(dragPosition);

        repaint();
    }
}

void TransportBar::mouseMove(const juce::MouseEvent& event)
{
    auto newHover = getHoverStateAt(event.getPosition());
    if (newHover != currentHover)
    {
        currentHover = newHover;
        repaint();
    }
}

void TransportBar::mouseExit(const juce::MouseEvent& /*event*/)
{
    if (currentHover != HoverState::None)
    {
        currentHover = HoverState::None;
        repaint();
    }
}

//==============================================================================
void TransportBar::changeListenerCallback(juce::ChangeBroadcaster* /*source*/)
{
    repaint();
}

//==============================================================================
void TransportBar::drawButton(juce::Graphics& g, juce::Rectangle<float> bounds,
                               ButtonIcon icon, bool isHovered, bool enabled)
{
    const bool dark = ThemeManager::getInstance().isDark();
    if (enabled && isHovered)
    {
        g.setColour(dark ? juce::Colours::white.withAlpha(0.11f)
                         : juce::Colours::black.withAlpha(0.07f));
        g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);
    }

    g.setColour(!enabled ? (dark ? juce::Colour(0xFFA4ABA4) : juce::Colour(0xFF706C66))
                         : isHovered ? (dark ? juce::Colour(0xFFE4868B) : juce::Colour(0xFFB8414D))
                                     : (dark ? Theme::Dark::TextPrimary : juce::Colour(0xFF292725)));
    const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
    juce::Path glyph;
    switch (icon)
    {
        case ButtonIcon::SkipBack:
            g.fillRoundedRectangle(cx - 7.0f, cy - 5.5f, 2.0f, 11.0f, 0.6f);
            glyph.addTriangle(cx + 5.0f, cy - 5.8f, cx - 4.0f, cy, cx + 5.0f, cy + 5.8f);
            break;
        case ButtonIcon::Play:
            glyph.addTriangle(cx - 4.5f, cy - 6.0f, cx + 6.0f, cy, cx - 4.5f, cy + 6.0f);
            break;
        case ButtonIcon::Pause:
            g.fillRoundedRectangle(cx - 5.0f, cy - 5.7f, 3.0f, 11.4f, 0.7f);
            g.fillRoundedRectangle(cx + 2.0f, cy - 5.7f, 3.0f, 11.4f, 0.7f);
            break;
        case ButtonIcon::SkipForward:
            glyph.addTriangle(cx - 5.0f, cy - 5.8f, cx + 4.0f, cy, cx - 5.0f, cy + 5.8f);
            g.fillRoundedRectangle(cx + 5.0f, cy - 5.5f, 2.0f, 11.0f, 0.6f);
            break;
    }
    g.fillPath(glyph);
}

void TransportBar::drawTimeLabel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    juce::String timeText = formatTime(currentPositionSeconds) + " / " + formatTime(totalDurationSeconds);

    bool isDark = theme.isDark();
    juce::Colour timeColour = isEnabled
        ? (isDark ? Theme::Dark::TextSecondary : juce::Colour(0xFF5F5B55))
        : (isDark ? juce::Colour(0xFFA4ABA4) : juce::Colour(0xFF706C66));
    g.setColour(timeColour);
    g.setFont(FontManager::getInstance().getMedium(14.0f));
    g.drawText(timeText, bounds, juce::Justification::centredLeft);
}

void TransportBar::drawProgressBar(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    bool isDark = theme.isDark();

    // Keep the original 8px seek hit area; draw a quieter 4px track inside it.
    auto track = bounds.withSizeKeepingCentre(bounds.getWidth(), 4.0f);
    g.setColour(isDark ? juce::Colour(0xFF828982) : juce::Colour(0xFFA9A49C));
    g.fillRoundedRectangle(track, 2.0f);

    if (!isEnabled || totalDurationSeconds <= 0)
        return;

    double progress = isDraggingProgressBar ? dragPosition : (currentPositionSeconds / totalDurationSeconds);
    progress = juce::jlimit(0.0, 1.0, progress);

    float fillWidth = static_cast<float>(bounds.getWidth() * progress);
    fillWidth = juce::jmin(fillWidth, bounds.getWidth());  // Don't exceed track
    auto fillBounds = track.withWidth(std::max(track.getHeight(), fillWidth));

    g.setColour(isDark ? juce::Colour(0xFFE4868B) : juce::Colour(0xFFB8414D));
    g.fillRoundedRectangle(fillBounds, 2.0f);

    // Small outlined thumb makes seek available without a large white disc.
    bool isHoveringProgressBar = currentHover == HoverState::ProgressBar || isDraggingProgressBar;
    float knobSize = isHoveringProgressBar ? 11.0f : 9.0f;
    float knobHalf = knobSize / 2.0f;
    // Clamp knob within track bounds so it doesn't overflow
    float knobX = bounds.getX() + fillWidth;
    knobX = juce::jlimit(bounds.getX() + knobHalf, bounds.getRight() - knobHalf, knobX);
    auto knobBounds = juce::Rectangle<float>(knobX - knobSize / 2, bounds.getCentreY() - knobSize / 2,
                                              knobSize, knobSize);

    g.setColour(isDark ? Theme::Dark::HeaderSurface : juce::Colour(0xFFF5F0E8));
    g.fillEllipse(knobBounds);
    g.setColour(isDark ? juce::Colour(0xFFE4868B) : juce::Colour(0xFFB8414D));
    g.drawEllipse(knobBounds, 1.6f);
}

//==============================================================================
juce::String TransportBar::formatTime(double seconds) const
{
    if (seconds < 0 || std::isnan(seconds) || std::isinf(seconds))
        seconds = 0;

    int totalSeconds = static_cast<int>(seconds);
    int minutes = totalSeconds / 60;
    int secs = totalSeconds % 60;

    return juce::String::formatted("%02d:%02d", minutes, secs);
}

TransportBar::HoverState TransportBar::getHoverStateAt(juce::Point<int> position) const
{
    if (loopBounds.contains(position))
        return HoverState::Loop;
    if (skipBackBounds.contains(position))
        return HoverState::SkipBack;
    if (playPauseBounds.contains(position))
        return HoverState::PlayPause;
    if (skipForwardBounds.contains(position))
        return HoverState::SkipForward;
    if (progressBarBounds.expanded(0, 8).contains(position))
        return HoverState::ProgressBar;
    return HoverState::None;
}

double TransportBar::getProgressBarPosition(int x) const
{
    double pos = static_cast<double>(x - progressBarBounds.getX()) / progressBarBounds.getWidth();
    return juce::jlimit(0.0, 1.0, pos);
}

} // namespace BlindCard
