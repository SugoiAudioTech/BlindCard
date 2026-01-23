/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    ResultsPanel.h
    Created: 2026-01-19
    Author:  BlindCard

    Results panel component displaying mode-specific results and statistics.
    Adapts its content based on the current rating mode.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <string>
#include <functional>
#include "../Theme/ThemeManager.h"
#include "../../Core/Types.h"

namespace BlindCard
{

//==============================================================================
/**
 * Result entry for Stars mode.
 */
struct StarsResult
{
    std::string trackName;
    int currentRating = 0;       // Rating for current round (1-5)
    float averageRating = 0.0f;  // Average rating across all rounds (shown on Reveal)
    int cardPosition = 0;
};

/**
 * Result entry for Guess mode.
 */
struct GuessResult
{
    int cardPosition = 0;
    std::string actualTrack;
    std::string guessedTrack;
    bool isCorrect = false;
};

/**
 * Result entry for Q&A mode.
 */
struct QAResult
{
    std::string pluginName;
    int cardPosition = 0;
    bool wasCorrect = false;
};

//==============================================================================
/**
 * ResultsPanel displays mode-specific results.
 *
 * Stars Mode Layout:
 * +---------------------------+
 * | RESULTS (sorted by stars) |
 * | ★★★★★ Plugin A (Pos 3)   |
 * | ★★★★☆ Plugin C (Pos 1)   |
 * | ★★★☆☆ Plugin B (Pos 2)   |
 * | ★★☆☆☆ Plugin D (Pos 4)   |
 * +---------------------------+
 *
 * Guess Mode Layout:
 * +---------------------------+
 * | RESULTS                   |
 * | Card 1: ✓ Correct         |
 * | Card 2: ✗ Was: Plugin B   |
 * | Card 3: ✓ Correct         |
 * | Card 4: ✗ Was: Plugin A   |
 * | Score: 2/4 (50%)          |
 * |       [SUBMIT GUESSES]    |
 * +---------------------------+
 *
 * Q&A Mode Layout:
 * +---------------------------+
 * | RESULTS                   |
 * | ✓ Plugin A: Card 3        |
 * | ✗ Plugin B: Guessed 2     |
 * | ✓ Plugin C: Card 1        |
 * | ✓ Plugin D: Card 4        |
 * | Score: 3/4 (75%)          |
 * +---------------------------+
 *
 * Usage:
 *   ResultsPanel panel;
 *   panel.setMode(RatingMode::Stars);
 *   panel.setStarsResults(results);
 *   panel.onSubmitGuesses = [this]() { submitGuesses(); };
 */
class ResultsPanel : public juce::Component,
                     public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    ResultsPanel();

    /** Destructor */
    ~ResultsPanel() override;

    //==========================================================================
    // Callbacks
    std::function<void()> onSubmitGuesses;  // Guess mode submit button

    //==========================================================================
    /**
     * Sets the rating mode (determines display format).
     * @param mode Stars, Guess, or QA
     */
    void setMode(blindcard::RatingMode mode);

    /** Returns the current mode */
    blindcard::RatingMode getMode() const { return currentMode; }

    /**
     * Sets the game phase (hides track names during BlindTesting).
     * @param phase Setup, BlindTesting, or Revealed
     */
    void setPhase(blindcard::GamePhase phase);

    /** Returns the current phase */
    blindcard::GamePhase getPhase() const { return currentPhase; }

    //==========================================================================
    // Stars mode

    /**
     * Sets the Stars mode results.
     * Results will be sorted by rating (highest first).
     * @param results Vector of StarsResult entries
     */
    void setStarsResults(const std::vector<StarsResult>& results);

    //==========================================================================
    // Guess mode

    /**
     * Sets the Guess mode results.
     * @param results Vector of GuessResult entries
     */
    void setGuessResults(const std::vector<GuessResult>& results);

    /**
     * Sets whether the submit button is enabled.
     * @param enabled true to enable submit button
     */
    void setSubmitEnabled(bool enabled);

    /**
     * Sets the Guess mode to show results (after submission).
     * @param showResults true to show results, false to show pending guesses
     */
    void setGuessResultsVisible(bool showResults);

    //==========================================================================
    // Q&A mode

    /**
     * Sets the Q&A mode results.
     * @param results Vector of QAResult entries
     */
    void setQAResults(const std::vector<QAResult>& results);

    //==========================================================================
    // Common

    /**
     * Clears all results.
     */
    void clearResults();

    /**
     * Sets whether to show "Final Results" instead of "Current Round".
     * @param showFinal true to show "Final Results" header
     */
    void setShowFinalResults(bool showFinal);

    /**
     * Sets the current round number and total rounds (for Guess mode tracking).
     * @param current Current round number (1-based)
     * @param total Total number of rounds
     */
    void setRoundInfo(int current, int total);

    /**
     * Accumulates the current round's guess results into cumulative totals.
     * Call this after revealing results each round.
     */
    void accumulateGuessResults();

    /**
     * Resets cumulative tracking (call when starting new game).
     */
    void resetCumulativeTracking();

    /**
     * Sets the countdown value to display (0 = hide countdown).
     * @param seconds Countdown seconds remaining (3, 2, 1, or 0 to hide)
     */
    void setCountdown(int seconds);

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Dimensions */
    static constexpr int kPreferredWidth = 280;
    static constexpr int kMinHeight = 200;

private:
    //==========================================================================
    // State
    blindcard::RatingMode currentMode = blindcard::RatingMode::Stars;
    blindcard::GamePhase currentPhase = blindcard::GamePhase::Setup;

    std::vector<StarsResult> starsResults;
    std::vector<GuessResult> guessResults;
    std::vector<QAResult> qaResults;

    bool guessResultsVisible = false;
    bool submitEnabled = true;
    bool showFinalResults = false;

    // Cumulative tracking for Guess mode
    int totalCorrectGuesses = 0;
    int totalGuessAttempts = 0;
    int currentRoundNumber = 0;
    int totalRounds = 1;

    // Countdown display
    int countdownSeconds = 0;  // 0 = hidden, 1-3 = show countdown

    //==========================================================================
    // Child components
    std::unique_ptr<juce::TextButton> submitButton;

    //==========================================================================
    // Drawing helpers - Stars mode
    void drawStarsResults(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawStarsRow(juce::Graphics& g, juce::Rectangle<float> bounds,
                      const StarsResult& result, bool isWinner);

    // Drawing helpers - Guess mode
    void drawGuessResults(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGuessPending(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGuessRow(juce::Graphics& g, juce::Rectangle<float> bounds,
                      const GuessResult& result);
    void drawGuessScore(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Drawing helpers - Q&A mode
    void drawQAResults(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawQARow(juce::Graphics& g, juce::Rectangle<float> bounds,
                   const QAResult& result);
    void drawQAScore(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Common helpers
    void drawSectionHeader(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawEmptyState(juce::Graphics& g, juce::Rectangle<float> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResultsPanel)
};

} // namespace BlindCard
