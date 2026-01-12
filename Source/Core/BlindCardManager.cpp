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
            state.selectedCardId = -1;
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
        state.selectedCardId = -1;
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
        state.selectedCardId = -1;

        // 清除評分資料但保留卡牌
        for (auto& card : state.cards)
        {
            card.rounds.clear();
            card.displayPosition = card.id;
            card.isRemoved = false;
        }

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

        if (state.phase != GamePhase::BlindTesting)
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
