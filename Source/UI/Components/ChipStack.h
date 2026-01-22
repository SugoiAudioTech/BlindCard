/*
  ==============================================================================

    ChipStack.h
    Created: 2026-01-19
    Author:  BlindCard

    Decorative stacked poker chips for the PokerTable visual.
    Renders multiple chips stacked vertically with alternating colors.

    Default configurations:
    - Left stack:  5 chips (red/black alternating, starting with red)
    - Right stack: 4 chips (black/red alternating, starting with black)

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/ThemeManager.h"

namespace BlindCard
{

//==============================================================================
/**
 * ChipStack displays a vertical stack of decorative poker chips.
 *
 * Visual structure (per chip, bottom to top):
 * 1. Outer ring with cream stripes (similar to ChipButton)
 * 2. Gold border line
 * 3. Inner circle with base color
 *
 * The chips are rendered with perspective (larger at bottom, smaller at top)
 * to create a 3D stacking effect.
 *
 * Usage:
 *   ChipStack leftStack(5, true);   // 5 chips, red first
 *   ChipStack rightStack(4, false); // 4 chips, black first
 */
class ChipStack : public juce::Component,
                  public juce::ChangeListener
{
public:
    //==========================================================================
    /**
     * Constructor.
     * @param chipCount Number of chips in the stack
     * @param startWithRed If true, bottom chip is red; otherwise black
     */
    ChipStack(int chipCount = 5, bool startWithRed = true);

    /** Destructor */
    ~ChipStack() override;

    //==========================================================================
    /**
     * Sets the number of chips in the stack.
     * @param count Number of chips (1-10)
     */
    void setChipCount(int count);

    /** Returns the number of chips */
    int getChipCount() const { return numChips; }

    /**
     * Sets whether the stack starts with red (bottom chip).
     * @param redFirst true for red at bottom, false for black
     */
    void setStartsWithRed(bool redFirst);

    /** Returns true if the bottom chip is red */
    bool startsWithRed() const { return startRed; }

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Dimensions */
    static constexpr int kChipDiameter = 48;
    static constexpr int kChipThickness = 8;
    static constexpr int kStripeCount = 8;

private:
    //==========================================================================
    // State
    int numChips;
    bool startRed;

    //==========================================================================
    // Drawing helpers
    void drawChipWithEdge(juce::Graphics& g, juce::Rectangle<float> bounds, bool isRed, int stackIndex);
    void drawChipTop(juce::Graphics& g, juce::Rectangle<float> bounds, bool isRed);

    // Color helpers
    juce::Colour getChipColor(bool isRed) const;
    juce::Colour getDarkChipColor(bool isRed) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChipStack)
};

} // namespace BlindCard
