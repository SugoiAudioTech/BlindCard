#include "BlindCardManager.h"
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

    notifyListeners();
    return card.id;
}

void BlindCardManager::unregisterInstance (BlindCardProcessor* instance)
{
    juce::ScopedLock sl (lock);

    int index = instances.indexOf (instance);
    if (index < 0)
        return;

    instances.remove (index);

    // 如果在盲測中，標記卡牌為已移除而非刪除
    if (state.phase == GamePhase::BlindTesting && index < state.cards.size())
    {
        state.cards.getReference (index).isRemoved = true;
    }
    else if (state.phase == GamePhase::Setup && index < state.cards.size())
    {
        state.cards.remove (index);
        // 重新編號
        for (int i = 0; i < state.cards.size(); ++i)
        {
            state.cards.getReference (i).id = i;
            state.cards.getReference (i).displayPosition = i;
        }
    }

    notifyListeners();
}

void BlindCardManager::setTotalRounds (int rounds)
{
    juce::ScopedLock sl (lock);
    if (state.phase == GamePhase::Setup)
    {
        state.totalRounds = juce::jlimit (1, 10, rounds);
        notifyListeners();
    }
}

void BlindCardManager::shuffle()
{
    juce::ScopedLock sl (lock);

    if (state.phase != GamePhase::Setup)
        return;

    if (state.cards.isEmpty())
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
    state.selectedCardId = -1;

    notifyListeners();
}

void BlindCardManager::nextRound()
{
    juce::ScopedLock sl (lock);

    if (state.phase != GamePhase::BlindTesting)
        return;

    if (state.currentRound < state.totalRounds - 1)
    {
        state.currentRound++;
        state.selectedCardId = -1;
        notifyListeners();
    }
}

void BlindCardManager::reveal()
{
    juce::ScopedLock sl (lock);

    if (state.phase != GamePhase::BlindTesting)
        return;

    state.phase = GamePhase::Revealed;
    state.selectedCardId = -1;
    notifyListeners();
}

void BlindCardManager::reset()
{
    juce::ScopedLock sl (lock);

    state.phase = GamePhase::Setup;
    state.currentRound = 0;
    state.selectedCardId = -1;

    // 清除評分資料但保留卡牌
    for (auto& card : state.cards)
    {
        card.rounds.clear();
        card.displayPosition = card.id;
        card.isRemoved = false;
    }

    notifyListeners();
}

void BlindCardManager::selectCard (int cardId)
{
    juce::ScopedLock sl (lock);

    if (state.phase != GamePhase::BlindTesting)
        return;

    if (cardId >= 0 && cardId < state.cards.size())
    {
        state.selectedCardId = cardId;
        notifyListeners();
    }
}

void BlindCardManager::deselectCard()
{
    juce::ScopedLock sl (lock);
    state.selectedCardId = -1;
    notifyListeners();
}

void BlindCardManager::rateCard (int cardId, int stars)
{
    juce::ScopedLock sl (lock);

    if (state.phase != GamePhase::BlindTesting)
        return;

    if (cardId >= 0 && cardId < state.cards.size())
    {
        auto& card = state.cards.getReference (cardId);
        if (state.currentRound < card.rounds.size())
        {
            card.rounds.getReference (state.currentRound).rating = juce::jlimit (0, 5, stars);
            notifyListeners();
        }
    }
}

void BlindCardManager::setNote (int cardId, const juce::String& note)
{
    juce::ScopedLock sl (lock);

    if (state.phase != GamePhase::BlindTesting)
        return;

    if (cardId >= 0 && cardId < state.cards.size())
    {
        auto& card = state.cards.getReference (cardId);
        if (state.currentRound < card.rounds.size())
        {
            card.rounds.getReference (state.currentRound).note = note;
            notifyListeners();
        }
    }
}

void BlindCardManager::setTrackName (int cardId, const juce::String& name)
{
    juce::ScopedLock sl (lock);

    if (state.phase != GamePhase::Setup)
        return;

    if (cardId >= 0 && cardId < state.cards.size())
    {
        state.cards.getReference (cardId).realTrackName = name;
        notifyListeners();
    }
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

void BlindCardManager::notifyListeners()
{
    sendChangeMessage();
}

} // namespace blindcard
