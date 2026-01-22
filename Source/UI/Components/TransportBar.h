/*
  ==============================================================================

    TransportBar.h
    Created: 2025-01-21
    Author:  BlindCard

    Transport controls for Standalone mode.
    Includes play/pause, skip, time display, and seekable progress bar.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "../Theme/ThemeManager.h"

namespace BlindCard
{

//==============================================================================
/**
 * TransportBar provides playback controls for Standalone mode.
 *
 * Layout:
 * [◀◀][▶/⏸][▶▶] 01:23 / 03:45 [━━━━●━━━━━]
 *
 * Features:
 * - Play/pause toggle button
 * - Skip backward/forward buttons (5 seconds)
 * - Current time / total time display
 * - Draggable progress bar for seeking
 */
class TransportBar : public juce::Component,
                     public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    TransportBar();

    /** Destructor */
    ~TransportBar() override;

    //==========================================================================
    // Callbacks

    /** Called when play/pause is clicked */
    std::function<void()> onPlayPauseClicked;

    /** Called when skip backward is clicked */
    std::function<void()> onSkipBackward;

    /** Called when skip forward is clicked */
    std::function<void()> onSkipForward;

    /** Called when user seeks to a new position (0.0 to 1.0) */
    std::function<void(double normalizedPosition)> onSeek;

    //==========================================================================
    // State setters

    /**
     * Set whether audio is currently playing.
     * @param playing true if playing, false if paused
     */
    void setPlaying(bool playing);

    /**
     * Set the current playback position.
     * @param positionSeconds Current position in seconds
     */
    void setPosition(double positionSeconds);

    /**
     * Set the total duration.
     * @param durationSeconds Total duration in seconds
     */
    void setDuration(double durationSeconds);

    /**
     * Enable or disable the transport controls.
     * @param enabled true to enable (audio loaded), false to disable
     */
    void setEnabled(bool enabled);

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Recommended width for the transport bar */
    static constexpr int kRecommendedWidth = 280;
    static constexpr int kHeight = 32;

private:
    //==========================================================================
    // State
    bool isPlaying = false;
    bool isEnabled = false;
    double currentPositionSeconds = 0.0;
    double totalDurationSeconds = 0.0;

    // Dragging state
    bool isDraggingProgressBar = false;
    double dragPosition = 0.0;

    //==========================================================================
    // Hit areas (computed in resized())
    juce::Rectangle<int> skipBackBounds;
    juce::Rectangle<int> playPauseBounds;
    juce::Rectangle<int> skipForwardBounds;
    juce::Rectangle<int> timeLabelBounds;
    juce::Rectangle<int> progressBarBounds;

    // Hover state
    enum class HoverState
    {
        None,
        SkipBack,
        PlayPause,
        SkipForward,
        ProgressBar
    };
    HoverState currentHover = HoverState::None;

    //==========================================================================
    // Drawing helpers
    void drawButton(juce::Graphics& g, juce::Rectangle<float> bounds,
                    const juce::String& symbol, bool isHovered, bool isEnabled);
    void drawTimeLabel(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawProgressBar(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Utility
    juce::String formatTime(double seconds) const;
    HoverState getHoverStateAt(juce::Point<int> position) const;
    double getProgressBarPosition(int x) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};

} // namespace BlindCard
