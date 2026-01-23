/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    PokerCard.h
    Created: 2026-01-19
    Author:  BlindCard

    Main poker card component with flip animation and integrated controls.
    Central visual element where each card represents one audio track being tested.

    Default Size: 140x200px (scalable)

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>
#include <string>
#include "../Theme/ThemeManager.h"
#include "../Animation/AnimatedValue.h"
#include "StarRating.h"
#include "GuessDropdown.h"
#include "../../Core/Types.h"

namespace BlindCard
{

//==============================================================================
/**
 * Suit enumeration for poker cards.
 */
enum class Suit
{
    Spades,   // Position 1, 5 (black)
    Clubs,    // Position 2, 6 (black)
    Diamonds, // Position 3, 7 (red)
    Hearts    // Position 4, 8 (red)
};

//==============================================================================
/**
 * Data structure representing a card's state.
 */
struct CardData
{
    int position = 0;              // 0-7 position index
    std::string trackName;         // Plugin/track name to display
    int rating = 0;                // 0-5 star rating (0 = unrated)
    int guessedTrackIndex = -1;    // -1 if not guessed, otherwise track index
    bool isRevealed = false;       // Whether card front is visible
    bool isSelected = false;       // Whether card has gold selection glow
    bool isPlaying = false;        // Whether audio is currently playing
    bool isCorrectAnswer = false;  // Q&A mode: true when this is the correct answer (bright glow)

    // Random card face value (1-13: A,2,3,4,5,6,7,8,9,10,J,Q,K)
    int cardValue = 1;             // 1=A, 2-10, 11=J, 12=Q, 13=K
    int suitIndex = 0;             // 0=Spades, 1=Clubs, 2=Diamonds, 3=Hearts

    // Get suit from suitIndex
    Suit getSuit() const
    {
        switch (suitIndex % 4)
        {
            case 0: return Suit::Spades;
            case 1: return Suit::Clubs;
            case 2: return Suit::Diamonds;
            case 3: return Suit::Hearts;
            default: return Suit::Spades;
        }
    }

