/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "BlindCardManager.h"
#include "../PluginProcessor.h"
#include <random>
#include <set>
#include <vector>
#include <algorithm>

namespace blindcard
{

BlindCardManager::BlindCardManager()
{
    DBG ("BlindCardManager created");
}

BlindCardManager::~BlindCardManager()
{
    DBG ("BlindCardManager destroyed");
}

int BlindCardManager::registerInstance (BlindCardProcessor* instance, const juce::String& trackName)
{
    int resultCardId = -1;
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // Don't allow new instances during blind testing
        if (state.phase == GamePhase::BlindTesting)
            return -1;

        // Maximum 8 instances
        if (instances.size() >= GameState::MaxCards)
            return -1;

        instances.add (instance);

        // Create corresponding card
        CardSlot card;
        card.id = static_cast<int> (state.cards.size());
        card.realTrackName = trackName.isEmpty()
            ? "Track " + juce::String (card.id + 1)
            : trackName;
        card.displayPosition = card.id;

        // Assign random poker card value (avoid duplicates with existing cards)
        std::set<std::pair<int, int>> usedCards;
        for (const auto& existingCard : state.cards)
            usedCards.insert ({ existingCard.cardValue, existingCard.suitIndex });

        // Build list of available poker cards
        juce::Array<std::pair<int, int>> availableCards;
        for (int suit = 0; suit < 4; ++suit)
            for (int value = 1; value <= 13; ++value)
                if (usedCards.find ({ value, suit }) == usedCards.end())
                    availableCards.add ({ value, suit });

        // Randomly select one
        if (!availableCards.isEmpty())
        {
            std::random_device rd;
            std::mt19937 gen (rd());
            std::uniform_int_distribution<> dis (0, availableCards.size() - 1);
            int idx = dis (gen);
            card.cardValue = availableCards[idx].first;
            card.suitIndex = availableCards[idx].second;
        }
        else
        {
            // Fallback: if none available (unlikely), use random values
            std::random_device rd;
            std::mt19937 gen (rd());
            card.cardValue = std::uniform_int_distribution<> (1, 13) (gen);
            card.suitIndex = std::uniform_int_distribution<> (0, 3) (gen);
        }

        state.cards.add (card);

        resultCardId = card.id;

        // Auto-select first card (ensure only one track plays)
        if (state.cards.size() == 1)
            state.selectedCardId = 0;

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
    return resultCardId;
}

void BlindCardManager::unregisterInstance (BlindCardProcessor* instance)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        int index = instances.indexOf (instance);
        if (index < 0)
            return;

