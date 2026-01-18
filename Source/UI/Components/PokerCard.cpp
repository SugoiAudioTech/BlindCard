/*
  ==============================================================================

    PokerCard.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the PokerCard component with flip animation.

  ==============================================================================
*/

#include "PokerCard.h"
#include <cmath>

namespace BlindCard
{

//==============================================================================
// Suit Unicode symbols
namespace SuitSymbols
{
    const juce::String Spades   = juce::String::charToString(0x2660);   // Black Spade
    const juce::String Clubs    = juce::String::charToString(0x2663);   // Black Club
    const juce::String Diamonds = juce::String::charToString(0x2666);   // Black Diamond
    const juce::String Hearts   = juce::String::charToString(0x2665);   // Black Heart
}

//==============================================================================
// Layout constants
namespace CardLayout
{
    // Typography
    constexpr float cornerNumberFontSize = 24.0f;
    constexpr float cornerSuitFontSize = 18.0f;
    constexpr float centerSuitFontSize = 48.0f;
    constexpr float pluginNameFontSize = 12.0f;
    constexpr float positionLabelFontSize = 16.0f;

    // Margins and spacing
    constexpr float cornerPadding = 8.0f;
    constexpr float cornerSpacing = 2.0f;

    // Card back
    const juce::Colour cardBackColor { 0xFF1A3D5C };          // Dark blue
    const juce::Colour cardBackPatternColor { 0xFF2A5D8C };   // Lighter pattern
    const juce::Colour positionLabelColor { 0xFFFFD700 };     // Gold

    // Selection glow
    const juce::Colour selectionGlowColor { 0xFFFFD700 };     // Gold
    constexpr float glowRadius = 12.0f;

    // Playing indicator
    const juce::Colour playingColor { 0xFF22C55E };           // Green
    constexpr float playingIndicatorSize = 12.0f;

    // Reveal flash
    const juce::Colour revealFlashColor { 0xFFFFD700 };       // Gold

    // Child component positions
    constexpr int starRatingOffsetY = 8;
    constexpr int guessDropdownWidth = 120;
    constexpr int guessDropdownHeight = 28;
    constexpr int qaButtonWidth = 80;
    constexpr int qaButtonHeight = 28;
}

//==============================================================================
PokerCard::PokerCard()
    : flipProgress(0.0f)
    , selectionGlow(0.0f)
    , playingPulse(0.0f)
    , hoverGlow(0.0f)
    , revealFlash(0.0f)
{
    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);

    // Create child components
    starRating = std::make_unique<StarRating>();
    starRating->onRatingChanged = [this](int rating) {
        cardData.rating = rating;
        if (onRatingChanged)
            onRatingChanged(rating);
    };
    addChildComponent(starRating.get());

    guessDropdown = std::make_unique<GuessDropdown>();
    guessDropdown->onSelectionChanged = [this](int index) {
        cardData.guessedTrackIndex = index;
        if (onGuessChanged)
            onGuessChanged(index);
    };
    addChildComponent(guessDropdown.get());

    qaSelectButton = std::make_unique<juce::TextButton>("Select");
    qaSelectButton->onClick = [this]() {
        if (onQASelected)
            onQASelected();
    };
    addChildComponent(qaSelectButton.get());

    // Create animation timer
    animationTimer = std::make_unique<AnimationTimer>(*this);

    // Enable mouse tracking
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

PokerCard::~PokerCard()
{
    ThemeManager::getInstance().removeChangeListener(this);

    if (animationTimer)
        animationTimer->stopTimer();
}

//==============================================================================
void PokerCard::setCardData(const CardData& data)
{
    bool wasRevealed = cardData.isRevealed;
    cardData = data;

    // Update star rating
    if (starRating)
    {
        starRating->setRating(data.rating, false);
    }

    // Update guess dropdown
    if (guessDropdown)
    {
        guessDropdown->setSelectedIndex(data.guessedTrackIndex);
    }

    // Update selection glow
    selectionGlow.setTarget(data.isSelected ? 1.0f : 0.0f, kGlowDurationMs);

    // Trigger reveal flash if just revealed
    if (data.isRevealed && !wasRevealed)
    {
        triggerRevealFlash();
    }

    // Update playing state
    if (data.isPlaying && !animationTimer->isTimerRunning())
    {
        pulseStartTime = juce::Time::currentTimeMillis();
        startAnimationTimer();
    }

    updateChildComponentVisibility();
    repaint();
}

void PokerCard::setMode(blindcard::RatingMode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
        updateChildComponentVisibility();
        resized();
        repaint();
    }
}

