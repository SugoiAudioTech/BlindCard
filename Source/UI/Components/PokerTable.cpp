/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    PokerTable.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "PokerTable.h"
#include "BinaryData.h"

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
        card->setStandaloneMode(standaloneMode);  // Inherit standalone mode

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
            // Only reveal if not already revealed (avoid flipping back)
            if (!card->getCardData().isRevealed)
            {
                card->flip();
            }
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

void PokerTable::setSelectedCard(int index)
{
    if (selectedCardIndex == index) return;

    // Deselect previous
    if (selectedCardIndex >= 0 && selectedCardIndex < static_cast<int>(cards.size()))
    {
        cards[static_cast<size_t>(selectedCardIndex)]->setSelected(false);
    }

    // Select new
    selectedCardIndex = index;
    if (index >= 0 && index < static_cast<int>(cards.size()))
    {
        cards[static_cast<size_t>(index)]->setSelected(true);
    }
}

void PokerTable::setStandaloneMode(bool enabled)
{
    standaloneMode = enabled;
    for (auto& card : cards)
    {
        card->setStandaloneMode(enabled);
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

    // Stadium/pill shape - corner radius is half the height for full rounded ends
    float stadiumRadius = bounds.getHeight() / 2.0f;

    // Salmon/coral padded frame with subtle gradient for depth
    juce::ColourGradient frameGradient(
        tm.getColour(ColourId::TableFrameOuter),
        bounds.getTopLeft(),
        tm.getColour(ColourId::TableFrameInner),
        bounds.getBottomRight(),
        false
    );

    g.setGradientFill(frameGradient);
    g.fillRoundedRectangle(bounds, stadiumRadius);

    // Subtle highlight on top edge for 3D effect
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), stadiumRadius - 1.0f, 1.5f);

    // Inner shadow for depth
    auto innerBounds = bounds.reduced(kFrameThickness - 2);
    float innerRadius = innerBounds.getHeight() / 2.0f;
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawRoundedRectangle(innerBounds, innerRadius, 2.0f);
}

void PokerTable::drawTableRail(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Stadium shape for rail
    float stadiumRadius = bounds.getHeight() / 2.0f;

    // Darker coral rail/padding area
    g.setColour(tm.getColour(ColourId::TableRail));
    g.fillRoundedRectangle(bounds, stadiumRadius);

    // Subtle inner shadow for depth
    auto innerBounds = bounds.reduced(2.0f);
    float innerRadius = innerBounds.getHeight() / 2.0f;
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.drawRoundedRectangle(innerBounds, innerRadius, 1.5f);
}

void PokerTable::drawNeonBorder(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto neonColor = tm.getColour(ColourId::NeonRed);

    // Stadium shape radius
    float stadiumRadius = bounds.getHeight() / 2.0f;

    // Outer glow (multiple passes for blur effect) - more pronounced
    for (int i = 6; i >= 1; --i)
    {
        float alpha = 0.05f + (6 - i) * 0.03f;
        float expand = static_cast<float>(i * 3);
        auto expandedBounds = bounds.expanded(expand);
        float expandedRadius = expandedBounds.getHeight() / 2.0f;
        g.setColour(neonColor.withAlpha(alpha));
        g.drawRoundedRectangle(expandedBounds, expandedRadius, 3.0f);
    }

    // Main neon line - thicker and brighter
    g.setColour(neonColor);
    g.drawRoundedRectangle(bounds, stadiumRadius, static_cast<float>(kNeonBorderWidth) + 1.0f);

    // Inner highlight (brighter center for glow effect)
    auto innerBounds1 = bounds.reduced(1.0f);
    float innerRadius1 = innerBounds1.getHeight() / 2.0f;
    g.setColour(neonColor.brighter(0.5f).withAlpha(0.9f));
    g.drawRoundedRectangle(innerBounds1, innerRadius1, 1.5f);

    // Core highlight (white center for intensity)
    auto innerBounds2 = bounds.reduced(1.5f);
    float innerRadius2 = innerBounds2.getHeight() / 2.0f;
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRoundedRectangle(innerBounds2, innerRadius2, 0.5f);
}

