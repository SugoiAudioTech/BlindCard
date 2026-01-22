#pragma once

#include <juce_core/juce_core.h>

namespace blindcard
{

// Single round rating data
struct RoundData
{
    int rating = 0;          // 1-5 stars, 0 = not rated
    juce::String note;       // Text note
    int guessedTrackId = -1; // Guessed original track ID (-1 = not guessed)
};

// Single card slot
struct CardSlot
{
    int id = -1;                              // 0-7 card ID
    juce::String realTrackName;               // Real track name
    int displayPosition = -1;                 // Display position after shuffle
    bool isRemoved = false;                   // Whether removed
    juce::Array<RoundData> rounds;            // Records for each round

    // Poker card display value (randomly assigned during shuffle)
    int cardValue = 1;                        // 1=A, 2-10, 11=J, 12=Q, 13=K
    int suitIndex = 0;                        // 0=Spades, 1=Clubs, 2=Diamonds, 3=Hearts

    // Level Matching
    float measuredLUFS = -100.0f;             // Measured loudness (LUFS), -100 = not measured
    float autoGainDb = 0.0f;                  // Auto gain compensation (dB)
    float manualGainDb = 0.0f;                // Manual gain adjustment (dB)

    float getTotalGainDb() const { return autoGainDb + manualGainDb; }
    bool hasLUFSMeasurement() const { return measuredLUFS > -100.0f; }

    // Calculate average rating (ignoring unrated rounds)
    float getAverageRating() const
    {
        int count = 0;
        int total = 0;
        for (const auto& r : rounds)
        {
            if (r.rating > 0)
            {
                total += r.rating;
                count++;
            }
        }
        return count > 0 ? static_cast<float>(total) / count : 0.0f;
    }

    // Calculate guess accuracy (returns correctCount and totalGuesses)
    std::pair<int, int> getGuessAccuracy() const
    {
        int correct = 0;
        int total = 0;
        for (const auto& r : rounds)
        {
            if (r.guessedTrackId >= 0)  // Made a guess
            {
                total++;
                if (r.guessedTrackId == id)  // Guessed correctly
                    correct++;
            }
        }
        return { correct, total };
    }
};

// Game phase
enum class GamePhase
{
    Setup,        // Initial state, track names editable
    BlindTesting, // Blind testing in progress
    Revealed      // Results revealed
};

// Rating mode
enum class RatingMode
{
    Stars,   // Star rating
    Guess,   // Guess track
    QA       // Q&A quiz mode
};

// Q&A mode state
struct QAState
{
    int currentQuestion = 0;              // Current question index (0-based)
    int targetCardId = -1;                // Target card ID for current question
    juce::Array<int> askedCardIds;        // Card IDs that have been asked
    juce::Array<bool> answers;            // Answer results for each question (true = correct)

    enum class FeedbackState { None, Correct, Wrong };
    FeedbackState lastFeedback = FeedbackState::None;
    int lastAnsweredCardId = -1;          // Card selected in last answer

    // Answer reveal countdown state
    bool isShowingAnswer = false;         // Whether showing answer (counting down)
    int countdownValue = 0;               // Countdown value (3, 2, 1, 0)
    int revealedTargetCardId = -1;        // Correct answer card ID during reveal

    bool isComplete(int maxQuestions) const { return currentQuestion >= maxQuestions; }

    int getCorrectCount() const
    {
        int count = 0;
        for (auto a : answers)
            if (a) count++;
        return count;
    }

    void reset()
    {
        currentQuestion = 0;
        targetCardId = -1;
        askedCardIds.clear();
        answers.clear();
        lastFeedback = FeedbackState::None;
        lastAnsweredCardId = -1;
        isShowingAnswer = false;
        countdownValue = 0;
        revealedTargetCardId = -1;
    }
};

// Game state
struct GameState
{
    GamePhase phase = GamePhase::Setup;
    int totalRounds = 1;                      // Total rounds
    int currentRound = 0;                     // Current round (0-indexed)
    int selectedCardId = -1;                  // Currently soloed card (-1 = none)
    juce::Array<CardSlot> cards;              // Up to 8 cards
    QAState qaState;                          // Q&A mode state
    int qaQuestionCount = 5;                  // Q&A mode question count (user configurable, 1-8)

    static constexpr int MinCards = 2;        // Minimum card count (blind test needs comparison)
    static constexpr int MaxCards = 8;        // Maximum card count (dual row 4+4 layout limit)
    static constexpr int MinQAQuestions = 1;  // Q&A minimum questions
    static constexpr int MaxQAQuestions = 8;  // Q&A maximum questions
};

} // namespace blindcard
