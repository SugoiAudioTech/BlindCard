/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    HeaderBar.h
    Created: 2026-01-19
    Author:  BlindCard

    Header bar component showing logo, connection status, and controls.
    Height: 56px, spans full width of plugin window.

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
 * HeaderBar component displays the plugin branding, connection status,
 * and control buttons (theme toggle, info, settings).
 *
 * Layout:
 * ┌─────────────────────────────────────────────────────────────┐
 * │ [Logo]  BlindCard          [Connected ●]    [🌙] [ℹ] [⚙]  │
 * │         BY SUGOI AUDIO                                      │
 * └─────────────────────────────────────────────────────────────┘
 *
 * Usage:
 *   HeaderBar header;
 *   header.onSettingsClicked = [this]() { showSettings(); };
 *   header.onInfoClicked = [this]() { showInfo(); };
 *   header.onThemeToggled = [this]() { theme.toggleTheme(); };
 *   header.setConnected(true);
 */
class HeaderBar : public juce::Component,
                  public juce::ChangeListener,
                  public juce::Timer
{
public:
    //==========================================================================
    /** Constructor */
    HeaderBar();

    /** Destructor */
    ~HeaderBar() override;

    //==========================================================================
    /** Callback triggered when the settings button is clicked */
    std::function<void()> onSettingsClicked;

    /** Callback triggered when the info button is clicked */
    std::function<void()> onInfoClicked;

    /** Callback triggered when the theme toggle is clicked */
    std::function<void()> onThemeToggled;

    /** Callback triggered when the reset button is clicked */
    std::function<void()> onResetClicked;

    //==========================================================================
    /**
     * Sets the current playing track info for display.
     * @param rmsDb Current RMS level in dB (-100 for silence)
     * @param trackName Track name or "Card X" during blind testing
     */
    void setCurrentTrackInfo(float rmsDb, const juce::String& trackName);

    /** Returns the current RMS level in dB */
    float getCurrentRMS() const { return currentRMSdB; }

    /** Returns the current track name */
    juce::String getCurrentTrackName() const { return currentTrackName; }

    /** Show/hide update notification dot on the settings gear icon */
    void setUpdateAvailable(bool available);

    /**
     * Sets standalone mode - hides "Now Playing" info to save space for TransportBar.
     * @param enabled true to hide track info display
     */
    void setStandaloneMode(bool enabled);

    /** Returns true if standalone mode is enabled */
    bool isStandaloneMode() const { return standaloneMode; }

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Timer override (for pulse animation)
    void timerCallback() override;

    //==========================================================================
    /** Standard height for the header bar */
    static constexpr int kHeight = 56;

private:
    //==========================================================================
    // State
    bool standaloneMode = false;
    bool showUpdateDot = false;

    // Current track info
    float currentRMSdB = -100.0f;
    juce::String currentTrackName;

    // Pulse animation (for level indicator)
    float pulsePhase = 0.0f;
    static constexpr float kPulsePeriodMs = 1500.0f;
    static constexpr int kTimerIntervalMs = 16; // ~60fps

    //==========================================================================
    // Button hit areas (computed in resized())
    juce::Rectangle<int> themeToggleBounds;
    juce::Rectangle<int> infoButtonBounds;
    juce::Rectangle<int> settingsButtonBounds;
    juce::Rectangle<int> resetButtonBounds;
    juce::Rectangle<int> trackInfoBounds;

    // Hover states
    enum class HoverState
    {
        None,
        ThemeToggle,
        InfoButton,
        SettingsButton,
        ResetButton
    };
    HoverState currentHover = HoverState::None;

    //==========================================================================
    // Drawing helpers
    void drawLogo(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawBrandText(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawTrackInfo(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawThemeToggle(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered);
    void drawInfoButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered);
    void drawSettingsButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered);
    void drawResetButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered);

    // Utility
    HoverState getHoverStateAt(juce::Point<int> position) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};

} // namespace BlindCard