void PokerTable::drawFeltSurface(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Stadium shape radius
    float stadiumRadius = bounds.getHeight() / 2.0f;

    // Green radial gradient for felt (lighter in center, darker at edges)
    juce::ColourGradient feltGradient(
        tm.getColour(ColourId::TableFeltCenter),
        bounds.getCentre(),
        tm.getColour(ColourId::TableFelt),
        bounds.getTopLeft(),
        true  // radial
    );

    g.setGradientFill(feltGradient);
    g.fillRoundedRectangle(bounds, stadiumRadius);

    // Subtle texture overlay (fine grain pattern for felt texture)
    juce::Random random(42);  // Fixed seed for consistent pattern
    g.setColour(juce::Colours::black.withAlpha(0.015f));

    for (int i = 0; i < 600; ++i)
    {
        float x = bounds.getX() + random.nextFloat() * bounds.getWidth();
        float y = bounds.getY() + random.nextFloat() * bounds.getHeight();
        float size = 0.5f + random.nextFloat() * 1.0f;
        g.fillEllipse(x, y, size, size);
    }

    // Draw "SUGOI AUDIO" branding text on the felt (like "TEXAS HOLD'EM" style)
    drawBrandingText(g, bounds);

    // Vignette effect (subtle darker edges)
    juce::ColourGradient vignetteGradient(
        juce::Colours::transparentBlack,
        bounds.getCentre(),
        juce::Colours::black.withAlpha(0.15f),
        bounds.getTopLeft(),
        true  // radial
    );
    g.setGradientFill(vignetteGradient);
    g.fillRoundedRectangle(bounds, stadiumRadius);
}