        if (state.phase == GamePhase::BlindTesting)
        {
            // During blind testing, mark card as removed but keep both arrays in sync
            if (index < state.cards.size())
                state.cards.getReference (index).isRemoved = true;
            // Don't remove from instances to maintain correspondence
            // Just set to nullptr to indicate it's gone
            instances.set (index, nullptr);
        }
        else
        {
            instances.remove (index);
            if (index < state.cards.size())
            {
                state.cards.remove (index);
                // Re-number cards
                for (int i = 0; i < state.cards.size(); ++i)
                {
                    state.cards.getReference (i).id = i;
                    state.cards.getReference (i).displayPosition = i;
                }

                // Update selectedCardId to keep it valid
                if (state.selectedCardId == index)
                {
                    // Selected card was removed, select first card
                    state.selectedCardId = state.cards.isEmpty() ? -1 : 0;
                }
                else if (state.selectedCardId > index)
                {
                    // Selected card was after the removed one, decrement ID
                    state.selectedCardId--;
                }

                // Notify all processors to update their cardId
                for (int i = 0; i < instances.size(); ++i)
                {
                    if (instances[i] != nullptr)
                        instances[i]->updateCardId (i);
                }
            }
        }

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::setTotalRounds (int rounds)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        if (state.phase == GamePhase::Setup)
        {
            state.totalRounds = juce::jlimit (1, 10, rounds);
            shouldNotify = true;
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::shuffle()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (state.phase != GamePhase::Setup)
            return;

        // Check minimum card count
        if (state.cards.size() < GameState::MinCards)
            return;

        // Initialize round data for each card
        for (auto& card : state.cards)
        {
            card.rounds.clear();
            for (int i = 0; i < state.totalRounds; ++i)
                card.rounds.add (RoundData{});
        }

        // Randomly shuffle displayPosition
        std::random_device rd;
        std::mt19937 gen (rd());

        juce::Array<int> positions;
        for (int i = 0; i < state.cards.size(); ++i)
            positions.add (i);

        for (int i = positions.size() - 1; i > 0; --i)
        {
            std::uniform_int_distribution<> dis (0, i);
            int j = dis (gen);
            positions.swap (i, j);
        }

        for (int i = 0; i < state.cards.size(); ++i)
            state.cards.getReference (i).displayPosition = positions[i];

        // Note: Card values (cardValue, suitIndex) were assigned when cards were created,
        // shuffle() only shuffles display positions, not card suits/values

        state.phase = GamePhase::BlindTesting;
        state.currentRound = 0;

        // Select the visually first position (displayPosition=0) card
        // This prevents users from guessing card identity by playback position
        state.selectedCardId = -1;
        for (int i = 0; i < state.cards.size(); ++i)
        {
            if (state.cards[i].displayPosition == 0)
            {
                state.selectedCardId = i;
                break;
            }
        }

        // Q&A mode: Initialize and select first question
        if (ratingMode == RatingMode::QA)
        {
            state.qaState.reset();
            selectNextQAQuestion();
        }

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::nextRound()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // Allow transition from both BlindTesting and Revealed phases
        if (state.phase != GamePhase::BlindTesting && state.phase != GamePhase::Revealed)
            return;

        if (state.currentRound < state.totalRounds - 1)
        {
            state.currentRound++;

            // Re-shuffle each round - shuffle displayPosition
            std::random_device rd;
            std::mt19937 gen (rd());

            juce::Array<int> positions;
            for (int i = 0; i < state.cards.size(); ++i)
                positions.add (i);

            for (int i = positions.size() - 1; i > 0; --i)
            {
                std::uniform_int_distribution<> dis (0, i);
                int j = dis (gen);
                positions.swap (i, j);
            }

            for (int i = 0; i < state.cards.size(); ++i)
                state.cards.getReference (i).displayPosition = positions[i];

            // Select the visually first position (displayPosition=0) card
            // This prevents users from guessing card identity by playback position
            for (int i = 0; i < state.cards.size(); ++i)
            {
                if (state.cards[i].displayPosition == 0)
                {
                    state.selectedCardId = i;
                    break;
                }
            }

            // Set phase back to BlindTesting for the next round
            state.phase = GamePhase::BlindTesting;

            shouldNotify = true;
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::reveal()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (state.phase != GamePhase::BlindTesting)
            return;

        state.phase = GamePhase::Revealed;
        // Auto-select first card (ensure only one track plays)
        state.selectedCardId = state.cards.isEmpty() ? -1 : 0;
        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::reset()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        state.phase = GamePhase::Setup;
        state.currentRound = 0;

        // Clear rating data but keep cards
        for (auto& card : state.cards)
        {
            card.rounds.clear();
            card.displayPosition = card.id;
            card.isRemoved = false;
        }

        // Reassign random poker card values (different for each new game)
        if (!state.cards.isEmpty())
        {
            std::random_device rd;
            std::mt19937 gen (rd());

            // Build a 52-card deck
            struct PokerCard { int value; int suit; };
            juce::Array<PokerCard> deck;
            for (int suit = 0; suit < 4; ++suit)
                for (int value = 1; value <= 13; ++value)
                    deck.add ({ value, suit });

            // Shuffle
            for (int i = deck.size() - 1; i > 0; --i)
            {
                std::uniform_int_distribution<> dis (0, i);
                int j = dis (gen);
                deck.swap (i, j);
            }

            // Assign to cards
            for (int i = 0; i < state.cards.size(); ++i)
            {
                auto& card = state.cards.getReference (i);
                card.cardValue = deck[i].value;
                card.suitIndex = deck[i].suit;
            }
        }

        // Clear Q&A state
        state.qaState.reset();

        // Auto-select first card (ensure only one track plays)
        state.selectedCardId = state.cards.isEmpty() ? -1 : 0;

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::selectCard (int cardId)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // Allow card selection in all phases (including Revealed, so users can re-listen)
        if (cardId >= 0 && cardId < state.cards.size() && !state.cards[cardId].isRemoved)
        {
            state.selectedCardId = cardId;
            shouldNotify = true;
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::deselectCard()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // Prevent deselection during blind testing (avoid multiple tracks playing)
        if (state.phase == GamePhase::BlindTesting)
            return;

        state.selectedCardId = -1;
        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::rateCard (int cardId, int stars)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (state.phase != GamePhase::BlindTesting)
            return;

        if (cardId >= 0 && cardId < state.cards.size() && !state.cards[cardId].isRemoved)
        {
            auto& card = state.cards.getReference (cardId);
            if (state.currentRound < card.rounds.size())
            {
                card.rounds.getReference (state.currentRound).rating = juce::jlimit (0, 5, stars);
                shouldNotify = true;
            }
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::setNote (int cardId, const juce::String& note)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (state.phase != GamePhase::BlindTesting)
            return;

        if (cardId >= 0 && cardId < state.cards.size() && !state.cards[cardId].isRemoved)
        {
            auto& card = state.cards.getReference (cardId);
            if (state.currentRound < card.rounds.size())
            {
                card.rounds.getReference (state.currentRound).note = note;
                shouldNotify = true;
            }
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::setTrackName (int cardId, const juce::String& name)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // Allow track name updates in any phase (UI hides real names during blind testing)
        if (cardId >= 0 && cardId < state.cards.size())
        {
            // Only update if name actually changed
            if (state.cards[cardId].realTrackName != name)
            {
                state.cards.getReference (cardId).realTrackName = name;
                shouldNotify = true;
            }
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::setGuess (int cardId, int guessedTrackId)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (state.phase != GamePhase::BlindTesting)
            return;

        if (cardId >= 0 && cardId < state.cards.size() && !state.cards[cardId].isRemoved)
        {
            auto& card = state.cards.getReference (cardId);
            if (state.currentRound < card.rounds.size())
            {
                card.rounds.getReference (state.currentRound).guessedTrackId = guessedTrackId;
                shouldNotify = true;
            }
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

GamePhase BlindCardManager::getPhase() const
{
    juce::ScopedLock sl (lock);
    return state.phase;
}

int BlindCardManager::getCurrentRound() const
{
    juce::ScopedLock sl (lock);
    return state.currentRound;
}

int BlindCardManager::getTotalRounds() const
{
    juce::ScopedLock sl (lock);
    return state.totalRounds;
}

int BlindCardManager::getSelectedCardId() const
{
    juce::ScopedLock sl (lock);
    return state.selectedCardId;
}

bool BlindCardManager::isCardSelected (int cardId) const
{
    juce::ScopedLock sl (lock);
    return state.selectedCardId == cardId;
}

juce::Array<CardSlot> BlindCardManager::getCards() const
{
    juce::ScopedLock sl (lock);
    return state.cards;
}

int BlindCardManager::getRegisteredCount() const
{
    juce::ScopedLock sl (lock);
    return instances.size();
}

bool BlindCardManager::canJoinGame() const
{
    juce::ScopedLock sl (lock);
    return state.phase == GamePhase::Setup && instances.size() < GameState::MaxCards;
}

float BlindCardManager::getCurrentPlayingRMSdB() const
{
    BlindCardProcessor* processor = nullptr;
    {
        juce::ScopedLock sl (lock);
        int selectedId = state.selectedCardId;
        if (selectedId >= 0 && selectedId < instances.size())
            processor = instances[selectedId];
    }

    // Call processor method outside lock (avoid deadlock)
    if (processor != nullptr)
        return processor->getCurrentRMSdB();

    return -100.0f;
}

juce::String BlindCardManager::getCurrentPlayingTrackName() const
{
    juce::ScopedLock sl (lock);
    int selectedId = state.selectedCardId;

    if (selectedId < 0 || selectedId >= state.cards.size())
        return {};

    const auto& card = state.cards[selectedId];

    // During blind testing, only show card number, not track name
    if (state.phase == GamePhase::BlindTesting)
        return "Card " + juce::String (card.displayPosition + 1);

    // In Setup or Revealed phase, show track name
    return card.realTrackName;
}

void BlindCardManager::setBypassAll (bool bypass)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        if (bypassAll != bypass)
        {
            bypassAll = bypass;
            shouldNotify = true;
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

bool BlindCardManager::isBypassAll() const
{
    juce::ScopedLock sl (lock);
    return bypassAll;
}

void BlindCardManager::setRatingMode (RatingMode mode)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        // Q&A mode requires at least 2 tracks
        if (mode == RatingMode::QA && state.cards.size() < 2)
            return;

        if (ratingMode != mode)
        {
            ratingMode = mode;
            shouldNotify = true;
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

RatingMode BlindCardManager::getRatingMode() const
{
    juce::ScopedLock sl (lock);
    return ratingMode;
}

void BlindCardManager::setMeasuredLUFS (int cardId, float lufs)
{
    bool shouldNotify = false;
    bool shouldRecalculate = false;
    bool shouldAutoLock = false;
    {
        juce::ScopedLock sl (lock);
        if (cardId >= 0 && cardId < state.cards.size())
        {
            state.cards.getReference (cardId).measuredLUFS = lufs;
            shouldNotify = true;

            // During calibration, don't auto-recalculate gains (wait for lockCalibration)
            // But if not calibrating (manual LUFS setting), auto-recalculate
            if (!calibrating_)
                shouldRecalculate = true;

            DBG ("BlindCardManager: Card " << cardId << " LUFS = " << lufs << " dB");

            // During calibration: check if all cards have been measured
            if (calibrating_)
            {
                int measuredCount = 0;
                int totalCount = 0;
                for (const auto& card : state.cards)
                {
                    if (!card.isRemoved)
                    {
                        totalCount++;
                        if (card.hasLUFSMeasurement())
                            measuredCount++;
                    }
                }
                DBG ("BlindCardManager: Calibration progress " << measuredCount << "/" << totalCount);

                // All cards measured, auto-lock calibration
                if (measuredCount >= totalCount && totalCount > 0)
                    shouldAutoLock = true;
            }
        }
    }
    if (shouldRecalculate)
        recalculateAutoGains();
    if (shouldAutoLock)
        lockCalibration();
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::setManualGain (int cardId, float gainDb)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        if (cardId >= 0 && cardId < state.cards.size())
        {
            // Limit manual gain range to -12 to +12 dB
            state.cards.getReference (cardId).manualGainDb = juce::jlimit (-12.0f, 12.0f, gainDb);
            shouldNotify = true;
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::recalculateAutoGains()
{
    juce::ScopedLock sl (lock);

    // Collect all valid LUFS measurements
    std::vector<float> lufsValues;
    for (const auto& card : state.cards)
    {
        if (card.hasLUFSMeasurement() && !card.isRemoved)
            lufsValues.push_back (card.measuredLUFS);
    }

    // If no valid measurements, reset all gains
    if (lufsValues.empty())
    {
        for (auto& card : state.cards)
            card.autoGainDb = 0.0f;
        return;
    }

    // Calculate median as reference
    std::sort (lufsValues.begin(), lufsValues.end());
    float medianLUFS;
    size_t n = lufsValues.size();
    if (n % 2 == 1)
    {
        // Odd count: take middle value
        medianLUFS = lufsValues[n / 2];
    }
    else
    {
        // Even count: take smaller of two middle values (more conservative, avoid excessive volume)
        medianLUFS = lufsValues[n / 2 - 1];
    }

    // Calculate gains using median as reference
    for (auto& card : state.cards)
    {
        if (card.hasLUFSMeasurement() && !card.isRemoved)
        {
            // Calculate gain needed to reach median loudness
            card.autoGainDb = medianLUFS - card.measuredLUFS;
            // Limit auto gain range to -24 to +24 dB
            card.autoGainDb = juce::jlimit (-24.0f, 24.0f, card.autoGainDb);
        }
        else
        {
            card.autoGainDb = 0.0f;
        }
    }
}

float BlindCardManager::getGainForCard (int cardId) const
{
    juce::ScopedLock sl (lock);

    // If Level-Match is not enabled or calibration not complete, don't apply gain
    if (!levelMatchEnabled_ || !calibrated_)
        return 0.0f;

    if (cardId >= 0 && cardId < state.cards.size())
        return state.cards[cardId].getTotalGainDb();
    return 0.0f;
}

void BlindCardManager::resetLevelMatching()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        for (auto& card : state.cards)
        {
            card.measuredLUFS = -100.0f;
            card.autoGainDb = 0.0f;
            card.manualGainDb = 0.0f;
        }
        // Reset calibration state
        calibrating_ = false;
        calibrated_ = false;
        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

// ============================================================================
// Level Matching - Calibration Flow
// ============================================================================

void BlindCardManager::startCalibration()
{
    bool shouldNotify = false;
    juce::Array<BlindCardProcessor*> instancesCopy;
    {
        juce::ScopedLock sl (lock);

        // Allow calibration in Setup and BlindTesting phases
        // Users may want to enable Level-Match after starting blind test
        if (state.phase != GamePhase::Setup && state.phase != GamePhase::BlindTesting)
            return;

        // Reset all measurement data
        for (auto& card : state.cards)
        {
            card.measuredLUFS = -100.0f;
            card.autoGainDb = 0.0f;
        }

        calibrating_ = true;
        calibrated_ = false;
        calibrationStartTime_ = juce::Time::currentTimeMillis();

        // Copy instance list (avoid calling external methods while holding lock)
        instancesCopy = instances;
        shouldNotify = true;

        DBG ("BlindCardManager: Calibration started");
    }

    // Start measurement on all instances outside lock
    for (auto* instance : instancesCopy)
    {
        if (instance != nullptr)
            instance->startMeasurement (kCalibrationDuration);
    }

    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::lockCalibration()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (!calibrating_)
            return;

        // Check if all cards have measurements
        int measuredCount = 0;
        for (const auto& card : state.cards)
        {
            if (card.hasLUFSMeasurement() && !card.isRemoved)
                measuredCount++;
        }

        // Need at least one measurement to lock
        if (measuredCount == 0)
        {
            DBG ("BlindCardManager: Cannot lock calibration - no measurements");
            return;
        }

        // Calculate gains (using median as reference)
        recalculateAutoGains();

        calibrating_ = false;
        calibrated_ = true;
        shouldNotify = true;

        DBG ("BlindCardManager: Calibration locked with " << measuredCount << " measurements");
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::setLevelMatchEnabled (bool enabled)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        if (levelMatchEnabled_ != enabled)
        {
            levelMatchEnabled_ = enabled;
            shouldNotify = true;

            DBG ("BlindCardManager: Level-Match " << (enabled ? "enabled" : "disabled"));
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

bool BlindCardManager::isLevelMatchEnabled() const
{
    juce::ScopedLock sl (lock);
    return levelMatchEnabled_;
}

bool BlindCardManager::isCalibrating() const
{
    juce::ScopedLock sl (lock);
    return calibrating_;
}

bool BlindCardManager::isCalibrated() const
{
    juce::ScopedLock sl (lock);
    return calibrated_;
}

std::pair<int, int> BlindCardManager::getCalibrationProgress() const
{
    juce::ScopedLock sl (lock);
    int measuredCount = 0;
    int totalCount = 0;
    for (const auto& card : state.cards)
    {
        if (!card.isRemoved)
        {
            totalCount++;
            if (card.hasLUFSMeasurement())
                measuredCount++;
        }
    }
    return { measuredCount, totalCount };
}

float BlindCardManager::getCalibrationTimeRemaining() const
{
    juce::ScopedLock sl (lock);
    if (!calibrating_)
        return 0.0f;

    auto elapsed = (juce::Time::currentTimeMillis() - calibrationStartTime_) / 1000.0f;
    auto remaining = kCalibrationDuration - elapsed;
    return juce::jmax (0.0f, remaining);
}

void BlindCardManager::notifyListeners()
{
    sendChangeMessage();
}

// Q&A Mode Implementation

void BlindCardManager::selectNextQAQuestion()
{
    // Randomly select from unasked cards (already holding lock when called)
    juce::Array<int> availableIds;
    for (const auto& card : state.cards)
    {
        if (!card.isRemoved && !state.qaState.askedCardIds.contains (card.id))
            availableIds.add (card.id);
    }

    // All cards asked once — reset cycle so cards can be asked again
    if (availableIds.isEmpty())
    {
        state.qaState.askedCardIds.clear();
        for (const auto& card : state.cards)
        {
            if (!card.isRemoved)
                availableIds.add (card.id);
        }
    }

    if (availableIds.isEmpty())
        return;

    std::random_device rd;
    std::mt19937 gen (rd());
    std::uniform_int_distribution<> dis (0, availableIds.size() - 1);

    state.qaState.targetCardId = availableIds[dis (gen)];
    state.qaState.askedCardIds.add (state.qaState.targetCardId);
    state.qaState.lastFeedback = QAState::FeedbackState::None;
    state.qaState.lastAnsweredCardId = -1;
}

void BlindCardManager::submitQAAnswer (int selectedCardId)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (state.phase != GamePhase::BlindTesting)
            return;
        if (ratingMode != RatingMode::QA)
            return;

        int maxQ = getQAMaxQuestions();
        if (state.qaState.isComplete (maxQ))
            return;

        // If countdown is active, ignore clicks (let user see the answer)
        if (state.qaState.isShowingAnswer)
            return;

        bool correct = (selectedCardId == state.qaState.targetCardId);
        state.qaState.answers.add (correct);
        state.qaState.lastFeedback = correct ? QAState::FeedbackState::Correct
                                             : QAState::FeedbackState::Wrong;
        state.qaState.lastAnsweredCardId = selectedCardId;

        // Start answer reveal countdown (3 seconds)
        state.qaState.isShowingAnswer = true;
        state.qaState.countdownValue = 3;
        state.qaState.revealedTargetCardId = state.qaState.targetCardId;

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::nextQAQuestion()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (state.phase != GamePhase::BlindTesting)
            return;
        if (ratingMode != RatingMode::QA)
            return;

        int maxQ = getQAMaxQuestions();
        if (state.qaState.isComplete (maxQ))
            return;

        selectNextQAQuestion();
        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

const QAState& BlindCardManager::getQAState() const
{
    juce::ScopedLock sl (lock);
    return state.qaState;
}

juce::String BlindCardManager::getCurrentQuestionTrackName() const
{
    juce::ScopedLock sl (lock);
    for (const auto& card : state.cards)
    {
        if (card.id == state.qaState.targetCardId)
            return card.realTrackName;
    }
    return {};
}

bool BlindCardManager::canStartQAMode() const
{
    juce::ScopedLock sl (lock);
    int activeCount = 0;
    for (const auto& card : state.cards)
    {
        if (!card.isRemoved)
            activeCount++;
    }
    return activeCount >= 2;
}

int BlindCardManager::getQAMaxQuestions() const
{
    juce::ScopedLock sl (lock);
    return state.qaQuestionCount;
}

void BlindCardManager::setQAQuestionCount (int count)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        int clampedCount = juce::jlimit (GameState::MinQAQuestions, GameState::MaxQAQuestions, count);
        if (state.qaQuestionCount != clampedCount)
        {
            state.qaQuestionCount = clampedCount;
            shouldNotify = true;
        }
    }
    if (shouldNotify)
        sendChangeMessage();
}

int BlindCardManager::getQAQuestionCount() const
{
    juce::ScopedLock sl (lock);
    return state.qaQuestionCount;
}

void BlindCardManager::tickQACountdown()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (!state.qaState.isShowingAnswer)
            return;

        state.qaState.countdownValue--;

        if (state.qaState.countdownValue <= 0)
        {
            // Countdown finished, advance to next question
            state.qaState.isShowingAnswer = false;
            state.qaState.countdownValue = 0;
            state.qaState.revealedTargetCardId = -1;
            state.qaState.currentQuestion++;

            int maxQ = getQAMaxQuestions();
            if (state.qaState.isComplete (maxQ))
            {
                // All questions complete, enter Revealed phase
                state.phase = GamePhase::Revealed;
            }
            else
            {
                // More questions remaining, select next question
                selectNextQAQuestion();
            }
        }

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::skipQACountdown()
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        if (!state.qaState.isShowingAnswer)
            return;

        // End countdown immediately
        state.qaState.isShowingAnswer = false;
        state.qaState.countdownValue = 0;
        state.qaState.revealedTargetCardId = -1;
        state.qaState.currentQuestion++;

        int maxQ = getQAMaxQuestions();
        if (state.qaState.isComplete (maxQ))
        {
            state.phase = GamePhase::Revealed;
        }
        else
        {
            selectNextQAQuestion();
        }

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

int BlindCardManager::getQACorrectAnswerCardId() const
{
    juce::ScopedLock sl (lock);
    // If showing answer, return the revealed correct card ID
    if (state.qaState.isShowingAnswer)
        return state.qaState.revealedTargetCardId;
    return -1;
}

void BlindCardManager::addTestCards (int count)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // Only allow in Setup phase
        if (state.phase != GamePhase::Setup)
            return;

        // If already have enough cards, no need to add
        if (state.cards.size() >= count)
            return;

        // Limit count
        count = juce::jlimit (1, static_cast<int> (GameState::MaxCards), count);

        // Test card names
        juce::StringArray testNames = { "Kick Drum", "Snare", "Hi-Hat", "Bass",
                                        "Piano", "Guitar", "Synth Lead", "Vocal" };

        // Build a 52-card deck and shuffle (ensure no duplicates)
        std::random_device rd;
        std::mt19937 gen (rd());

        struct PokerCard { int value; int suit; };
        juce::Array<PokerCard> deck;
        for (int suit = 0; suit < 4; ++suit)
            for (int value = 1; value <= 13; ++value)
                deck.add ({ value, suit });

        for (int i = deck.size() - 1; i > 0; --i)
        {
            std::uniform_int_distribution<> dis (0, i);
            int j = dis (gen);
            deck.swap (i, j);
        }

        // Add test cards starting from existing count
        int startId = state.cards.size();
        for (int i = startId; i < count; ++i)
        {
            CardSlot card;
            card.id = i;
            card.realTrackName = testNames[i % testNames.size()];
            card.displayPosition = i;

            // Assign from shuffled deck (guaranteed no duplicates)
            card.cardValue = deck[i].value;
            card.suitIndex = deck[i].suit;

            state.cards.add (card);
        }

        // Auto-select first card (if not already selected)
        if (state.selectedCardId < 0)
            state.selectedCardId = 0;

        shouldNotify = true;

        DBG ("BlindCardManager: Added test cards, total now: " << state.cards.size());
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::setTestCardCount (int count)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // Only allow changes in Setup phase
        if (state.phase != GamePhase::Setup)
            return;

        // Limit count
        count = juce::jlimit (static_cast<int> (GameState::MinCards),
                              static_cast<int> (GameState::MaxCards), count);

        int currentCount = state.cards.size();

        // If count is same, no need to change
        if (count == currentCount)
            return;

        // Test card names
        juce::StringArray testNames = { "Kick Drum", "Snare", "Hi-Hat", "Bass",
                                        "Piano", "Guitar", "Synth Lead", "Vocal" };

        if (count > currentCount)
        {
            // Adding cards: collect existing poker card values to avoid duplicates
            std::set<std::pair<int, int>> usedCards;
            for (const auto& card : state.cards)
                usedCards.insert ({ card.cardValue, card.suitIndex });

            // Build remaining available poker cards
            std::random_device rd;
            std::mt19937 gen (rd());

            juce::Array<std::pair<int, int>> availableCards;
            for (int suit = 0; suit < 4; ++suit)
                for (int value = 1; value <= 13; ++value)
                    if (usedCards.find ({ value, suit }) == usedCards.end())
                        availableCards.add ({ value, suit });

            // Shuffle
            for (int i = availableCards.size() - 1; i > 0; --i)
            {
                std::uniform_int_distribution<> dis (0, i);
                int j = dis (gen);
                availableCards.swap (i, j);
            }

            // Add new cards
            int cardIdx = 0;
            for (int i = currentCount; i < count; ++i)
            {
                CardSlot card;
                card.id = i;
                // In standalone mode, don't auto-assign track names, wait for user to drop audio files
                card.realTrackName = standaloneMode ? "" : testNames[i % testNames.size()];
                card.displayPosition = i;

                // Assign from available deck
                if (cardIdx < availableCards.size())
                {
                    card.cardValue = availableCards[cardIdx].first;
                    card.suitIndex = availableCards[cardIdx].second;
                    cardIdx++;
                }

                state.cards.add (card);
            }

            DBG ("BlindCardManager: Added cards, total now: " << state.cards.size());
        }
        else
        {
            // Reducing cards: remove extra cards (from the end)
            while (state.cards.size() > count)
                state.cards.removeLast();

            // Ensure selected card is still valid
            if (state.selectedCardId >= count)
                state.selectedCardId = count - 1;

            DBG ("BlindCardManager: Removed cards, total now: " << state.cards.size());
        }

        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

//==============================================================================
void BlindCardManager::setStandaloneMode (bool enabled)
{
    juce::ScopedLock sl (lock);
    standaloneMode = enabled;

    // Clear all existing card track names (when switching to Standalone mode)
    if (enabled)
    {
        for (auto& card : state.cards)
        {
            card.realTrackName = "";
        }
    }
}

} // namespace blindcard
