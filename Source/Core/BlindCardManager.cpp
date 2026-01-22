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

        // 分配隨機撲克牌數值（避免與現有卡牌重複）
        std::set<std::pair<int, int>> usedCards;
        for (const auto& existingCard : state.cards)
            usedCards.insert ({ existingCard.cardValue, existingCard.suitIndex });

        // 建立可用的撲克牌列表
        juce::Array<std::pair<int, int>> availableCards;
        for (int suit = 0; suit < 4; ++suit)
            for (int value = 1; value <= 13; ++value)
                if (usedCards.find ({ value, suit }) == usedCards.end())
                    availableCards.add ({ value, suit });

        // 隨機選擇一張
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
            // 備用：如果沒有可用的（不太可能），用隨機值
            std::random_device rd;
            std::mt19937 gen (rd());
            card.cardValue = std::uniform_int_distribution<> (1, 13) (gen);
            card.suitIndex = std::uniform_int_distribution<> (0, 3) (gen);
        }

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

                // 更新 selectedCardId 以保持有效
                if (state.selectedCardId == index)
                {
                    // 被選中的卡牌被移除了，選擇第一張
                    state.selectedCardId = state.cards.isEmpty() ? -1 : 0;
                }
                else if (state.selectedCardId > index)
                {
                    // 被選中的卡牌在被移除的後面，ID 減 1
                    state.selectedCardId--;
                }

                // 通知所有 processor 更新它們的 cardId
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

        // 注意：卡牌數值 (cardValue, suitIndex) 已在卡牌創建時分配，
        // shuffle() 只打亂顯示位置，不改變卡牌花色數字

        state.phase = GamePhase::BlindTesting;
        state.currentRound = 0;

        // 選擇視覺上第一個位置（displayPosition=0）的卡片
        // 這樣用戶無法通過播放位置猜測卡片身份
        state.selectedCardId = -1;
        for (int i = 0; i < state.cards.size(); ++i)
        {
            if (state.cards[i].displayPosition == 0)
            {
                state.selectedCardId = i;
                break;
            }
        }

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

        // Allow transition from both BlindTesting and Revealed phases
        if (state.phase != GamePhase::BlindTesting && state.phase != GamePhase::Revealed)
            return;

        if (state.currentRound < state.totalRounds - 1)
        {
            state.currentRound++;

            // 每輪重新洗牌 - 打亂 displayPosition
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

            // 選擇視覺上第一個位置（displayPosition=0）的卡片
            // 這樣用戶無法通過播放位置猜測卡片身份
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

        // 重新分配隨機撲克牌數值（每場新遊戲都不同）
        if (!state.cards.isEmpty())
        {
            std::random_device rd;
            std::mt19937 gen (rd());

            // 建立一副 52 張撲克牌
            struct PokerCard { int value; int suit; };
            juce::Array<PokerCard> deck;
            for (int suit = 0; suit < 4; ++suit)
                for (int value = 1; value <= 13; ++value)
                    deck.add ({ value, suit });

            // 洗牌
            for (int i = deck.size() - 1; i > 0; --i)
            {
                std::uniform_int_distribution<> dis (0, i);
                int j = dis (gen);
                deck.swap (i, j);
            }

            // 分配給卡牌
            for (int i = 0; i < state.cards.size(); ++i)
            {
                auto& card = state.cards.getReference (i);
                card.cardValue = deck[i].value;
                card.suitIndex = deck[i].suit;
            }
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

        // 允許在所有階段選擇卡牌（包括 Revealed 階段，讓用戶可以重聽）
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

        // 允許任何階段更新軌道名稱（UI 會在盲測時隱藏真實名稱）
        if (cardId >= 0 && cardId < state.cards.size())
        {
            // 只有當名稱真正改變時才更新
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

    // 在鎖外呼叫 processor 方法（避免死鎖）
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

    // 盲測階段只顯示卡片編號，不顯示軌道名
    if (state.phase == GamePhase::BlindTesting)
        return "Card " + juce::String (card.displayPosition + 1);

    // Setup 或 Revealed 階段顯示軌道名
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
    bool shouldRecalculate = false;
    bool shouldAutoLock = false;
    {
        juce::ScopedLock sl (lock);
        if (cardId >= 0 && cardId < state.cards.size())
        {
            state.cards.getReference (cardId).measuredLUFS = lufs;
            shouldNotify = true;

            // 校準期間不自動重算增益（等 lockCalibration 時一次算）
            // 但如果不在校準中（手動設定 LUFS），則自動重算
            if (!calibrating_)
                shouldRecalculate = true;

            DBG ("BlindCardManager: Card " << cardId << " LUFS = " << lufs << " dB");

            // 校準期間：檢查是否所有卡牌都已測量完成
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

                // 所有卡牌都測量完成，自動鎖定校準
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

    // 收集所有有效的 LUFS 測量值
    std::vector<float> lufsValues;
    for (const auto& card : state.cards)
    {
        if (card.hasLUFSMeasurement() && !card.isRemoved)
            lufsValues.push_back (card.measuredLUFS);
    }

    // 如果沒有有效測量值，重置所有增益
    if (lufsValues.empty())
    {
        for (auto& card : state.cards)
            card.autoGainDb = 0.0f;
        return;
    }

    // 計算中位數作為基準
    std::sort (lufsValues.begin(), lufsValues.end());
    float medianLUFS;
    size_t n = lufsValues.size();
    if (n % 2 == 1)
    {
        // 奇數：取正中間
        medianLUFS = lufsValues[n / 2];
    }
    else
    {
        // 偶數：取中間兩個的較小值（更保守，避免音量過大）
        medianLUFS = lufsValues[n / 2 - 1];
    }

    // 使用中位數作為基準計算增益
    for (auto& card : state.cards)
    {
        if (card.hasLUFSMeasurement() && !card.isRemoved)
        {
            // 計算需要的增益來達到中位數響度
            card.autoGainDb = medianLUFS - card.measuredLUFS;
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

    // 如果 Level-Match 未啟用或尚未校準完成，不應用增益
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
        // 重置校準狀態
        calibrating_ = false;
        calibrated_ = false;
        shouldNotify = true;
    }
    if (shouldNotify)
        sendChangeMessage();
}

// ============================================================================
// Level Matching - 校準流程
// ============================================================================

void BlindCardManager::startCalibration()
{
    bool shouldNotify = false;
    juce::Array<BlindCardProcessor*> instancesCopy;
    {
        juce::ScopedLock sl (lock);

        // 只在 Setup 階段允許開始校準
        if (state.phase != GamePhase::Setup)
            return;

        // 重置所有測量數據
        for (auto& card : state.cards)
        {
            card.measuredLUFS = -100.0f;
            card.autoGainDb = 0.0f;
        }

        calibrating_ = true;
        calibrated_ = false;
        calibrationStartTime_ = juce::Time::currentTimeMillis();

        // 複製實例列表（避免在鎖內呼叫外部方法）
        instancesCopy = instances;
        shouldNotify = true;

        DBG ("BlindCardManager: Calibration started");
    }

    // 在鎖外啟動所有實例的測量
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

        // 檢查是否所有卡牌都有測量值
        int measuredCount = 0;
        for (const auto& card : state.cards)
        {
            if (card.hasLUFSMeasurement() && !card.isRemoved)
                measuredCount++;
        }

        // 至少需要有測量值才能鎖定
        if (measuredCount == 0)
        {
            DBG ("BlindCardManager: Cannot lock calibration - no measurements");
            return;
        }

        // 計算增益（使用中位數基準）
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

        // 如果正在倒數中，忽略點擊（讓用戶看完答案）
        if (state.qaState.isShowingAnswer)
            return;

        bool correct = (selectedCardId == state.qaState.targetCardId);
        state.qaState.answers.add (correct);
        state.qaState.lastFeedback = correct ? QAState::FeedbackState::Correct
                                             : QAState::FeedbackState::Wrong;
        state.qaState.lastAnsweredCardId = selectedCardId;

        // 開始顯示答案倒數（3 秒）
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
    int activeCount = 0;
    for (const auto& card : state.cards)
    {
        if (!card.isRemoved)
            activeCount++;
    }
    // 使用用戶設定的問題數，但不超過活躍卡牌數
    return juce::jmin (state.qaQuestionCount, activeCount);
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
            // 倒數結束，前進到下一題
            state.qaState.isShowingAnswer = false;
            state.qaState.countdownValue = 0;
            state.qaState.revealedTargetCardId = -1;
            state.qaState.currentQuestion++;

            int maxQ = getQAMaxQuestions();
            if (state.qaState.isComplete (maxQ))
            {
                // 所有問題完成，進入 Revealed 階段
                state.phase = GamePhase::Revealed;
            }
            else
            {
                // 還有問題，選擇下一題
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

        // 直接結束倒數
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
    // 如果正在顯示答案，回傳揭曉的正確卡牌 ID
    if (state.qaState.isShowingAnswer)
        return state.qaState.revealedTargetCardId;
    return -1;
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

        // 建立一副 52 張撲克牌並洗牌（確保不重複）
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

        // 從現有數量開始加入測試卡牌
        int startId = state.cards.size();
        for (int i = startId; i < count; ++i)
        {
            CardSlot card;
            card.id = i;
            card.realTrackName = testNames[i % testNames.size()];
            card.displayPosition = i;

            // 從洗好的牌堆分配（保證不重複）
            card.cardValue = deck[i].value;
            card.suitIndex = deck[i].suit;

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

void BlindCardManager::setTestCardCount (int count)
{
    bool shouldNotify = false;
    {
        juce::ScopedLock sl (lock);

        // 只在 Setup 階段允許變更
        if (state.phase != GamePhase::Setup)
            return;

        // 限制數量
        count = juce::jlimit (static_cast<int> (GameState::MinCards),
                              static_cast<int> (GameState::MaxCards), count);

        int currentCount = state.cards.size();

        // 如果數量相同，不需要變更
        if (count == currentCount)
            return;

        // 測試卡牌名稱
        juce::StringArray testNames = { "Kick Drum", "Snare", "Hi-Hat", "Bass",
                                        "Piano", "Guitar", "Synth Lead", "Vocal" };

        if (count > currentCount)
        {
            // 增加卡牌：收集現有的撲克牌值，避免重複
            std::set<std::pair<int, int>> usedCards;
            for (const auto& card : state.cards)
                usedCards.insert ({ card.cardValue, card.suitIndex });

            // 建立剩餘可用的撲克牌
            std::random_device rd;
            std::mt19937 gen (rd());

            juce::Array<std::pair<int, int>> availableCards;
            for (int suit = 0; suit < 4; ++suit)
                for (int value = 1; value <= 13; ++value)
                    if (usedCards.find ({ value, suit }) == usedCards.end())
                        availableCards.add ({ value, suit });

            // 洗牌
            for (int i = availableCards.size() - 1; i > 0; --i)
            {
                std::uniform_int_distribution<> dis (0, i);
                int j = dis (gen);
                availableCards.swap (i, j);
            }

            // 加入新卡牌
            int cardIdx = 0;
            for (int i = currentCount; i < count; ++i)
            {
                CardSlot card;
                card.id = i;
                // Standalone 模式下不自動分配軌道名稱，等用戶拖入音訊檔案
                card.realTrackName = standaloneMode ? "" : testNames[i % testNames.size()];
                card.displayPosition = i;

                // 從可用牌堆分配
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
            // 減少卡牌：移除多餘的卡牌（從後面移除）
            while (state.cards.size() > count)
                state.cards.removeLast();

            // 確保選中的卡牌仍然有效
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

    // 清除所有現有卡牌的軌道名稱（如果切換到 Standalone 模式）
    if (enabled)
    {
        for (auto& card : state.cards)
        {
            card.realTrackName = "";
        }
    }
}

} // namespace blindcard
