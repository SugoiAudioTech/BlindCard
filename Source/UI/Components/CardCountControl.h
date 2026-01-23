/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    CardCountControl.h
    Created: 2025-01-21
    Author:  BlindCard

    Card count control for Standalone mode.
    Allows users to add/remove cards (2-8 cards).

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
 * CardCountControl provides +/- buttons to adjust the number of cards.
 *
 * Layout:
 * Cards: [－] 4 [＋]
 *
 * Used only in Standalone mode where the number of cards is user-controlled
 * (unlike plugin mode where it's determined by DAW tracks).
 */
class CardCountControl : public juce::Component,
                         public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    CardCountControl();

    /** Destructor */
    ~CardCountControl() override;

    //==========================================================================
    // Callbacks

    /** Called when the card count changes */
    std::function<void(int newCount)> onCountChanged;

    //==========================================================================
    /**
     * Set the current card count.
     * @param count The number of cards (will be clamped to 2-8)
     */
    void setCount(int count);

    /** Get the current card count */
    int getCount() const { return currentCount; }

    /**
     * Set the minimum card count.
     * @param min Minimum value (default: 2)
     */
    void setMinCount(int min) { minCount = juce::jlimit(1, 7, min); }

    /**
     * Set the maximum card count.
     * @param max Maximum value (default: 8)
     */
    void setMaxCount(int max) { maxCount = juce::jlimit(2, 8, max); }

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    static constexpr int kPreferredHeight = 32;

private:
    //==========================================================================
    int currentCount = 2;
    int minCount = 2;
    int maxCount = 8;

    // Hit areas
    juce::Rectangle<int> decrementBounds;
    juce::Rectangle<int> incrementBounds;
    juce::Rectangle<int> labelBounds;
    juce::Rectangle<int> valueBounds;

    // Hover state
    enum class HoverState { None, Decrement, Increment };
    HoverState currentHover = HoverState::None;

    //==========================================================================
    void increment();
    void decrement();
    HoverState getHoverStateAt(juce::Point<int> position) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CardCountControl)
};

} // namespace BlindCard