void PokerCard::setPhase(blindcard::GamePhase phase)
{
    if (currentPhase != phase)
    {
        currentPhase = phase;

        // Set revealed state based on phase
        bool shouldBeRevealed = (phase != blindcard::GamePhase::BlindTesting);
        if (shouldBeRevealed != cardData.isRevealed)
        {
            cardData.isRevealed = shouldBeRevealed;
            // Animate the flip
            flipProgress.setTarget(shouldBeRevealed ? 1.0f : 0.0f, kFlipDurationMs, EasingType::EaseInOut);
            if (shouldBeRevealed)
            {
                triggerRevealFlash();
            }
            startAnimationTimer();
        }

        updateChildComponentVisibility();
        repaint();
    }
}

void PokerCard::flip()
{
    bool toFront = flipProgress.getTarget() < 0.5f;
    flipProgress.setTarget(toFront ? 1.0f : 0.0f, kFlipDurationMs, EasingType::EaseInOut);
    cardData.isRevealed = toFront;

    if (toFront)
    {
        triggerRevealFlash();
    }

    startAnimationTimer();
}

void PokerCard::setTracks(const std::vector<std::string>& tracks)
{
    if (guessDropdown)
    {
        guessDropdown->setTracks(tracks);
    }
}

void PokerCard::setSelected(bool selected)
{
    if (cardData.isSelected != selected)
    {
        cardData.isSelected = selected;
        selectionGlow.setTarget(selected ? 1.0f : 0.0f, kGlowDurationMs);
        startAnimationTimer();
    }
}

void PokerCard::setPlaying(bool playing)
{
    if (cardData.isPlaying != playing)
    {
        cardData.isPlaying = playing;
        if (playing)
        {
            pulseStartTime = juce::Time::currentTimeMillis();
        }
        startAnimationTimer();
        repaint();
    }
}

//==============================================================================
void PokerCard::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Calculate flip transform
    float flip = flipProgress.getValue();

    // Calculate visual rotation: 0->0.5 = 0->90 degrees, 0.5->1 = 90->0 degrees (but showing front)
    float rotationProgress;
    bool showFront;

    if (flip < 0.5f)
    {
        rotationProgress = flip * 2.0f;  // 0 to 1 for first half
        showFront = false;
    }
    else
    {
        rotationProgress = (1.0f - flip) * 2.0f;  // 1 to 0 for second half
        showFront = true;
    }

    // Calculate horizontal scale for 3D rotation effect
    // At 0 degrees: scale = 1, at 90 degrees: scale = 0
    float scaleX = 1.0f - rotationProgress;

    // Calculate vertical translation during flip
    float translateY = kFlipTranslateY * std::sin(flip * juce::MathConstants<float>::pi);

    // Apply transformations
    auto transformedBounds = bounds;
    transformedBounds = transformedBounds.translated(0, translateY);

    // Scale horizontally around center
    float centerX = transformedBounds.getCentreX();
    float newWidth = transformedBounds.getWidth() * scaleX;
    transformedBounds.setX(centerX - newWidth / 2.0f);
    transformedBounds.setWidth(newWidth);

    // Draw selection glow (behind card)
    float glow = selectionGlow.getValue();
    if (glow > 0.0f)
    {
        drawSelectionGlow(g, bounds, glow);
    }

    // Draw reveal flash (behind card)
    float flash = revealFlash.getValue();
    if (flash > 0.0f)
    {
        drawRevealFlash(g, bounds, flash);
    }

    // Only draw card if it has some width
    if (scaleX > 0.01f)
    {
        // Save graphics state
        juce::Graphics::ScopedSaveState saveState(g);

        // Draw card shadow
        auto shadowBounds = transformedBounds.translated(2, 4);
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(shadowBounds, static_cast<float>(kCornerRadius));

        // Draw card
        if (showFront || cardData.isRevealed)
        {
            drawCardFront(g, transformedBounds);
        }
        else
        {
            drawCardBack(g, transformedBounds);
        }
    }

    // Draw playing indicator (on top)
    if (cardData.isPlaying)
    {
        // Calculate pulse
        auto currentTime = juce::Time::currentTimeMillis();
        float pulseTime = static_cast<float>(currentTime - pulseStartTime) / kPulseDurationMs;
        float pulse = 0.5f + 0.5f * std::sin(pulseTime * juce::MathConstants<float>::twoPi);
        drawPlayingIndicator(g, bounds, pulse);
    }
}

