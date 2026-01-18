/*
  ==============================================================================

    ControlPanel.h
    Created: 2026-01-19
    Author:  BlindCard

    Control panel component containing tracks display, sliders, toggles,
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
 * Layout:
 * +----------------------------------------------------------+
 * | TRACKS                           |  SETTINGS             |
 * | 1. Plugin A                      |  Rounds: [===*===]  4 |
 * | 2. Plugin B                      |  Auto Gain: [ON/OFF]  |
 * | 3. Plugin C                      |                       |
 * | 4. Plugin D                      |                       |
 * +----------------------------------------------------------+
 * |        [SHUFFLE]  [REVEAL]  [RESET]  [NEXT]              |
 * +----------------------------------------------------------+
 *
 * Components:
 * - Tracks list (read-only display of loaded tracks)
 * - Rounds slider (2-8 rounds)
 * - Auto Gain toggle
 * - Action buttons: Shuffle, Reveal, Reset, Next Round
 *
 * Usage:
 *   ControlPanel panel;
 *   panel.setTracks(trackNames);
 *   panel.onShuffleClicked = [this]() { shuffle(); };
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
     * @param rounds Number of rounds (2-8)
     */
    void setRounds(int rounds);

    /** Returns the current number of rounds */
    int getRounds() const;

    /**
     * Sets the auto gain state.
     * @param enabled true to enable auto gain
     */
    void setAutoGain(bool enabled);

    /** Returns the auto gain state */
    bool getAutoGain() const { return autoGainEnabled; }

    //==========================================================================
    // Button state control

    /**
     * Sets the shuffle button enabled state.
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
     * Sets the next round button enabled state.
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
    static constexpr int kMinHeight = 300;
    static constexpr int kButtonAreaHeight = 100;

private:
    //==========================================================================
    // State
    std::vector<std::string> trackNames;
    bool autoGainEnabled = true;

    //==========================================================================
    // Child components
    std::unique_ptr<juce::Slider> roundsSlider;
    std::unique_ptr<juce::ToggleButton> autoGainToggle;

    std::unique_ptr<ChipButton> shuffleButton;
    std::unique_ptr<ChipButton> revealButton;
    std::unique_ptr<ChipButton> resetButton;
    std::unique_ptr<ChipButton> nextRoundButton;

    //==========================================================================
    // Custom slider style
    class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle, juce::Slider& slider) override;
    };
    std::unique_ptr<CustomSliderLookAndFeel> sliderLookAndFeel;

    //==========================================================================
    // Drawing helpers
    void drawSectionBackground(juce::Graphics& g, juce::Rectangle<float> bounds,
                               const juce::String& title);
    void drawTracksList(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawToggleButton(juce::Graphics& g, juce::Rectangle<float> bounds,
                          const juce::String& label, bool isOn);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};

} // namespace BlindCard
