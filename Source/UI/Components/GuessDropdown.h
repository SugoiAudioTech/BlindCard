/*
  ==============================================================================

    GuessDropdown.h
    Created: 2026-01-19
    Author:  BlindCard

    Dropdown component for track guessing in Guess mode.
    Displays a styled dropdown with track names for selection.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>
#include <string>
#include "../Theme/ThemeManager.h"

namespace BlindCard
{

//==============================================================================
/**
 * GuessDropdown component displays a dropdown for selecting tracks in Guess mode.
 *
 * Visual structure:
 * +----------------------+
 * |  Guess Track...   v  |  <- closed state (v = down arrow)
 * +----------------------+
 *
 * +----------------------+
 * |  Guess Track...   ^  |  <- open state (^ = up arrow)
 * +----------------------+
 * |  Track A             |
 * |  Track B             |
 * |  Track C             |
 * +----------------------+
 *
 * Usage:
 *   GuessDropdown dropdown;
 *   dropdown.setTracks({"Track A", "Track B", "Track C"});
 *   dropdown.onSelectionChanged = [this](int index) { handleSelection(index); };
 *   dropdown.setSelectedIndex(-1); // No selection (shows placeholder)
 */
class GuessDropdown : public juce::Component,
                      public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    GuessDropdown();

    /** Destructor */
    ~GuessDropdown() override;

    //==========================================================================
    /** Callback triggered when selection changes */
    std::function<void(int)> onSelectionChanged;

    //==========================================================================
    /**
     * Sets the list of track names to display in the dropdown.
     * Resets selection to -1 (no selection).
     * @param trackNames Vector of track name strings
     */
    void setTracks(const std::vector<std::string>& trackNames);

    /**
     * Sets the selected track index.
     * @param index The index to select, or -1 for no selection (shows placeholder)
     */
    void setSelectedIndex(int index);

    /** Returns the currently selected index, or -1 if no selection */
    int getSelectedIndex() const { return selectedIndex; }

    /** Returns the currently selected track name, or empty string if no selection */
    std::string getSelectedTrackName() const;

    /**
     * Sets the enabled state of the dropdown.
     * When disabled, clicking has no effect and appearance is dimmed.
     * @param shouldBeEnabled true to enable, false to disable
     */
    void setEnabled(bool shouldBeEnabled);

    /** Returns true if the dropdown is enabled */
    bool isEnabled() const { return enabledState; }

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Standard dimensions */
    static constexpr int kDefaultWidth = 200;
    static constexpr int kHeight = 36;
    static constexpr int kCornerRadius = 4;
    static constexpr int kBorderWidth = 1;
    static constexpr int kArrowPadding = 12;
    static constexpr int kTextPadding = 12;

private:
    //==========================================================================
    // State
    std::vector<std::string> tracks;
    int selectedIndex = -1;
    bool enabledState = true;
    bool isHovered = false;
    bool isMenuOpen = false;

    //==========================================================================
    // Visual constants
    static constexpr float kDisabledOpacity = 0.4f;
    static constexpr float kFontSize = 14.0f;

    //==========================================================================
    // Placeholder text
    const juce::String placeholderText = "Guess Track...";

    //==========================================================================
    // Drawing helpers
    void drawBackground(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawBorder(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawText(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawArrow(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Menu handling
    void showPopupMenu();
    void handleMenuResult(int result);

    // Colors
    juce::Colour getBackgroundColour() const;
    juce::Colour getBorderColour() const;
    juce::Colour getTextColour() const;
    juce::Colour getArrowColour() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuessDropdown)
};

} // namespace BlindCard
