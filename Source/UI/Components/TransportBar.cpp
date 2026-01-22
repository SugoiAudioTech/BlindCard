/*
  ==============================================================================

    TransportBar.cpp
    Created: 2025-01-21
    Author:  BlindCard

  ==============================================================================
*/

#include "TransportBar.h"
#include "../Theme/FontManager.h"

namespace BlindCard
{

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
    if (!isDraggingProgressBar && currentPositionSeconds != positionSeconds)
    {
        currentPositionSeconds = positionSeconds;
        repaint();
    }
}

void TransportBar::setDuration(double durationSeconds)
{
    if (totalDurationSeconds != durationSeconds)
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

//==============================================================================
void TransportBar::paint(juce::Graphics& g)
{
    auto& theme = ThemeManager::getInstance();

    // Draw buttons - use simple ASCII symbols for better font compatibility
    drawButton(g, skipBackBounds.toFloat(), "<<",
               currentHover == HoverState::SkipBack, isEnabled);

    // Play/Pause button
    juce::String playSymbol = isPlaying ? "||" : ">";
    drawButton(g, playPauseBounds.toFloat(), playSymbol,
               currentHover == HoverState::PlayPause, isEnabled);

    drawButton(g, skipForwardBounds.toFloat(), ">>",
               currentHover == HoverState::SkipForward, isEnabled);

    // Draw time label
    drawTimeLabel(g, timeLabelBounds.toFloat());

    // Draw progress bar
    drawProgressBar(g, progressBarBounds.toFloat());
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds();
    int x = 0;

    // Button sizes
    const int buttonSize = 24;
    const int buttonSpacing = 4;
    const int timeLabelWidth = 85;
    const int progressBarWidth = 100;

    // Skip back button
    skipBackBounds = juce::Rectangle<int>(x, (bounds.getHeight() - buttonSize) / 2,
                                           buttonSize, buttonSize);
    x += buttonSize + buttonSpacing;

    // Play/pause button
    playPauseBounds = juce::Rectangle<int>(x, (bounds.getHeight() - buttonSize) / 2,
                                            buttonSize, buttonSize);
    x += buttonSize + buttonSpacing;

    // Skip forward button
    skipForwardBounds = juce::Rectangle<int>(x, (bounds.getHeight() - buttonSize) / 2,
                                              buttonSize, buttonSize);
    x += buttonSize + 8;

    // Time label
    timeLabelBounds = juce::Rectangle<int>(x, 0, timeLabelWidth, bounds.getHeight());
    x += timeLabelWidth + 8;

    // Progress bar (remaining width)
    int remainingWidth = bounds.getWidth() - x;
    progressBarBounds = juce::Rectangle<int>(x, (bounds.getHeight() - 8) / 2,
                                              std::max(50, remainingWidth), 8);
}

//==============================================================================
void TransportBar::mouseDown(const juce::MouseEvent& event)
{
    if (!isEnabled)
        return;

    auto pos = event.getPosition();

    if (skipBackBounds.contains(pos))
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

void TransportBar::mouseUp(const juce::MouseEvent& event)
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
                               const juce::String& symbol, bool isHovered, bool enabled)
{
    auto& theme = ThemeManager::getInstance();

    // Background
    juce::Colour bgColor = enabled
        ? (isHovered ? theme.getColour(ColourId::Accent).withAlpha(0.2f) : juce::Colours::transparentBlack)
        : juce::Colours::transparentBlack;

    if (bgColor.getAlpha() > 0)
    {
        g.setColour(bgColor);
        g.fillRoundedRectangle(bounds, 4.0f);
    }

    // Symbol
    juce::Colour textColor = enabled
        ? (isHovered ? theme.getColour(ColourId::Accent) : theme.getColour(ColourId::TextPrimary))
        : theme.getColour(ColourId::TextPrimary).withAlpha(0.3f);

    g.setColour(textColor);
    g.setFont(FontManager::getInstance().getBebasNeue(14.0f));
    g.drawText(symbol, bounds, juce::Justification::centred);
}

void TransportBar::drawTimeLabel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    juce::String timeText = formatTime(currentPositionSeconds) + " / " + formatTime(totalDurationSeconds);

    g.setColour(isEnabled ? theme.getColour(ColourId::TextPrimary) : theme.getColour(ColourId::TextPrimary).withAlpha(0.3f));
    g.setFont(FontManager::getInstance().getBebasNeue(12.0f));
    g.drawText(timeText, bounds, juce::Justification::centredLeft);
}

void TransportBar::drawProgressBar(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    // Track background
    g.setColour(theme.isDark() ? juce::Colour(0xFF3A3A3A) : juce::Colour(0xFFCCCCCC));
    g.fillRoundedRectangle(bounds, bounds.getHeight() / 2);

    if (!isEnabled || totalDurationSeconds <= 0)
        return;

    // Progress fill
    double progress = isDraggingProgressBar ? dragPosition : (currentPositionSeconds / totalDurationSeconds);
    progress = juce::jlimit(0.0, 1.0, progress);

    float fillWidth = static_cast<float>(bounds.getWidth() * progress);
    auto fillBounds = bounds.withWidth(std::max(bounds.getHeight(), fillWidth));

    g.setColour(theme.getColour(ColourId::Accent));
    g.fillRoundedRectangle(fillBounds, bounds.getHeight() / 2);

    // Handle/knob
    float knobX = bounds.getX() + fillWidth;
    float knobSize = bounds.getHeight() + 6;
    auto knobBounds = juce::Rectangle<float>(knobX - knobSize / 2, bounds.getCentreY() - knobSize / 2,
                                              knobSize, knobSize);

    bool isHoveringProgressBar = currentHover == HoverState::ProgressBar || isDraggingProgressBar;

    g.setColour(juce::Colours::white);
    g.fillEllipse(knobBounds);

    if (isHoveringProgressBar)
    {
        g.setColour(theme.getColour(ColourId::Accent).withAlpha(0.3f));
        g.fillEllipse(knobBounds.expanded(3));
    }
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