    // Get display string for card value (A, 2-10, J, Q, K)
    juce::String getDisplayNumber() const
    {
        switch (cardValue)
        {
            case 1:  return "A";
            case 11: return "J";
            case 12: return "Q";
            case 13: return "K";
            default: return juce::String(cardValue);
        }
    }
};

//==============================================================================
/**
 * PokerCard displays a poker-style card with flip animation.
 *
 * Visual Design - Card Back (BlindTesting):
 * - Dark placeholder (card back image will be added later)
 * - Gold position label in bottom-right corner (1, 2, 3...)
 * - Stars or dropdown above/below depending on mode
 *
 * Visual Design - Card Front (Revealed):
 * +-----------------+
 * | 4               |  <- Left top: number (24px Bold)
 * | Spade           |  <- Left top: suit symbol
 * |                 |
 * |      Spade      |  <- Center: large suit (48px)
 * |                 |
 * |   Plugin A -    |  <- Center-bottom: Plugin name (12px)
 * |   Compressor    |
 * |                 |
 * |          Heart  |  <- Right bottom: suit (inverted)
 * |             4   |  <- Right bottom: number (inverted)
 * +-----------------+
 * Background: #F5F5F0 (cream)
 * Suits: Spades/Clubs black, Hearts/Diamonds red (#FF3B4E)
 *
 * States:
 * - Back (BlindTesting): shows card back
 * - Front (Setup/Revealed): shows suit, number, plugin name
 * - Selected: gold glow border
 * - Playing: audio indicator with pulse
 *
 * Flip Animation (400ms):
 * - rotateY 0 -> 90 -> 180 degrees
 * - At 50% switch content
 * - Slight translateY (-8px) during flip
 * - Gold glow flash on reveal
 *
 * Usage:
 *   PokerCard card;
 *   card.setCardData(data);
 *   card.setMode(RatingMode::Stars);
 *   card.flip();
 *   card.onClicked = [this]() { handleClick(); };
 */
class PokerCard : public juce::Component,
                  public juce::ChangeListener,
                  public juce::FileDragAndDropTarget
{
public:
    //==========================================================================
    /** Constructor */
    PokerCard();

    /** Destructor */
    ~PokerCard() override;

    //==========================================================================
    // Callbacks
    std::function<void()> onClicked;
    std::function<void(int)> onRatingChanged;      // Stars mode: rating 0-5
    std::function<void(int)> onGuessChanged;       // Guess mode: track index
    std::function<void()> onQASelected;            // Q&A mode: selection

    /** Standalone mode: Called when a file is dropped on the card */
    std::function<void(const juce::File&)> onFileDropped;

    /** Standalone mode: Called when remove button is clicked */
    std::function<void()> onRemoveFileClicked;

    //==========================================================================
    /**
     * Sets the card data to display.
     * @param data The CardData structure with all card state
     */
    void setCardData(const CardData& data);

    /** Returns the current card data */
    const CardData& getCardData() const { return cardData; }

    /**
     * Sets the rating mode (determines which controls to show).
     * @param mode Stars, Guess, or QA
     */
    void setMode(blindcard::RatingMode mode);

    /** Returns the current mode */
    blindcard::RatingMode getMode() const { return currentMode; }

    /**
     * Sets the game phase (affects card appearance).
     * @param phase Setup, BlindTesting, or Revealed
     */
    void setPhase(blindcard::GamePhase phase);

    /** Returns the current phase */
    blindcard::GamePhase getPhase() const { return currentPhase; }

    /**
     * Triggers the flip animation.
     * Call when revealing the card or flipping back.
     */
    void flip();

    /**
     * Sets tracks for the GuessDropdown.
     * @param tracks Vector of track names
     */
    void setTracks(const std::vector<std::string>& tracks);

    /**
     * Sets the selected state (gold glow).
     * @param selected true to show selection glow
     */
    void setSelected(bool selected);

    /** Returns true if card is selected */
    bool isSelected() const { return cardData.isSelected; }

    /**
     * Sets the playing state (audio indicator).
     * @param playing true to show playing indicator
     */
    void setPlaying(bool playing);

    /** Returns true if audio is playing */
    bool isPlaying() const { return cardData.isPlaying; }

    //==========================================================================
    // Standalone mode support

    /**
     * Enable or disable standalone mode (file drag-drop).
     * @param enabled true to enable file dropping
     */
    void setStandaloneMode(bool enabled);

    /** Check if standalone mode is enabled */
    bool isStandaloneMode() const { return standaloneMode; }

    /**
     * Set the loaded audio file info for display.
     * @param file The loaded audio file (empty File if none)
     * @param durationSeconds Duration of the file in seconds
     */
    void setLoadedAudioFile(const juce::File& file, double durationSeconds);

    /** Get the loaded audio file */
    const juce::File& getLoadedAudioFile() const { return loadedAudioFile; }

    /** Check if this card has an audio file loaded */
    bool hasAudioFileLoaded() const { return loadedAudioFile.existsAsFile(); }

    //==========================================================================
    // FileDragAndDropTarget overrides
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;

    // Timer callback for animation updates
    void updateAnimations();

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Standard dimensions */
    static constexpr int kDefaultWidth = 120;
    static constexpr int kDefaultHeight = 170;
    static constexpr int kCornerRadius = 8;

    // Card front colors
    static inline const juce::Colour kCardFrontColor { 0xFFF5F5F0 };  // Cream
    static inline const juce::Colour kRedSuitColor { 0xFFFF3B4E };    // Red for hearts/diamonds
    static inline const juce::Colour kBlackSuitColor { 0xFF1A1A1A };  // Black for spades/clubs

private:
    //==========================================================================
    // State
    CardData cardData;
    blindcard::RatingMode currentMode = blindcard::RatingMode::Stars;
    blindcard::GamePhase currentPhase = blindcard::GamePhase::Setup;
    bool isHovered = false;

    // Standalone mode state
    bool standaloneMode = false;
    bool isDragHovering = false;
    juce::File loadedAudioFile;
    double audioDurationSeconds = 0.0;
    juce::Rectangle<int> removeButtonBounds;

    //==========================================================================
    // Child components
    std::unique_ptr<StarRating> starRating;
    std::unique_ptr<GuessDropdown> guessDropdown;
    std::unique_ptr<juce::TextButton> qaSelectButton;

    //==========================================================================
    // Animation values
    AnimatedValue flipProgress;        // 0.0 = back, 1.0 = front
    AnimatedValue selectionGlow;       // 0.0 = none, 1.0 = full glow
    AnimatedValue playingPulse;        // Pulse animation for playing state
    AnimatedValue hoverGlow;           // Hover highlight
    AnimatedValue correctAnswerGlow;   // Bright green glow for correct answer in Q&A

    // Animation constants
    static constexpr float kFlipDurationMs = 400.0f;
    static constexpr float kGlowDurationMs = 200.0f;
    static constexpr float kPulseDurationMs = 1000.0f;
    static constexpr float kFlipTranslateY = -8.0f;

    // Flash animation for reveal
    AnimatedValue revealFlash;         // Gold flash on reveal
    bool isFlashActive = false;

    //==========================================================================
    // Timer for animations
    class AnimationTimer : public juce::Timer
    {
    public:
        AnimationTimer(PokerCard& owner) : card(owner) {}
        void timerCallback() override { card.updateAnimations(); }
    private:
        PokerCard& card;
    };
    std::unique_ptr<AnimationTimer> animationTimer;
    juce::int64 lastUpdateTime = 0;

    // Pulse timing
    juce::int64 pulseStartTime = 0;

    //==========================================================================
    // Card back image (loaded from binary data)
    static juce::Image& getCardBackImage();

    //==========================================================================
    // Drawing helpers - Card Back
    void drawCardBack(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawPositionLabel(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Drawing helpers - Card Front
    void drawCardFront(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawTopLeftCorner(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawBottomRightCorner(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCenterSuit(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawPluginName(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Drawing helpers - Effects
    void drawSelectionGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float glowAmount);
    void drawPlayingGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float pulseAmount);
    void drawPlayingIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float pulseAmount);
    void drawRevealFlash(juce::Graphics& g, juce::Rectangle<float> bounds, float flashAmount);
    void drawCorrectAnswerGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float glowAmount);

    // Drawing helpers - Standalone mode
    void drawEmptyCardState(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawDragHoverOverlay(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawLoadedAudioInfo(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawRemoveButton(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Standalone mode helpers
    bool isValidAudioFile(const juce::String& path) const;

    // Suit helpers
    juce::String getSuitSymbol(Suit suit) const;
    juce::Colour getSuitColor(Suit suit) const;

    // Layout helpers
    void updateChildComponentVisibility();
    void updateChildComponentBounds();

    // Animation utilities
    void startAnimationTimer();
    void stopAnimationTimerIfIdle();
    void triggerRevealFlash();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PokerCard)
};

} // namespace BlindCard
