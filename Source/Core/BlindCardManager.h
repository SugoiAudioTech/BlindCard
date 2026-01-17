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

    // 實例註冊
    int registerInstance (BlindCardProcessor* instance, const juce::String& trackName);
    void unregisterInstance (BlindCardProcessor* instance);

    // 遊戲控制
    void setTotalRounds (int rounds);
    void shuffle();
    void nextRound();
    void reveal();
    void reset();

    // 卡牌操作
    void selectCard (int cardId);
    void deselectCard();
    void rateCard (int cardId, int stars);
    void setNote (int cardId, const juce::String& note);
    void setTrackName (int cardId, const juce::String& name);
    void setGuess (int cardId, int guessedTrackId);

    // 狀態查詢
    GamePhase getPhase() const;
    int getCurrentRound() const;
    int getTotalRounds() const;
    int getSelectedCardId() const;
    bool isCardSelected (int cardId) const;
    juce::Array<CardSlot> getCards() const;
    int getRegisteredCount() const;
    bool canJoinGame() const;

    // Bypass 控制
    void setBypassAll (bool bypass);
    bool isBypassAll() const;

    // 評分模式
    void setRatingMode (RatingMode mode);
    RatingMode getRatingMode() const;

    // Q&A 模式
    void submitQAAnswer (int selectedCardId);
    void nextQAQuestion();
    const QAState& getQAState() const;
    juce::String getCurrentQuestionTrackName() const;
    bool canStartQAMode() const;
    int getQAMaxQuestions() const;

    // Level Matching
    void setMeasuredLUFS (int cardId, float lufs);
    void setManualGain (int cardId, float gainDb);
    void recalculateAutoGains();
    float getGainForCard (int cardId) const;
    void setTargetLUFS (float lufs);
    float getTargetLUFS() const;
    void resetLevelMatching();

    // 測試用：加入假卡牌（Standalone 測試動畫用）
    void addTestCards (int count = 4);

private:
    GameState state;
    juce::Array<BlindCardProcessor*> instances;
    mutable juce::CriticalSection lock;
    bool bypassAll = false;
    float targetLUFS = -14.0f;  // 預設目標響度 (streaming 標準)
    RatingMode ratingMode = RatingMode::Stars;  // 預設為星等模式

    void notifyListeners();
    void selectNextQAQuestion();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardManager)
};

// SharedResourcePointer 會自動管理生命週期
using SharedBlindCardManager = juce::SharedResourcePointer<BlindCardManager>;

} // namespace blindcard
