/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    ControlPanel.h
    Created: 2026-01-19
    Author:  BlindCard

    Control panel component containing tracks info, rounds slider, auto gain toggle,
    and action buttons for the BlindCard game.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <string>
#include <functional>
#include "../Theme/ThemeManager.h"
#include "ChipButton.h"

namespace BlindCard
{

//==============================================================================
/**
 * ControlPanel provides game controls and settings.
 *
 * Layout (matching original design):
 * +----------------------------------------------------------+
 * | ♪ Current Tracks [4]    # Rounds [====*] 1/3             |
 * +----------------------------------------------------------+
 * | 🔊 Auto Gain                              [==ON==]       |
 * +----------------------------------------------------------+
 * |              [REVEAL]              [RESET]               |
 * +----------------------------------------------------------+
 *
 * Components:
 * - Current Tracks display with count
 * - Rounds slider with counter display
 * - Auto Gain toggle switch
 * - Action buttons: Reveal, Reset (Shuffle and Next shown contextually)
 *
 * Usage:
 *   ControlPanel panel;
 *   panel.setTracks(trackNames);
 *   panel.onRevealClicked = [this]() { reveal(); };
 *   panel.onRoundsChanged = [this](int rounds) { setRounds(rounds); };
 */
class ControlPanel : public juce::Component,
                     public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    ControlPanel();

    /** Destructor */
    ~ControlPanel() override;

    //==========================================================================
    // Callbacks
    std::function<void()> onShuffleClicked;
    std::function<void()> onRevealClicked;
    std::function<void()> onResetClicked;
    std::function<void()> onNextRoundClicked;
    std::function<void(int)> onRoundsChanged;       // Number of rounds
    std::function<void(bool)> onAutoGainChanged;    // Auto gain toggle
    std::function<void(int)> onQAQuestionsChanged;  // Q&A question count

    //==========================================================================
    /**
     * Sets the track names to display.
     * @param tracks Vector of track names
     */
    void setTracks(const std::vector<std::string>& tracks);

    /** Returns the current track names */
    const std::vector<std::string>& getTracks() const { return trackNames; }

    /**
     * Sets the number of rounds.
     * @param rounds Number of rounds (1-8)
     */
    void setRounds(int rounds);

    /** Returns the current number of rounds */
    int getRounds() const;

    /**
     * Sets the current round number (1-based).
     * @param round Current round number
     */
    void setCurrentRound(int round);

    /** Returns the current round number */
    int getCurrentRound() const { return currentRound; }

    /**
     * Sets the auto gain state.
     * @param enabled true to enable auto gain
     */
    void setAutoGain(bool enabled);

    /** Returns the auto gain state */
    bool getAutoGain() const { return autoGainEnabled; }

    /**
     * Sets the calibration status for Level-Match.
     * @param calibrating true if currently calibrating
     * @param calibrated true if calibration is complete
     * @param timeRemaining seconds remaining for calibration
     */
    void setCalibrationStatus(bool calibrating, bool calibrated, float timeRemaining = 0.0f);

    /** Returns true if currently calibrating */
    bool isCalibrating() const { return calibratingStatus; }

    /** Returns true if calibration is complete */
    bool isCalibrated() const { return calibratedStatus; }

    //==========================================================================
    // Standalone Mode

    /**
     * Sets standalone mode (hides Level-Match since it requires DAW audio).
     * @param standalone true if running in Standalone mode
     */
    void setStandaloneMode(bool standalone);

    //==========================================================================
    // Q&A Mode Settings

    /**
     * Sets the Q&A question count.
     * @param count Number of questions (1-8)
     */
    void setQAQuestions(int count);

    /** Returns the current Q&A question count */
    int getQAQuestions() const;

    /**
     * Sets whether Q&A mode is active (shows/hides Q&A slider).
     * @param isQA true if Q&A mode is active
     */
    void setQAMode(bool isQA);

    //==========================================================================
    // Button state control

    /**
     * Sets the shuffle button enabled/visible state.
     */
    void setShuffleEnabled(bool enabled);

    /**
     * Sets the reveal button enabled state.
     */
    void setRevealEnabled(bool enabled);

    /**
     * Sets the reset button enabled state.
     */
    void setResetEnabled(bool enabled);

    /**
     * Sets the next round button enabled/visible state.
     */
    void setNextRoundEnabled(bool enabled);

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Dimensions */
    static constexpr int kPreferredWidth = 320;
    static constexpr int kMinHeight = 220;  // ROUND indicator + info row + buttons
    static constexpr int kButtonAreaHeight = 100;

private:
    //==========================================================================
    // State
    std::vector<std::string> trackNames;
    bool autoGainEnabled = false;  // Default off
    bool calibratingStatus = false;
    bool calibratedStatus = false;
    float calibrationTimeRemaining = 0.0f;  // Remaining seconds
    int currentRound = 1;  // 1-based current round number
    bool isQAModeActive = false;  // Whether Q&A mode is active
    bool isStandaloneModeActive = false;  // Whether Standalone mode (hides Level-Match)

    // Layout constants
    static constexpr int sliderWidth = 100;

    //==========================================================================
    // Child components
    std::unique_ptr<juce::Slider> roundsSlider;
    std::unique_ptr<juce::Slider> qaQuestionsSlider;  // Q&A question count slider
    std::unique_ptr<juce::ToggleButton> autoGainToggle;

    std::unique_ptr<ChipButton> shuffleButton;
    std::unique_ptr<ChipButton> revealButton;
    std::unique_ptr<ChipButton> resetButton;
    std::unique_ptr<ChipButton> nextRoundButton;

    //==========================================================================
    // Custom slider look and feel
    class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle, juce::Slider& slider) override;
    };
    std::unique_ptr<CustomSliderLookAndFeel> sliderLookAndFeel;

    //==========================================================================
    // Custom toggle look and feel
    class CustomToggleLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    };
    std::unique_ptr<CustomToggleLookAndFeel> toggleLookAndFeel;

    //==========================================================================
    // Drawing helpers
    void drawRoundIndicator(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawInfoRow(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawAutoGainRow(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawQARow(juce::Graphics& g, juce::Rectangle<float> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};

} // namespace BlindCard
