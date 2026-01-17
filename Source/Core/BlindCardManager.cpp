#include "BlindCardManager.h"
#include "../PluginProcessor.h"
#include <random>

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

        // 盲測中不允許新實例加入
        if (state.phase == GamePhase::BlindTesting)
            return -1;

        // 最多 8 個實例
        if (instances.size() >= GameState::MaxCards)
            return -1;

        instances.add (instance);

        // 建立對應的卡牌
        CardSlot card;
        card.id = static_cast<int> (state.cards.size());
        card.realTrackName = trackName.isEmpty()
            ? "Track " + juce::String (card.id + 1)
            : trackName;
        card.displayPosition = card.id;
        state.cards.add (card);

        resultCardId = card.id;

        // 自動選擇第一張卡牌（確保只有一軌播放）
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
                // 重新編號
                for (int i = 0; i < state.cards.size(); ++i)
                {
                    state.cards.getReference (i).id = i;
                    state.cards.getReference (i).displayPosition = i;
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

        // 檢查卡牌數量下限
        if (state.cards.size() < GameState::MinCards)
            return;

        // 初始化每張卡牌的輪次資料
        for (auto& card : state.cards)
        {
            card.rounds.clear();
            for (int i = 0; i < state.totalRounds; ++i)
                card.rounds.add (RoundData{});
        }

        // 隨機打亂 displayPosition
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

        state.phase = GamePhase::BlindTesting;
        state.currentRound = 0;
        // 自動選擇第一張卡（確保只有一軌播放）
        state.selectedCardId = state.cards.isEmpty() ? -1 : 0;

        // Q&A 模式：初始化並選擇第一個問題
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

        if (state.phase != GamePhase::BlindTesting)
            return;

        if (state.currentRound < state.totalRounds - 1)
        {
            state.currentRound++;
            // 保持當前選擇，不重置（避免全部軌道同時播放）
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
        // 自動選擇第一張卡牌（確保只有一軌播放）
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

        // 清除評分資料但保留卡牌
        for (auto& card : state.cards)
        {
            card.rounds.clear();
            card.displayPosition = card.id;
            card.isRemoved = false;
        }

        // 清除 Q&A 狀態
        state.qaState.reset();

        // 自動選擇第一張卡牌（確保只有一軌播放）
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

        // 允許在 Setup 和 BlindTesting 階段選擇卡牌
        if (state.phase == GamePhase::Revealed)
            return;

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

        // 盲測階段禁止取消選擇（避免多軌同時播放）
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

        if (state.phase != GamePhase::Setup)
            return;

        if (cardId >= 0 && cardId < state.cards.size())
        {
            state.cards.getReference (cardId).realTrackName = name;
            shouldNotify = true;
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
        // Q&A 模式至少需要 2 軌
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
    {
        juce::ScopedLock sl (lock);
        if (cardId >= 0 && cardId < state.cards.size())
        {
            state.cards.getReference (cardId).measuredLUFS = lufs;
            shouldNotify = true;
        }
    }
    if (shouldNotify)
    {
        recalculateAutoGains();
        sendChangeMessage();
    }
}

void BlindCardManager::setManualGain (int cardId, float gainDb)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        if (cardId >= 0 && cardId < state.cards.size())
        {
            // 限制手動增益範圍 -12 到 +12 dB
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

    for (auto& card : state.cards)
    {
        if (card.hasLUFSMeasurement() && !card.isRemoved)
        {
            // 計算需要的增益來達到目標響度
            card.autoGainDb = targetLUFS - card.measuredLUFS;
            // 限制自動增益範圍 -24 到 +24 dB
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
    if (cardId >= 0 && cardId < state.cards.size())
        return state.cards[cardId].getTotalGainDb();
    return 0.0f;
}

void BlindCardManager::setTargetLUFS (float lufs)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);
        // 限制目標響度範圍
        float newTarget = juce::jlimit (-30.0f, 0.0f, lufs);
        if (targetLUFS != newTarget)
        {
            targetLUFS = newTarget;
            shouldNotify = true;
        }
    }
    if (shouldNotify)
    {
        recalculateAutoGains();
        sendChangeMessage();
    }
}

float BlindCardManager::getTargetLUFS() const
{
    juce::ScopedLock sl (lock);
    return targetLUFS;
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
        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

void BlindCardManager::notifyListeners()
{
    sendChangeMessage();
}

// Q&A 模式實作

void BlindCardManager::selectNextQAQuestion()
{
    // 從未問過的卡牌中隨機選擇一個（呼叫時已在 lock 中）
    juce::Array<int> availableIds;
    for (const auto& card : state.cards)
    {
        if (!card.isRemoved && !state.qaState.askedCardIds.contains (card.id))
            availableIds.add (card.id);
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

        // 檢查是否正在等待 feedback 確認
        if (state.qaState.lastFeedback != QAState::FeedbackState::None)
            return;

        bool correct = (selectedCardId == state.qaState.targetCardId);
        state.qaState.answers.add (correct);
        state.qaState.lastFeedback = correct ? QAState::FeedbackState::Correct
                                             : QAState::FeedbackState::Wrong;
        state.qaState.lastAnsweredCardId = selectedCardId;
        state.qaState.currentQuestion++;

        // 檢查是否完成所有問題
        if (state.qaState.isComplete (maxQ))
        {
            state.phase = GamePhase::Revealed;
        }

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
    int activeCount = 0;
    for (const auto& card : state.cards)
    {
        if (!card.isRemoved)
            activeCount++;
    }
    return juce::jmin (5, activeCount);
}

void BlindCardManager::addTestCards (int count)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // 只在 Setup 階段允許
        if (state.phase != GamePhase::Setup)
            return;

        // 如果已有足夠卡牌，不需要加入
        if (state.cards.size() >= count)
            return;

        // 限制數量
        count = juce::jlimit (1, static_cast<int> (GameState::MaxCards), count);

        // 測試卡牌名稱
        juce::StringArray testNames = { "Kick Drum", "Snare", "Hi-Hat", "Bass",
                                        "Piano", "Guitar", "Synth Lead", "Vocal" };

        // 從現有數量開始加入測試卡牌
        int startId = state.cards.size();
        for (int i = startId; i < count; ++i)
        {
            CardSlot card;
            card.id = i;
            card.realTrackName = testNames[i % testNames.size()];
            card.displayPosition = i;
            state.cards.add (card);
        }

        // 自動選擇第一張卡牌（如果尚未選擇）
        if (state.selectedCardId < 0)
            state.selectedCardId = 0;

        shouldNotify = true;

        DBG ("BlindCardManager: Added test cards, total now: " << state.cards.size());
    }
    if (shouldNotify)
        sendChangeMessage();
}

} // namespace blindcard