void PokerTable::drawBrandingText(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Load Sugoi logo from binary data
    static juce::Image logoImage = juce::ImageCache::getFromMemory(
        BinaryData::sugoilogo_png, BinaryData::sugoilogo_pngSize);

    if (!logoImage.isValid())
        return;

    // Create a monochrome silhouette version of the logo (darker felt color)
    // This creates the "embossed into felt" effect like casino tables
    static juce::Image silhouetteImage;
    static bool silhouetteCreated = false;

    if (!silhouetteCreated)
    {
        silhouetteImage = logoImage.createCopy();
        // Convert to single-color silhouette (dark green, felt-like)
        juce::Colour silhouetteColour(0xFF1A5C3A);  // Darker green than felt

        juce::Image::BitmapData bitmap(silhouetteImage, juce::Image::BitmapData::readWrite);
        for (int y = 0; y < bitmap.height; ++y)
        {
            for (int x = 0; x < bitmap.width; ++x)
            {
                auto pixel = bitmap.getPixelColour(x, y);

                // Check if pixel is white or near-white (background)
                // White = high R, G, B values
                bool isWhiteBackground = (pixel.getRed() > 240 &&
                                          pixel.getGreen() > 240 &&
                                          pixel.getBlue() > 240);

                if (isWhiteBackground || pixel.getAlpha() < 10)
                {
                    // Make background transparent
                    bitmap.setPixelColour(x, y, juce::Colours::transparentBlack);
                }
                else
                {
                    // Convert actual logo content to silhouette color
                    bitmap.setPixelColour(x, y, silhouetteColour.withAlpha((juce::uint8)255));
                }
            }
        }
        silhouetteCreated = true;
    }

    // Logo size (scaled to fit nicely on the table)
    float logoHeight = bounds.getHeight() * 0.5f;
    float aspectRatio = static_cast<float>(silhouetteImage.getWidth()) / static_cast<float>(silhouetteImage.getHeight());
    float logoWidth = logoHeight * aspectRatio;

    // Position: distance from edge
    float chipEndOffset = 150.0f;

    float leftX = bounds.getX() + chipEndOffset;
    float rightX = bounds.getRight() - chipEndOffset;
    float centreY = bounds.getCentreY();

    // Subtle opacity for watermark effect
    float opacity = 0.4f;

    // Text setup - "Sugoi" below logo (same style as original text version)
    float fontSize = logoHeight * 0.25f;
    juce::Font brandFont(juce::FontOptions(fontSize).withStyle("Bold"));
    brandFont.setHorizontalScale(0.85f);  // Slightly condensed
    float textHeight = fontSize * 1.2f;
    float textYOffset = logoHeight / 2.0f + 5.0f;  // Below logo

    // === LEFT SIDE (facing forward) ===
    juce::Rectangle<float> leftLogoBounds(
        leftX - logoWidth / 2.0f,
        centreY - logoHeight / 2.0f - textHeight / 2.0f,
        logoWidth,
        logoHeight
    );
    g.setOpacity(opacity);
    g.drawImage(silhouetteImage, leftLogoBounds,
                juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

    // "SUGOI" text below left logo (white, semi-transparent like original)
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.setFont(brandFont);
    g.drawText("SUGOI",
               static_cast<int>(leftX - logoWidth / 2.0f),
               static_cast<int>(centreY + textYOffset - textHeight / 2.0f),
               static_cast<int>(logoWidth),
               static_cast<int>(textHeight),
               juce::Justification::centred, false);

    // === RIGHT SIDE (facing forward) ===
    juce::Rectangle<float> rightLogoBounds(
        rightX - logoWidth / 2.0f,
        centreY - logoHeight / 2.0f - textHeight / 2.0f,
        logoWidth,
        logoHeight
    );
    g.setOpacity(opacity);
    g.drawImage(silhouetteImage, rightLogoBounds,
                juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

    // "SUGOI" text below right logo (white, semi-transparent like original)
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.setFont(brandFont);
    g.drawText("SUGOI",
               static_cast<int>(rightX - logoWidth / 2.0f),
               static_cast<int>(centreY + textYOffset - textHeight / 2.0f),
               static_cast<int>(logoWidth),
               static_cast<int>(textHeight),
               juce::Justification::centred, false);
}

//==============================================================================
void PokerTable::resized()
{
    auto feltArea = getFeltArea();

    // Position chip stacks on the sides
    int chipStackWidth = 60;
    int chipStackHeight = 140;  // Increased to fit 5-chip stack with 3D edges
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

    // Reserved space for chip stacks and vertical padding
    int chipStackSpace = 80;
    int verticalPadding = 30;  // More space from table edges
    auto cardArea = feltArea.reduced(chipStackSpace, verticalPadding);

    int cardCount = static_cast<int>(cards.size());
    int cardWidth = PokerCard::kDefaultWidth;
    int cardHeight = PokerCard::kDefaultHeight;

    // Extra height for controls (Q&A button needs 28px height below card)
    // Button starts at kDefaultHeight - 4 = 166, ends at 166 + 28 = 194
    // So we need at least 194 - 170 = 24px extra, use 30 for padding
    int extraHeightForControls = 30;
    int componentHeight = cardHeight + extraHeightForControls;

    // Gap between cards
    int horizontalGap = 16;
    int verticalGap = 12;

    if (cardCount <= 4)
    {
        // Single row layout
        int totalWidth = cardCount * cardWidth + (cardCount - 1) * horizontalGap;
        int startX = cardArea.getCentreX() - totalWidth / 2;
        int startY = cardArea.getCentreY() - componentHeight / 2;

        for (int i = 0; i < cardCount; ++i)
        {
            cards[i]->setBounds(
                startX + i * (cardWidth + horizontalGap),
                startY,
                cardWidth,
                componentHeight
            );
        }
    }
    else
    {
        // Double row layout (4 on top, remaining on bottom)
        // Cards keep original size - window should be resized to fit
        int topRowCount = 4;
        int bottomRowCount = cardCount - 4;

        // Calculate total height and center vertically
        int totalHeight = componentHeight * 2 + verticalGap;
        int topY = cardArea.getCentreY() - totalHeight / 2;
        int bottomY = topY + componentHeight + verticalGap;

        // Top row
        int topTotalWidth = topRowCount * cardWidth + (topRowCount - 1) * horizontalGap;
        int topStartX = cardArea.getCentreX() - topTotalWidth / 2;

        for (int i = 0; i < topRowCount; ++i)
        {
            cards[i]->setBounds(
                topStartX + i * (cardWidth + horizontalGap),
                topY,
                cardWidth,
                componentHeight
            );
        }

        // Bottom row (centered)
        int bottomTotalWidth = bottomRowCount * cardWidth + (bottomRowCount - 1) * horizontalGap;
        int bottomStartX = cardArea.getCentreX() - bottomTotalWidth / 2;

        for (int i = 0; i < bottomRowCount; ++i)
        {
            cards[topRowCount + i]->setBounds(
                bottomStartX + i * (cardWidth + horizontalGap),
                bottomY,
                cardWidth,
                componentHeight
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
