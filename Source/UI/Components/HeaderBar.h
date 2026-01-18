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

    //==========================================================================
    /**
     * Sets the connection status and updates the display.
     * When connected, shows gold pulsing dot with 1.5s period.
     * @param connected true if connected to a DAW/audio system
     */
    void setConnected(bool connected);

    /** Returns the current connection status */
    bool isConnected() const { return connectionStatus; }

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
    // Connection status
    bool connectionStatus = false;

    // Pulse animation
    float pulsePhase = 0.0f;
    static constexpr float kPulsePeriodMs = 1500.0f;
    static constexpr int kTimerIntervalMs = 16; // ~60fps

    //==========================================================================
    // Button hit areas (computed in resized())
    juce::Rectangle<int> themeToggleBounds;
    juce::Rectangle<int> infoButtonBounds;
    juce::Rectangle<int> settingsButtonBounds;
    juce::Rectangle<int> connectionStatusBounds;

    // Hover states
    enum class HoverState
    {
        None,
        ThemeToggle,
        InfoButton,
        SettingsButton
    };
    HoverState currentHover = HoverState::None;

    //==========================================================================
    // Drawing helpers
    void drawLogo(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawBrandText(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawConnectionStatus(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawThemeToggle(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered);
    void drawInfoButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered);
    void drawSettingsButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered);

    // Utility
    HoverState getHoverStateAt(juce::Point<int> position) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};

} // namespace BlindCard
