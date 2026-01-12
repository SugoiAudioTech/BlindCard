#pragma once

#include "Types.h"
#include <juce_events/juce_events.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace blindcard
{

class BlindCardProcessor; // forward declaration

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

    // 狀態查詢
    GamePhase getPhase() const;
    int getCurrentRound() const;
    int getTotalRounds() const;
    int getSelectedCardId() const;
    bool isCardSelected (int cardId) const;
    const juce::Array<CardSlot>& getCards() const;
    int getRegisteredCount() const;
    bool canJoinGame() const;

private:
    GameState state;
    juce::Array<BlindCardProcessor*> instances;
    mutable juce::CriticalSection lock;

    void notifyListeners();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardManager)
};

// SharedResourcePointer 會自動管理生命週期
using SharedBlindCardManager = juce::SharedResourcePointer<BlindCardManager>;

} // namespace blindcard
