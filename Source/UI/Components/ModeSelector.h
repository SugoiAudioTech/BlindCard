/*
  ==============================================================================

    ModeSelector.h
    Created: 2026-01-19
    Author:  BlindCard

    Mode selector component for switching between Stars/Guess/Q&A modes.
    Width: ~280px, Height: 40px

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "../Theme/ThemeManager.h"
#include "../../Core/Types.h"

namespace BlindCard
{

//==============================================================================
/**
 * ModeSelector component displays rating mode options as a segmented control.
 *
 * Layout:
 * +----------------------------------+
 * |  * Stars  |  @ Guess  |  / Q&A  |
 * +----------------------------------+
 *
 * Usage:
 *   ModeSelector selector;
 *   selector.onModeChanged = [this](blindcard::RatingMode mode) { handleModeChange(mode); };
 *   selector.setMode(blindcard::RatingMode::Stars);
 *   selector.setLocked(true); // During BlindTesting phase
 */
class ModeSelector : public juce::Component,
                     public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    ModeSelector();

    /** Destructor */
    ~ModeSelector() override;

    //==========================================================================
    /** Callback triggered when mode selection changes */
    std::function<void(blindcard::RatingMode)> onModeChanged;

    //==========================================================================
    /**
     * Sets the current rating mode.
     * @param mode The mode to select
     */
    void setMode(blindcard::RatingMode mode);

    /** Returns the currently selected mode */
    blindcard::RatingMode getMode() const { return currentMode; }

    /**
     * Locks or unlocks the selector.
     * When locked, mode changes are prevented and a lock icon is shown.
     * @param locked true to lock, false to unlock
     */
    void setLocked(bool locked);

    /** Returns true if the selector is locked */
    bool isLocked() const { return lockedState; }

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Standard dimensions */
    static constexpr int kWidth = 280;
    static constexpr int kHeight = 40;

private:
    //==========================================================================
    // Mode state
    blindcard::RatingMode currentMode = blindcard::RatingMode::Stars;
    bool lockedState = false;

    //==========================================================================
    // Button hit areas (computed in resized())
    juce::Rectangle<int> starsBounds;
    juce::Rectangle<int> guessBounds;
    juce::Rectangle<int> qaBounds;

    // Hover state
    enum class HoverState
    {
        None,
        Stars,
        Guess,
        QA
    };
    HoverState currentHover = HoverState::None;

    //==========================================================================
    // Drawing helpers
    void drawModeOption(juce::Graphics& g,
                        juce::Rectangle<float> bounds,
                        const juce::String& icon,
                        const juce::String& label,
                        bool isSelected,
                        bool isHovered);

    void drawLockOverlay(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Utility
    HoverState getHoverStateAt(juce::Point<int> position) const;
    blindcard::RatingMode hoverStateToMode(HoverState state) const;
    juce::Rectangle<int> getModeButtonBounds(blindcard::RatingMode mode) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelector)
};

} // namespace BlindCard
