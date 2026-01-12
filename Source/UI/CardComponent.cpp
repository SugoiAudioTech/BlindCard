#include "CardComponent.h"

namespace blindcard
{

CardComponent::CardComponent()
{
    for (int i = 0; i < 5; ++i)
    {
        starButtons[i].setButtonText (juce::String::charToString (0x2606)); // 空心星
        starButtons[i].onClick = [this, i]()
        {
            if (onRated && cardData != nullptr)
                onRated (cardData->id, i + 1);
        };
        addAndMakeVisible (starButtons[i]);
    }
}

void CardComponent::setCard (const CardSlot* card, int currentRound, bool isSelected, GamePhase phase)
{
    cardData = card;
    round = currentRound;
    selected = isSelected;
    currentPhase = phase;

    updateStars();
    repaint();
}

void CardComponent::updateStars()
{
    if (cardData == nullptr)
        return;

    int rating = 0;
    if (round < cardData->rounds.size())
        rating = cardData->rounds[round].rating;

    for (int i = 0; i < 5; ++i)
    {
        bool filled = (i < rating);
        starButtons[i].setButtonText (juce::String::charToString (filled ? 0x2605 : 0x2606));
        starButtons[i].setEnabled (currentPhase == GamePhase::BlindTesting);
    }
}

void CardComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 卡牌背景
    juce::Colour bgColour = selected
        ? juce::Colours::steelblue
        : juce::Colours::darkslategrey;

    if (cardData != nullptr && cardData->isRemoved)
        bgColour = juce::Colours::dimgrey;

    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, 10.0f);

    // 卡牌邊框
    g.setColour (selected ? juce::Colours::white : juce::Colours::grey);
    g.drawRoundedRectangle (bounds.reduced (1), 10.0f, 2.0f);

    if (cardData == nullptr)
        return;

    // 顯示內容
    g.setColour (juce::Colours::white);
    g.setFont (18.0f);

    juce::String displayText;

    switch (currentPhase)
    {
        case GamePhase::Setup:
            displayText = cardData->realTrackName;
            break;

        case GamePhase::BlindTesting:
            // 盲測模式：顯示卡牌代號 (A, B, C...)
            displayText = juce::String::charToString ('A' + cardData->displayPosition);
            break;

        case GamePhase::Revealed:
            // 揭曉：顯示真實名稱
            displayText = cardData->realTrackName;
            break;
    }

    g.drawFittedText (displayText,
                      bounds.toNearestInt().reduced (10, 10).withHeight (30),
                      juce::Justification::centred, 1);

    // 揭曉時顯示平均分
    if (currentPhase == GamePhase::Revealed)
    {
        float avg = cardData->getAverageRating();
        if (avg > 0)
        {
            g.setFont (14.0f);
            g.drawFittedText (juce::String (avg, 1) + " stars",
                              bounds.toNearestInt().reduced (10).withTrimmedTop (40).withHeight (20),
                              juce::Justification::centred, 1);
        }
    }
}

void CardComponent::resized()
{
    auto bounds = getLocalBounds().reduced (10);
    auto starArea = bounds.removeFromBottom (30);

    int starWidth = starArea.getWidth() / 5;
    for (int i = 0; i < 5; ++i)
    {
        starButtons[i].setBounds (starArea.removeFromLeft (starWidth));
    }
}

void CardComponent::mouseDown (const juce::MouseEvent&)
{
    if (onClicked && cardData != nullptr)
        onClicked (cardData->id);
}

} // namespace blindcard