void PokerCard::resized()
{
    auto bounds = getLocalBounds();

    updateChildComponentBounds();
}

//==============================================================================
void PokerCard::mouseEnter(const juce::MouseEvent&)
{
    isHovered = true;
    hoverGlow.setTarget(1.0f, 150.0f);
    startAnimationTimer();
}

void PokerCard::mouseExit(const juce::MouseEvent&)
{
    isHovered = false;
    hoverGlow.setTarget(0.0f, 150.0f);
    startAnimationTimer();
}

void PokerCard::mouseDown(const juce::MouseEvent& event)
{
    // Don't trigger click if clicking on child components
    if (event.originalComponent == this)
    {
        if (onClicked)
            onClicked();
    }
}

//==============================================================================
void PokerCard::updateAnimations()
{
    auto currentTime = juce::Time::currentTimeMillis();
    float deltaMs = static_cast<float>(currentTime - lastUpdateTime);
    lastUpdateTime = currentTime;

    // Clamp delta
    if (deltaMs > 100.0f || deltaMs < 0.0f)
        deltaMs = 16.0f;

    // Update all animated values
    flipProgress.update(deltaMs);
    selectionGlow.update(deltaMs);
    hoverGlow.update(deltaMs);
    revealFlash.update(deltaMs);

    // Handle reveal flash completion
    if (isFlashActive && revealFlash.isComplete())
    {
        isFlashActive = false;
    }

    repaint();
    stopAnimationTimerIfIdle();
}

void PokerCard::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        repaint();
    }
}

//==============================================================================
void PokerCard::drawCardBack(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Draw card background
    g.setColour(CardLayout::cardBackColor);
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));

    // Draw simple pattern (diagonal lines)
    g.setColour(CardLayout::cardBackPatternColor);
    float patternSpacing = 8.0f;

    juce::Path patternPath;
    auto clipBounds = bounds.reduced(2.0f);

    for (float x = clipBounds.getX() - clipBounds.getHeight(); x < clipBounds.getRight(); x += patternSpacing)
    {
        patternPath.startNewSubPath(x, clipBounds.getBottom());
        patternPath.lineTo(x + clipBounds.getHeight(), clipBounds.getY());
    }

    // Clip to rounded rectangle
    g.saveState();
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, static_cast<float>(kCornerRadius));
    g.reduceClipRegion(clipPath);
    g.strokePath(patternPath, juce::PathStrokeType(1.0f));
    g.restoreState();

    // Draw border
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, static_cast<float>(kCornerRadius), 1.0f);

    // Draw position label
    drawPositionLabel(g, bounds);
}

void PokerCard::drawPositionLabel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Gold position number in bottom-right corner
    g.setColour(CardLayout::positionLabelColor);
    g.setFont(juce::Font(CardLayout::positionLabelFontSize).boldened());

    juce::String positionText = juce::String(cardData.position + 1);

    auto labelBounds = bounds.removeFromBottom(30.0f).removeFromRight(30.0f);
    g.drawText(positionText, labelBounds, juce::Justification::centred);
}

//==============================================================================
void PokerCard::drawCardFront(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Draw card background (cream)
    g.setColour(kCardFrontColor);
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));

    // Draw border
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds, static_cast<float>(kCornerRadius), 1.0f);

    // Draw card content
    drawTopLeftCorner(g, bounds);
    drawCenterSuit(g, bounds);
    drawPluginName(g, bounds);
    drawBottomRightCorner(g, bounds);
}

void PokerCard::drawTopLeftCorner(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    Suit suit = cardData.getSuit();
    juce::Colour suitColor = getSuitColor(suit);
    juce::String suitSymbol = getSuitSymbol(suit);
    juce::String numberText = cardData.getDisplayNumber();

    float x = bounds.getX() + CardLayout::cornerPadding;
    float y = bounds.getY() + CardLayout::cornerPadding;

    // Draw number
    g.setColour(suitColor);
    g.setFont(juce::Font(CardLayout::cornerNumberFontSize).boldened());
    g.drawText(numberText, x, y, 30, 24, juce::Justification::centredLeft);

    // Draw suit symbol below number
    g.setFont(juce::Font(CardLayout::cornerSuitFontSize));
    g.drawText(suitSymbol, x, y + 22 + CardLayout::cornerSpacing, 30, 20, juce::Justification::centredLeft);
}

