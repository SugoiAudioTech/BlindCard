#pragma once

#include "Types.h"
#include <juce_events/juce_events.h>
#include <juce_audio_processors/juce_audio_processors.h>

class BlindCardProcessor; // forward declaration (global namespace)

namespace blindcard
{

class BlindCardManager final : public juce::ChangeBroadcaster
{
public:
    BlindCardManager();
    ~BlindCardManager() override;

    // Instance registration
    int registerInstance (BlindCardProcessor* instance, const juce::String& trackName);
    void unregisterInstance (BlindCardProcessor* instance);

    // Game control
    void setTotalRounds (int rounds);
    void shuffle();
    void nextRound();
    void reveal();
    void reset();

    // Card operations
    void selectCard (int cardId);
    void deselectCard();
    void rateCard (int cardId, int stars);
    void setNote (int cardId, const juce::String& note);
    void setTrackName (int cardId, const juce::String& name);
    void setGuess (int cardId, int guessedTrackId);

    // State queries
    GamePhase getPhase() const;
    int getCurrentRound() const;
    int getTotalRounds() const;
    int getSelectedCardId() const;
    bool isCardSelected (int cardId) const;
    juce::Array<CardSlot> getCards() const;
    int getRegisteredCount() const;
    bool canJoinGame() const;

    // Current playing track info (for UI display)
    float getCurrentPlayingRMSdB() const;         // Real-time RMS of currently selected card
    juce::String getCurrentPlayingTrackName() const;  // Track name of currently selected card

    // Bypass control
    void setBypassAll (bool bypass);
    bool isBypassAll() const;

    // Rating mode
    void setRatingMode (RatingMode mode);
    RatingMode getRatingMode() const;

    // Q&A mode
    void submitQAAnswer (int selectedCardId);
    void nextQAQuestion();
    void tickQACountdown();           // Countdown tick (called by UI every second)
    void skipQACountdown();           // Skip countdown and go to next question
    const QAState& getQAState() const;
    juce::String getCurrentQuestionTrackName() const;
    bool canStartQAMode() const;
    int getQAMaxQuestions() const;
    int getQACorrectAnswerCardId() const;  // Get current correct answer card ID
    void setQAQuestionCount (int count);   // Set Q&A question count (user selection)
    int getQAQuestionCount() const;        // Get Q&A question count setting

    // Level Matching - Calibration flow
    void startCalibration();              // Start calibration (reset measurement data)
    void lockCalibration();               // Lock calibration results
    void setLevelMatchEnabled (bool enabled);
    bool isLevelMatchEnabled() const;
    bool isCalibrating() const;           // Whether calibration is in progress
    bool isCalibrated() const;            // Whether calibration is complete
    std::pair<int, int> getCalibrationProgress() const;  // Returns (measured count, total count)
    float getCalibrationTimeRemaining() const;           // Returns remaining seconds

    // Level Matching - Internal use
    void setMeasuredLUFS (int cardId, float lufs);
    void setManualGain (int cardId, float gainDb);
    void recalculateAutoGains();          // Calculate using median as reference
    float getGainForCard (int cardId) const;
    void resetLevelMatching();

    // Testing: Add test cards (for Standalone animation testing)
    void addTestCards (int count = 4);

    // Testing: Set card count (will clear and rebuild)
    void setTestCardCount (int count);

    // Standalone mode: Don't auto-assign track names
    void setStandaloneMode (bool enabled);
    bool isStandaloneMode() const { return standaloneMode; }

private:
    GameState state;
    juce::Array<BlindCardProcessor*> instances;
    mutable juce::CriticalSection lock;
    bool bypassAll = false;
    bool standaloneMode = false;  // Standalone mode doesn't auto-assign track names
    RatingMode ratingMode = RatingMode::Stars;  // Default to star rating mode

    // Level Matching state
    bool levelMatchEnabled_ = false;      // Whether Level-Match is enabled (default off)
    bool calibrating_ = false;            // Whether calibration is in progress
    bool calibrated_ = false;             // Whether calibration is complete and locked
    juce::int64 calibrationStartTime_ = 0; // Calibration start time (ms)
    static constexpr float kCalibrationDuration = 10.0f; // Calibration duration (seconds)

    void notifyListeners();
    void selectNextQAQuestion();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardManager)
};

// SharedResourcePointer automatically manages lifecycle
using SharedBlindCardManager = juce::SharedResourcePointer<BlindCardManager>;

} // namespace blindcard
