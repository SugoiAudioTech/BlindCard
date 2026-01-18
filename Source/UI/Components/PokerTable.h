/*
  ==============================================================================

    PokerTable.h
    Created: 2026-01-19
    Author:  BlindCard

    Main poker table component displaying cards in a casino-style layout.
    Features a felt surface with wood frame and neon border accents.

    Layout:
    - 2-4 cards: single row, centered
    - 5-8 cards: double row (4+4 or similar)

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <memory>
#include "../Theme/ThemeManager.h"
#include "PokerCard.h"
#include "ChipStack.h"

namespace BlindCard
{

//==============================================================================
/**
 * PokerTable displays the main game area with poker-style visual design.
 *
 * Visual Structure (outside to inside):
 * 1. Dark padded frame (gradient dark gray)
 * 2. Table rail (dark padding)
 * 3. Neon red border (with glow effect)
 * 4. Dark felt surface (radial gradient)
 * 5. Decorative chip stacks (left and right sides)
 * 6. Card positions arranged in rows
 *
 * Card Layout Rules:
 * - 2 cards: 1 row, centered with gap
 * - 3 cards: 1 row, centered
 * - 4 cards: 1 row, evenly spaced
 * - 5-8 cards: 2 rows (4 top, remaining bottom)
 *
 * Usage:
 *   PokerTable table;
 *   table.setCardCount(4);
 *   table.setMode(RatingMode::Stars);
 *   table.getCard(0)->setCardData(data);
 */
class PokerTable : public juce::Component,
                   public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    PokerTable();

    /** Destructor */
    ~PokerTable() override;

    //==========================================================================
    // Card management

    /**
     * Sets the number of cards to display (2-8).
     * Cards are automatically created/destroyed as needed.
     * @param count Number of cards (clamped to 2-8)
     */
    void setCardCount(int count);

    /** Returns the current number of cards */
    int getCardCount() const { return static_cast<int>(cards.size()); }

    /**
     * Gets a specific card by index.
     * @param index Card index (0-based)
     * @return Pointer to the PokerCard, or nullptr if invalid index
     */
    PokerCard* getCard(int index);

    /** Gets all cards */
    const std::vector<std::unique_ptr<PokerCard>>& getCards() const { return cards; }

    //==========================================================================
    // Mode and phase

    /**
     * Sets the rating mode for all cards.
     * @param mode Stars, Guess, or QA
     */
    void setMode(blindcard::RatingMode mode);

    /** Returns the current mode */
    blindcard::RatingMode getMode() const { return currentMode; }

    /**
     * Sets the game phase for all cards.
     * @param phase Setup, BlindTesting, or Revealed
     */
    void setPhase(blindcard::GamePhase phase);

    /** Returns the current phase */
    blindcard::GamePhase getPhase() const { return currentPhase; }

    /**
     * Sets tracks for all GuessDropdowns.
     * @param tracks Vector of track names
     */
    void setTracks(const std::vector<std::string>& tracks);

    //==========================================================================
    // Actions

    /**
     * Reveals all cards with staggered animation.
     * @param delayBetweenCardsMs Delay between each card flip (default 100ms)
     */
    void revealAllCards(int delayBetweenCardsMs = 100);

    /**
     * Hides all cards (flips back).
     */
    void hideAllCards();

    /**
     * Shuffles and resets cards to hidden state.
     */
    void shuffleCards();

    //==========================================================================
    // Callbacks (forwarded from cards)
    std::function<void(int)> onCardClicked;               // card index
    std::function<void(int, int)> onCardRatingChanged;    // card index, rating
    std::function<void(int, int)> onCardGuessChanged;     // card index, track index
    std::function<void(int)> onCardQASelected;            // card index

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Dimensions */
    static constexpr int kMinWidth = 600;
    static constexpr int kMinHeight = 300;
    static constexpr int kFrameThickness = 20;
    static constexpr int kRailThickness = 8;
    static constexpr int kNeonBorderWidth = 3;

private:
    //==========================================================================
    // State
    blindcard::RatingMode currentMode = blindcard::RatingMode::Stars;
    blindcard::GamePhase currentPhase = blindcard::GamePhase::Setup;
    std::vector<std::string> trackList;

    //==========================================================================
    // Child components
    std::vector<std::unique_ptr<PokerCard>> cards;
    std::unique_ptr<ChipStack> leftChipStack;
    std::unique_ptr<ChipStack> rightChipStack;

    //==========================================================================
    // Drawing helpers
    void drawWoodFrame(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawTableRail(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawNeonBorder(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawFeltSurface(juce::Graphics& g, juce::Rectangle<float> bounds);

    //==========================================================================
    // Layout helpers
    void updateCardLayout();
    juce::Rectangle<int> getFeltArea() const;

    // Card callback setup
    void setupCardCallbacks(PokerCard& card, int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PokerTable)
};

} // namespace BlindCard