void PokerCard::drawBottomRightCorner(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    Suit suit = cardData.getSuit();
    juce::Colour suitColor = getSuitColor(suit);
    juce::String suitSymbol = getSuitSymbol(suit);
    juce::String numberText = cardData.getDisplayNumber();

    float x = bounds.getRight() - CardLayout::cornerPadding - 30;
    float y = bounds.getBottom() - CardLayout::cornerPadding - 44;

    // Draw suit symbol (above number, inverted position)
    g.setColour(suitColor);
    g.setFont(juce::Font(CardLayout::cornerSuitFontSize));
    g.drawText(suitSymbol, x, y, 30, 20, juce::Justification::centredRight);

    // Draw number below suit
    g.setFont(juce::Font(CardLayout::cornerNumberFontSize).boldened());
    g.drawText(numberText, x, y + 20 + CardLayout::cornerSpacing, 30, 24, juce::Justification::centredRight);
}

void PokerCard::drawCenterSuit(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    Suit suit = cardData.getSuit();
    juce::Colour suitColor = getSuitColor(suit);
    juce::String suitSymbol = getSuitSymbol(suit);

    // Draw large suit symbol in center (slightly above center)
    g.setColour(suitColor);
    g.setFont(juce::Font(CardLayout::centerSuitFontSize));

    auto centerBounds = bounds.withSizeKeepingCentre(60, 60);
    centerBounds = centerBounds.translated(0, -20);  // Move up a bit

    g.drawText(suitSymbol, centerBounds, juce::Justification::centred);
}

void PokerCard::drawPluginName(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (cardData.trackName.empty())
        return;

    // Draw plugin name in center-bottom area
    g.setColour(kBlackSuitColor.withAlpha(0.8f));
    g.setFont(juce::Font(CardLayout::pluginNameFontSize));

    auto nameBounds = bounds;
    nameBounds.removeFromTop(bounds.getHeight() * 0.55f);
    nameBounds.removeFromBottom(50.0f);
    nameBounds = nameBounds.reduced(10.0f, 0.0f);

    // Word wrap the plugin name
    juce::String name(cardData.trackName);
    g.drawFittedText(name, nameBounds.toNearestInt(),
                     juce::Justification::centred, 2, 0.9f);
}

//==============================================================================
void PokerCard::drawSelectionGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float glowAmount)
{
    auto glowColor = CardLayout::selectionGlowColor.withAlpha(0.6f * glowAmount);

    // Draw multiple glow layers
    for (int i = 4; i >= 0; --i)
    {
        float expansion = CardLayout::glowRadius * (static_cast<float>(i + 1) / 5.0f);
        float alpha = glowAmount * 0.15f / static_cast<float>(i + 1);
        g.setColour(glowColor.withAlpha(alpha));
        g.fillRoundedRectangle(bounds.expanded(expansion),
                               static_cast<float>(kCornerRadius) + expansion * 0.5f);
    }
}

void PokerCard::drawPlayingIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float pulseAmount)
{
    // Draw pulsing green dot in top-right corner
    float indicatorSize = CardLayout::playingIndicatorSize * (0.8f + 0.4f * pulseAmount);

    auto indicatorBounds = juce::Rectangle<float>(
        bounds.getRight() - indicatorSize - 8,
        bounds.getY() + 8,
        indicatorSize,
        indicatorSize
    );

    // Glow
    g.setColour(CardLayout::playingColor.withAlpha(0.3f * pulseAmount));
    g.fillEllipse(indicatorBounds.expanded(4));

    // Solid dot
    g.setColour(CardLayout::playingColor);
    g.fillEllipse(indicatorBounds);

    // Inner highlight
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.fillEllipse(indicatorBounds.reduced(indicatorSize * 0.3f).translated(-1, -1));
}

