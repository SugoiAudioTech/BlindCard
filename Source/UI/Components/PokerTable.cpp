/*
  ==============================================================================

    PokerTable.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "PokerTable.h"

namespace BlindCard
{

//==============================================================================
PokerTable::PokerTable()
{
    ThemeManager::getInstance().addChangeListener(this);

    // Create decorative chip stacks
    leftChipStack = std::make_unique<ChipStack>(5, true);   // 5 chips, red first
    rightChipStack = std::make_unique<ChipStack>(4, false); // 4 chips, black first

    addAndMakeVisible(*leftChipStack);
    addAndMakeVisible(*rightChipStack);

    // Start with 4 cards by default
    setCardCount(4);
}

PokerTable::~PokerTable()
{
    ThemeManager::getInstance().removeChangeListener(this);
}

//==============================================================================
void PokerTable::setCardCount(int count)
{
    count = juce::jlimit(2, 8, count);

    // Remove excess cards
    while (static_cast<int>(cards.size()) > count)
    {
        removeChildComponent(cards.back().get());
        cards.pop_back();
    }

    // Add new cards
    while (static_cast<int>(cards.size()) < count)
    {
        auto card = std::make_unique<PokerCard>();
        int index = static_cast<int>(cards.size());

        // Set initial card data
        CardData data;
        data.position = index;
        card->setCardData(data);
        card->setMode(currentMode);
        card->setPhase(currentPhase);

        if (!trackList.empty())
        {
            card->setTracks(trackList);
        }

        setupCardCallbacks(*card, index);
        addAndMakeVisible(*card);
        cards.push_back(std::move(card));
    }

    updateCardLayout();
}

PokerCard* PokerTable::getCard(int index)
{
    if (index >= 0 && index < static_cast<int>(cards.size()))
    {
        return cards[index].get();
    }
    return nullptr;
}

//==============================================================================
void PokerTable::setMode(blindcard::RatingMode mode)
{
    currentMode = mode;
    for (auto& card : cards)
    {
        card->setMode(mode);
    }
}

void PokerTable::setPhase(blindcard::GamePhase phase)
{
    currentPhase = phase;
    for (auto& card : cards)
    {
        card->setPhase(phase);
    }
}

void PokerTable::setTracks(const std::vector<std::string>& tracks)
{
    trackList = tracks;
    for (auto& card : cards)
    {
        card->setTracks(tracks);
    }
}

//==============================================================================
void PokerTable::revealAllCards(int delayBetweenCardsMs)
{
    // Staggered reveal animation
    for (size_t i = 0; i < cards.size(); ++i)
    {
        auto* card = cards[i].get();

        // Use a timer for staggered effect
        juce::Timer::callAfterDelay(static_cast<int>(i) * delayBetweenCardsMs, [card]()
        {
            card->flip();
        });
    }
}

void PokerTable::hideAllCards()
{
    for (auto& card : cards)
    {
        CardData data = card->getCardData();
        if (data.isRevealed)
        {
            card->flip();
        }
    }
}

void PokerTable::shuffleCards()
{
    hideAllCards();
    // Reset all card states
    for (size_t i = 0; i < cards.size(); ++i)
    {
        CardData data;
        data.position = static_cast<int>(i);
        data.isRevealed = false;
        data.isSelected = false;
        data.isPlaying = false;
        data.rating = 0;
        data.guessedTrackIndex = -1;
        cards[i]->setCardData(data);
    }
}

//==============================================================================
void PokerTable::changeListenerCallback(juce::ChangeBroadcaster*)
{
    repaint();
}

//==============================================================================
void PokerTable::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Draw wood frame (outermost)
    drawWoodFrame(g, bounds);

    // 2. Draw table rail
    auto railBounds = bounds.reduced(kFrameThickness);
    drawTableRail(g, railBounds);

    // 3. Draw neon border
    auto neonBounds = railBounds.reduced(kRailThickness);
    drawNeonBorder(g, neonBounds);

    // 4. Draw felt surface
    auto feltBounds = neonBounds.reduced(kNeonBorderWidth + 2);
    drawFeltSurface(g, feltBounds);
}

void PokerTable::drawWoodFrame(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Wood gradient from light to dark
    juce::ColourGradient woodGradient(
        tm.getColour(ColourId::TableWoodLight),
        bounds.getTopLeft(),
        tm.getColour(ColourId::TableWoodDark),
        bounds.getBottomRight(),
        true
    );

    g.setGradientFill(woodGradient);
    g.fillRoundedRectangle(bounds, 12.0f);

    // Inner shadow for depth
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(kFrameThickness - 2), 8.0f, 2.0f);
}

void PokerTable::drawTableRail(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Darker rail edge
    g.setColour(tm.getColour(ColourId::TableRail));
    g.fillRoundedRectangle(bounds, 8.0f);

    // Subtle highlight on top edge
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    auto highlightArea = bounds.removeFromTop(2.0f);
    g.fillRoundedRectangle(highlightArea.withWidth(bounds.getWidth() * 0.6f)
                                        .withX(bounds.getCentreX() - bounds.getWidth() * 0.3f),
                          1.0f);
}

void PokerTable::drawNeonBorder(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto neonColor = tm.getColour(ColourId::NeonRed);

    // Outer glow (multiple passes for blur effect)
    for (int i = 4; i >= 1; --i)
    {
        float alpha = 0.1f + (4 - i) * 0.05f;
        float expand = static_cast<float>(i * 2);
        g.setColour(neonColor.withAlpha(alpha));
        g.drawRoundedRectangle(bounds.expanded(expand), 6.0f + expand, 2.0f);
    }

    // Main neon line
    g.setColour(neonColor);
    g.drawRoundedRectangle(bounds, 6.0f, static_cast<float>(kNeonBorderWidth));

    // Inner highlight (brighter center)
    g.setColour(neonColor.brighter(0.3f).withAlpha(0.8f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.0f);
}

void PokerTable::drawFeltSurface(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Radial gradient for felt (lighter in center)
    juce::ColourGradient feltGradient(
        tm.getColour(ColourId::TableFeltCenter),
        bounds.getCentre(),
        tm.getColour(ColourId::TableFelt),
        bounds.getTopLeft(),
        true  // radial
    );

    g.setGradientFill(feltGradient);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Subtle texture overlay (noise-like pattern)
    juce::Random random(42);  // Fixed seed for consistent pattern
    g.setColour(juce::Colours::black.withAlpha(0.02f));

    for (int i = 0; i < 200; ++i)
    {
        float x = bounds.getX() + random.nextFloat() * bounds.getWidth();
        float y = bounds.getY() + random.nextFloat() * bounds.getHeight();
        float size = 1.0f + random.nextFloat() * 2.0f;
        g.fillEllipse(x, y, size, size);
    }

    // Vignette effect (darker edges)
    juce::ColourGradient vignetteGradient(
        juce::Colours::transparentBlack,
        bounds.getCentre(),
        juce::Colours::black.withAlpha(0.15f),
        bounds.getTopLeft(),
        true  // radial
    );
    g.setGradientFill(vignetteGradient);
    g.fillRoundedRectangle(bounds, 4.0f);
}

//==============================================================================
void PokerTable::resized()
{
    auto feltArea = getFeltArea();

    // Position chip stacks on the sides
    int chipStackWidth = 60;
    int chipStackHeight = 100;
    int chipMargin = 20;

    leftChipStack->setBounds(
        feltArea.getX() + chipMargin,
        feltArea.getCentreY() - chipStackHeight / 2,
        chipStackWidth,
        chipStackHeight
    );

    rightChipStack->setBounds(
        feltArea.getRight() - chipMargin - chipStackWidth,
        feltArea.getCentreY() - chipStackHeight / 2,
        chipStackWidth,
        chipStackHeight
    );

    updateCardLayout();
}

juce::Rectangle<int> PokerTable::getFeltArea() const
{
    auto bounds = getLocalBounds();
    int inset = kFrameThickness + kRailThickness + kNeonBorderWidth + 4;
    return bounds.reduced(inset);
}

void PokerTable::updateCardLayout()
{
    if (cards.empty())
        return;

    auto feltArea = getFeltArea();

    // Reserved space for chip stacks
    int chipStackSpace = 80;
    auto cardArea = feltArea.reduced(chipStackSpace, 20);

    int cardCount = static_cast<int>(cards.size());
    int cardWidth = PokerCard::kDefaultWidth;
    int cardHeight = PokerCard::kDefaultHeight;

    // Gap between cards
    int horizontalGap = 20;
    int verticalGap = 15;

    if (cardCount <= 4)
    {
        // Single row layout
        int totalWidth = cardCount * cardWidth + (cardCount - 1) * horizontalGap;
        int startX = cardArea.getCentreX() - totalWidth / 2;
        int startY = cardArea.getCentreY() - cardHeight / 2;

        for (int i = 0; i < cardCount; ++i)
        {
            cards[i]->setBounds(
                startX + i * (cardWidth + horizontalGap),
                startY,
                cardWidth,
                cardHeight
            );
        }
    }
    else
    {
        // Double row layout (4 on top, remaining on bottom)
        int topRowCount = 4;
        int bottomRowCount = cardCount - 4;

        // Top row
        int topTotalWidth = topRowCount * cardWidth + (topRowCount - 1) * horizontalGap;
        int topStartX = cardArea.getCentreX() - topTotalWidth / 2;
        int topY = cardArea.getCentreY() - cardHeight - verticalGap / 2;

        for (int i = 0; i < topRowCount; ++i)
        {
            cards[i]->setBounds(
                topStartX + i * (cardWidth + horizontalGap),
                topY,
                cardWidth,
                cardHeight
            );
        }

        // Bottom row (centered)
        int bottomTotalWidth = bottomRowCount * cardWidth + (bottomRowCount - 1) * horizontalGap;
        int bottomStartX = cardArea.getCentreX() - bottomTotalWidth / 2;
        int bottomY = cardArea.getCentreY() + verticalGap / 2;

        for (int i = 0; i < bottomRowCount; ++i)
        {
            cards[topRowCount + i]->setBounds(
                bottomStartX + i * (cardWidth + horizontalGap),
                bottomY,
                cardWidth,
                cardHeight
            );
        }
    }
}

//==============================================================================
void PokerTable::setupCardCallbacks(PokerCard& card, int index)
{
    card.onClicked = [this, index]()
    {
        if (onCardClicked)
            onCardClicked(index);
    };

    card.onRatingChanged = [this, index](int rating)
    {
        if (onCardRatingChanged)
            onCardRatingChanged(index, rating);
    };

    card.onGuessChanged = [this, index](int trackIndex)
    {
        if (onCardGuessChanged)
            onCardGuessChanged(index, trackIndex);
    };

    card.onQASelected = [this, index]()
    {
        if (onCardQASelected)
            onCardQASelected(index);
    };
}

} // namespace BlindCard