void PokerCard::drawRevealFlash(juce::Graphics& g, juce::Rectangle<float> bounds, float flashAmount)
{
    // Draw expanding gold flash
    auto flashColor = CardLayout::revealFlashColor.withAlpha(0.8f * flashAmount);

    float expansion = 20.0f * (1.0f - flashAmount);  // Flash shrinks as it fades

    g.setColour(flashColor);
    g.fillRoundedRectangle(bounds.expanded(expansion),
                           static_cast<float>(kCornerRadius) + expansion * 0.5f);
}

//==============================================================================
juce::String PokerCard::getSuitSymbol(Suit suit) const
{
    switch (suit)
    {
        case Suit::Spades:   return SuitSymbols::Spades;
        case Suit::Clubs:    return SuitSymbols::Clubs;
        case Suit::Diamonds: return SuitSymbols::Diamonds;
        case Suit::Hearts:   return SuitSymbols::Hearts;
        default:             return SuitSymbols::Spades;
    }
}

juce::Colour PokerCard::getSuitColor(Suit suit) const
{
    switch (suit)
    {
        case Suit::Spades:
        case Suit::Clubs:
            return kBlackSuitColor;

        case Suit::Diamonds:
        case Suit::Hearts:
            return kRedSuitColor;

        default:
            return kBlackSuitColor;
    }
}

//==============================================================================
void PokerCard::updateChildComponentVisibility()
{
    bool isBlindTesting = (currentPhase == blindcard::GamePhase::BlindTesting);

    // Stars mode: show star rating below card during blind testing
    bool showStars = isBlindTesting && (currentMode == blindcard::RatingMode::Stars);
    starRating->setVisible(showStars);
    starRating->setInteractive(showStars);

    // Guess mode: show dropdown above card during blind testing
    bool showGuess = isBlindTesting && (currentMode == blindcard::RatingMode::Guess);
    guessDropdown->setVisible(showGuess);
    guessDropdown->setEnabled(showGuess);

    // Q&A mode: show select button during blind testing
    bool showQA = isBlindTesting && (currentMode == blindcard::RatingMode::QA);
    qaSelectButton->setVisible(showQA);
    qaSelectButton->setEnabled(showQA);
}

void PokerCard::updateChildComponentBounds()
{
    auto bounds = getLocalBounds();

    // Star rating: below the card
    if (starRating)
    {
        auto starBounds = bounds;
        starBounds.removeFromTop(kDefaultHeight + CardLayout::starRatingOffsetY);
        starBounds = starBounds.withHeight(StarRating::kTotalHeight);
        starBounds = starBounds.withSizeKeepingCentre(StarRating::kTotalWidth, StarRating::kTotalHeight);
        starRating->setBounds(starBounds);
    }

    // Guess dropdown: above the card
    if (guessDropdown)
    {
        auto dropdownBounds = bounds;
        dropdownBounds = dropdownBounds.removeFromTop(CardLayout::guessDropdownHeight);
        dropdownBounds = dropdownBounds.withSizeKeepingCentre(CardLayout::guessDropdownWidth, CardLayout::guessDropdownHeight);
        guessDropdown->setBounds(dropdownBounds);
    }

    // Q&A button: below the card (similar to stars position)
    if (qaSelectButton)
    {
        auto buttonBounds = bounds;
        buttonBounds.removeFromTop(kDefaultHeight + CardLayout::starRatingOffsetY);
        buttonBounds = buttonBounds.withHeight(CardLayout::qaButtonHeight);
        buttonBounds = buttonBounds.withSizeKeepingCentre(CardLayout::qaButtonWidth, CardLayout::qaButtonHeight);
        qaSelectButton->setBounds(buttonBounds);
    }
}

//==============================================================================
void PokerCard::startAnimationTimer()
{
    if (animationTimer && !animationTimer->isTimerRunning())
    {
        lastUpdateTime = juce::Time::currentTimeMillis();
        animationTimer->startTimerHz(60); // 60 FPS
    }
}

void PokerCard::stopAnimationTimerIfIdle()
{
    bool allComplete = flipProgress.isComplete()
                    && selectionGlow.isComplete()
                    && hoverGlow.isComplete()
                    && revealFlash.isComplete()
                    && !cardData.isPlaying;

    if (allComplete && animationTimer)
    {
        animationTimer->stopTimer();
    }
}

void PokerCard::triggerRevealFlash()
{
    isFlashActive = true;
    revealFlash.setImmediate(1.0f);
    revealFlash.setTarget(0.0f, 300.0f, EasingType::EaseOut);
    startAnimationTimer();
}

} // namespace BlindCard
