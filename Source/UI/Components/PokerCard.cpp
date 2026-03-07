/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    PokerCard.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the PokerCard component with flip animation.

  ==============================================================================
*/

#include "PokerCard.h"
#include "../Theme/FontManager.h"
#include "../Localization/LocalizationManager.h"
#include <cmath>
#include "BinaryData.h"

namespace BlindCard
{

//==============================================================================
// Helper to check if current language needs CJK-compatible font
namespace
{
    bool isCJKLanguage()
    {
        auto lang = LocalizationManager::getInstance().getCurrentLanguage();
        return lang == Language::TraditionalChinese ||
               lang == Language::SimplifiedChinese ||
               lang == Language::Japanese ||
               lang == Language::Korean;
    }
}

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
    constexpr float pluginNameFontSize = 24.0f;  // Same as corner numbers for balance
    constexpr float positionLabelFontSize = 16.0f;

    // Margins and spacing
    constexpr float cornerPadding = 8.0f;
    constexpr float cornerSpacing = 2.0f;

    // Card back
    const juce::Colour cardBackColor { 0xFF1A3D5C };          // Dark blue
    const juce::Colour cardBackPatternColor { 0xFF2A5D8C };   // Lighter pattern
    const juce::Colour positionLabelColor { 0xFFFFD700 };     // Gold

    // Default card edge glow (subtle golden ambient glow on all cards)
    const juce::Colour cardEdgeGlowColor { 0xFFFFD700 };      // Gold
    constexpr float cardEdgeGlowAlpha = 0.15f;                // 15% opacity for subtle effect
    constexpr float cardEdgeGlowRadius = 8.0f;                // Glow radius in pixels

    // Border colors (matching original React design)
    const juce::Colour borderGoldMuted { 0xFFD4AF37 };        // #D4AF37 muted gold
    const juce::Colour borderGoldBright { 0xFFFFD700 };       // #FFD700 bright gold (selected)
    constexpr float borderDefaultAlpha = 0.30f;               // 30% for default state
    constexpr float borderWidth = 2.0f;                       // border-2

    // Selection glow (Original: gold-glow = box-shadow: 0 0 15px rgba(255,215,0,0.6), 0 0 30px rgba(255,215,0,0.3))
    const juce::Colour selectionGlowColor { 0xFFFFD700 };     // Gold
    constexpr float glowRadius15 = 15.0f;                     // First shadow radius
    constexpr float glowRadius30 = 30.0f;                     // Second shadow radius

    // Playing indicator - Cyan/teal for modern audio style
    const juce::Colour playingColor { 0xFF00D4AA };
    constexpr float playingIndicatorSize = 24.0f;             // Larger for volume icon

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

    qaSelectButton = std::make_unique<juce::TextButton>(LOCALIZE(CardSelect));
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

    // Update correct answer glow (Q&A mode)
    correctAnswerGlow.setTarget(data.isCorrectAnswer ? 1.0f : 0.0f, kGlowDurationMs);
    if (data.isCorrectAnswer)
        startAnimationTimer();

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

    // Draw correct answer glow (behind everything, Q&A mode)
    float correctGlow = correctAnswerGlow.getValue();
    if (correctGlow > 0.0f)
    {
        drawCorrectAnswerGlow(g, bounds, correctGlow);
    }

    // Draw selection glow (behind card, only during BlindTesting)
    float glow = selectionGlow.getValue();
    if (glow > 0.0f && currentPhase == blindcard::GamePhase::BlindTesting)
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

        // Draw playing glow (yellow glow around card when playing, only during BlindTesting)
        if (cardData.isPlaying && currentPhase == blindcard::GamePhase::BlindTesting)
        {
            auto currentTime = juce::Time::currentTimeMillis();
            float pulseTime = static_cast<float>(currentTime - pulseStartTime) / kPulseDurationMs;
            float pulse = 0.5f + 0.5f * std::sin(pulseTime * juce::MathConstants<float>::twoPi);
            // Use transformedBounds to match actual card position
            auto glowBounds = transformedBounds.withHeight(static_cast<float>(kDefaultHeight));
            drawPlayingGlow(g, glowBounds, pulse);
        }

        // Draw subtle golden edge glow (ambient glow on all cards, but not for empty standalone cards)
        bool isEmptyStandaloneCard = standaloneMode && !loadedAudioFile.existsAsFile();
        if (!isEmptyStandaloneCard)
        {
            for (int i = 4; i >= 1; --i)
            {
                float expansion = CardLayout::cardEdgeGlowRadius * (static_cast<float>(i) / 4.0f);
                float layerAlpha = CardLayout::cardEdgeGlowAlpha * (1.0f - static_cast<float>(i - 1) / 4.0f) * 0.5f;
                g.setColour(CardLayout::cardEdgeGlowColor.withAlpha(layerAlpha));
                g.fillRoundedRectangle(transformedBounds.expanded(expansion),
                                       static_cast<float>(kCornerRadius) + expansion * 0.3f);
            }
        }

        // Draw card shadow
        auto shadowBounds = transformedBounds.translated(2, 4);
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(shadowBounds, static_cast<float>(kCornerRadius));

        // Draw card based on state
        if (standaloneMode && !loadedAudioFile.existsAsFile() && !isDragHovering)
        {
            // Empty standalone card
            drawEmptyCardState(g, transformedBounds);
        }
        else if (showFront || cardData.isRevealed)
        {
            drawCardFront(g, transformedBounds);

            // Draw audio info and remove button for standalone mode
            if (standaloneMode && loadedAudioFile.existsAsFile())
            {
                drawLoadedAudioInfo(g, transformedBounds);
                drawRemoveButton(g, transformedBounds);
            }
        }
        else
        {
            drawCardBack(g, transformedBounds);
        }

        // Draw drag hover overlay on top
        if (isDragHovering)
        {
            drawDragHoverOverlay(g, transformedBounds);
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
        // Check for remove button click in standalone mode
        if (standaloneMode && loadedAudioFile.existsAsFile() &&
            removeButtonBounds.contains(event.getPosition()))
        {
            if (onRemoveFileClicked)
                onRemoveFileClicked();
            return;
        }

        if (onClicked)
            onClicked();
    }
}

void PokerCard::mouseDoubleClick(const juce::MouseEvent& event)
{
    // Only allow renaming in standalone mode, when card is face-up (Setup or Revealed)
    if (!standaloneMode)
        return;

    if (currentPhase != blindcard::GamePhase::Setup &&
        currentPhase != blindcard::GamePhase::Revealed)
        return;

    // Card must be revealed (face-up) to show the name
    if (!cardData.isRevealed && currentPhase == blindcard::GamePhase::BlindTesting)
        return;

    // Check if click is in the name area (center of card, excluding corners)
    auto bounds = getLocalBounds().toFloat();
    auto nameBounds = bounds;
    nameBounds.removeFromTop(55.0f);
    nameBounds.removeFromBottom(55.0f);
    nameBounds = nameBounds.reduced(15.0f, 0.0f);

    if (nameBounds.contains(event.position))
    {
        showNameEditor();
    }
}

void PokerCard::showNameEditor()
{
    if (nameEditor)
        return;  // Already showing

    nameEditor = std::make_unique<juce::TextEditor>();

    // Style the editor to match the card's plugin name appearance
    auto& fonts = FontManager::getInstance();
    nameEditor->setFont(fonts.getBold(CardLayout::pluginNameFontSize));
    nameEditor->setJustification(juce::Justification::centred);
    nameEditor->setMultiLine(false);
    nameEditor->setReturnKeyStartsNewLine(false);

    // Match card front colors
    nameEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFFFAFAF5));
    nameEditor->setColour(juce::TextEditor::textColourId, kBlackSuitColor);
    nameEditor->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFFD4AF37));  // Gold outline
    nameEditor->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFFD4AF37));

    // Set the current name and select all
    juce::String currentName(cardData.trackName);
    nameEditor->setText(currentName, false);
    nameEditor->selectAll();

    // Position in the name area
    auto bounds = getLocalBounds().toFloat();
    auto nameBounds = bounds;
    nameBounds.removeFromTop(55.0f);
    nameBounds.removeFromBottom(55.0f);
    nameBounds = nameBounds.reduced(15.0f, 0.0f);
    nameEditor->setBounds(nameBounds.toNearestInt());

    addAndMakeVisible(*nameEditor);
    nameEditor->grabKeyboardFocus();

    // Connect callbacks
    nameEditor->onReturnKey = [this]() { hideNameEditor(); };
    nameEditor->onFocusLost = [this]() { hideNameEditor(); };
}

void PokerCard::hideNameEditor()
{
    if (!nameEditor)
        return;

    juce::String newName = nameEditor->getText().trim();

    // Remove the editor first (prevents re-entry from focus lost)
    removeChildComponent(nameEditor.get());
    nameEditor.reset();

    // Update name if non-empty and different
    if (newName.isNotEmpty() && newName != juce::String(cardData.trackName))
    {
        cardData.trackName = newName.toStdString();
        if (onTrackNameChanged)
            onTrackNameChanged(newName);
    }

    repaint();
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
    correctAnswerGlow.update(deltaMs);

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
juce::Image& PokerCard::getCardBackImage()
{
    // Static image loaded once from binary data
    static juce::Image cardBackImage = juce::ImageCache::getFromMemory(
        BinaryData::cardback_png, BinaryData::cardback_pngSize);
    return cardBackImage;
}

//==============================================================================
void PokerCard::drawCardBack(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Get card back image
    auto& cardBackImg = getCardBackImage();

    if (cardBackImg.isValid())
    {
        // Clip to rounded rectangle
        g.saveState();
        juce::Path clipPath;
        clipPath.addRoundedRectangle(bounds, static_cast<float>(kCornerRadius));
        g.reduceClipRegion(clipPath);

        // Draw solid black background first (prevents table color bleeding through)
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));

        // Draw the card back image, scaled 10% larger to fill bounds completely
        // This creates a slightly zoomed-in effect, cropping edges but ensuring no black bars
        constexpr float imageScale = 1.10f;  // 110% = 放大 10%
        auto imageBounds = bounds.withSizeKeepingCentre(
            bounds.getWidth() * imageScale,
            bounds.getHeight() * imageScale);
        g.drawImage(cardBackImg, imageBounds,
                    juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);

        g.restoreState();
    }
    else
    {
        // Fallback: draw simple colored card back if image not loaded
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));
    }

    // Draw gold border - bright if selected, muted otherwise
    // Original: border-2 (2px), border-[#D4AF37]/30 default, border-[#FFD700] selected
    float borderAlpha = cardData.isSelected ? 1.0f : CardLayout::borderDefaultAlpha;
    juce::Colour borderColor = cardData.isSelected
        ? CardLayout::borderGoldBright
        : CardLayout::borderGoldMuted.withAlpha(borderAlpha);
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds, static_cast<float>(kCornerRadius), CardLayout::borderWidth);

    // Draw position label (gold number at bottom) and stars
    drawPositionLabel(g, bounds);
}

void PokerCard::drawPositionLabel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& fonts = FontManager::getInstance();

    // Gold position number in bottom-right corner
    g.setColour(CardLayout::positionLabelColor);
    g.setFont(fonts.getBebasNeue(CardLayout::positionLabelFontSize));

    juce::String positionText = juce::String(cardData.position + 1);

    auto labelBounds = bounds.removeFromBottom(30.0f).removeFromRight(30.0f);
    g.drawText(positionText, labelBounds, juce::Justification::centred);
}

//==============================================================================
void PokerCard::drawCardFront(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Original design: Cream gradient background
    // linear-gradient(145deg, #F5F5E8 0%, #E8E4D4 50%, #F5F5E8 100%)

    // Clip to rounded rectangle
    g.saveState();
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, static_cast<float>(kCornerRadius));
    g.reduceClipRegion(clipPath);

    // Draw outer gradient background
    juce::ColourGradient outerGradient(
        juce::Colour(0xFFF5F5E8),  // Light cream at top-left
        bounds.getX(), bounds.getY(),
        juce::Colour(0xFFE8E4D4),  // Slightly darker at bottom-right
        bounds.getRight(), bounds.getBottom(),
        false  // linear gradient
    );
    outerGradient.addColour(0.5, juce::Colour(0xFFE8E4D4));  // Mid transition
    g.setGradientFill(outerGradient);
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));

    // Draw inner lighter area (inset)
    auto innerBounds = bounds.reduced(4.0f);
    juce::ColourGradient innerGradient(
        juce::Colour(0xFFFAFAF5),  // from-[#FAFAF5]
        innerBounds.getX(), innerBounds.getY(),
        juce::Colour(0xFFF0EDE0),  // to-[#F0EDE0]
        innerBounds.getRight(), innerBounds.getBottom(),
        false
    );
    g.setGradientFill(innerGradient);
    g.fillRoundedRectangle(innerBounds, static_cast<float>(kCornerRadius) - 2.0f);

    g.restoreState();

    // Draw gold border - bright if selected, muted otherwise
    // Original: border-2 (2px), border-[#D4AF37]/50 default, border-[#FFD700] selected
    float borderAlpha = cardData.isSelected ? 1.0f : 0.5f;  // 50% for card front
    juce::Colour borderColor = cardData.isSelected
        ? CardLayout::borderGoldBright
        : CardLayout::borderGoldMuted.withAlpha(borderAlpha);
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds, static_cast<float>(kCornerRadius), CardLayout::borderWidth);

    // Draw card content (no center suit - keep clean for track name readability)
    drawTopLeftCorner(g, bounds);
    drawPluginName(g, bounds);
    drawBottomRightCorner(g, bounds);
}

void PokerCard::drawTopLeftCorner(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& fonts = FontManager::getInstance();

    Suit suit = cardData.getSuit();
    juce::Colour suitColor = getSuitColor(suit);
    juce::String suitSymbol = getSuitSymbol(suit);
    juce::String numberText = cardData.getDisplayNumber();

    float x = bounds.getX() + CardLayout::cornerPadding;
    float y = bounds.getY() + CardLayout::cornerPadding;

    // Draw number
    g.setColour(suitColor);
    g.setFont(fonts.getBold(CardLayout::cornerNumberFontSize));
    g.drawText(numberText, x, y, 30, 24, juce::Justification::centredLeft);

    // Draw suit symbol below number
    g.setFont(fonts.getRegular(CardLayout::cornerSuitFontSize));
    g.drawText(suitSymbol, x, y + 22 + CardLayout::cornerSpacing, 30, 20, juce::Justification::centredLeft);
}

void PokerCard::drawBottomRightCorner(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& fonts = FontManager::getInstance();

    Suit suit = cardData.getSuit();
    juce::Colour suitColor = getSuitColor(suit);
    juce::String suitSymbol = getSuitSymbol(suit);
    juce::String numberText = cardData.getDisplayNumber();

    float x = bounds.getRight() - CardLayout::cornerPadding - 30;
    float y = bounds.getBottom() - CardLayout::cornerPadding - 44;

    // Draw suit symbol (above number, inverted position)
    g.setColour(suitColor);
    g.setFont(fonts.getRegular(CardLayout::cornerSuitFontSize));
    g.drawText(suitSymbol, x, y, 30, 20, juce::Justification::centredRight);

    // Draw number below suit
    g.setFont(fonts.getBold(CardLayout::cornerNumberFontSize));
    g.drawText(numberText, x, y + 20 + CardLayout::cornerSpacing, 30, 24, juce::Justification::centredRight);
}

void PokerCard::drawCenterSuit(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& fonts = FontManager::getInstance();

    Suit suit = cardData.getSuit();
    juce::Colour suitColor = getSuitColor(suit);
    juce::String suitSymbol = getSuitSymbol(suit);

    // Draw large suit symbol in center (slightly above center)
    g.setColour(suitColor);
    g.setFont(fonts.getRegular(CardLayout::centerSuitFontSize));

    auto centerBounds = bounds.withSizeKeepingCentre(60, 60);
    centerBounds = centerBounds.translated(0, -20);  // Move up a bit

    g.drawText(suitSymbol, centerBounds, juce::Justification::centred);
}

void PokerCard::drawPluginName(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (cardData.trackName.empty())
        return;

    // Don't draw the name if the TextEditor is active (prevents double text)
    if (nameEditor)
        return;

    auto& fonts = FontManager::getInstance();

    // Draw plugin name in center area with bold font for more presence
    g.setColour(kBlackSuitColor.withAlpha(0.9f));
    g.setFont(fonts.getBold(CardLayout::pluginNameFontSize));

    // Center the name in the card (with margins for corners)
    auto nameBounds = bounds;
    nameBounds.removeFromTop(55.0f);     // Clear top-left corner
    nameBounds.removeFromBottom(55.0f);  // Clear bottom-right corner
    nameBounds = nameBounds.reduced(15.0f, 0.0f);

    // Word wrap the plugin name (up to 3 lines for longer names)
    juce::String name(cardData.trackName);
    g.drawFittedText(name, nameBounds.toNearestInt(),
                     juce::Justification::centred, 3, 0.85f);
}

//==============================================================================
void PokerCard::drawSelectionGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float glowAmount)
{
    // Original CSS: box-shadow: 0 0 15px rgba(255, 215, 0, 0.6), 0 0 30px rgba(255, 215, 0, 0.3)
    // Draw outer glow first (30px, 0.3 alpha)
    float outerAlpha = 0.3f * glowAmount;
    g.setColour(CardLayout::selectionGlowColor.withAlpha(outerAlpha));
    for (int i = 5; i >= 0; --i)
    {
        float expansion = CardLayout::glowRadius30 * (static_cast<float>(i + 1) / 6.0f);
        float layerAlpha = outerAlpha * (1.0f - static_cast<float>(i) / 6.0f) * 0.3f;
        g.setColour(CardLayout::selectionGlowColor.withAlpha(layerAlpha));
        g.fillRoundedRectangle(bounds.expanded(expansion),
                               static_cast<float>(kCornerRadius) + expansion * 0.3f);
    }

    // Draw inner glow (15px, 0.6 alpha)
    float innerAlpha = 0.6f * glowAmount;
    for (int i = 3; i >= 0; --i)
    {
        float expansion = CardLayout::glowRadius15 * (static_cast<float>(i + 1) / 4.0f);
        float layerAlpha = innerAlpha * (1.0f - static_cast<float>(i) / 4.0f) * 0.4f;
        g.setColour(CardLayout::selectionGlowColor.withAlpha(layerAlpha));
        g.fillRoundedRectangle(bounds.expanded(expansion),
                               static_cast<float>(kCornerRadius) + expansion * 0.3f);
    }
}

void PokerCard::drawPlayingGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float pulseAmount)
{
    // Golden glow effect matching original design
    juce::Colour glowColor { 0xFFFFD700 };  // Gold

    float pulse = 0.7f + 0.3f * pulseAmount;

    // Soft outer glow layers (like CSS box-shadow)
    for (int i = 6; i >= 1; --i)
    {
        float expansion = 4.0f * static_cast<float>(i);
        float layerAlpha = 0.15f * pulse / static_cast<float>(i);
        g.setColour(glowColor.withAlpha(layerAlpha));
        g.fillRoundedRectangle(bounds.expanded(expansion),
                               static_cast<float>(kCornerRadius) + expansion * 0.3f);
    }

    // Bright inner glow
    g.setColour(glowColor.withAlpha(0.4f * pulse));
    g.fillRoundedRectangle(bounds.expanded(6.0f), static_cast<float>(kCornerRadius) + 3.0f);

    // Crisp golden border
    g.setColour(glowColor.withAlpha(0.9f));
    g.drawRoundedRectangle(bounds, static_cast<float>(kCornerRadius), 3.0f);
}

void PokerCard::drawPlayingIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float pulseAmount)
{
    // Yellow checkmark badge in top-right corner (matching original design)
    auto cardBounds = bounds.withHeight(static_cast<float>(kDefaultHeight));

    // Badge size and position
    float badgeSize = 28.0f;
    float badgeX = cardBounds.getRight() - badgeSize / 2.0f - 4.0f;
    float badgeY = cardBounds.getY() - badgeSize / 2.0f + 4.0f;

    auto badgeBounds = juce::Rectangle<float>(badgeX, badgeY, badgeSize, badgeSize);

    // Subtle pulse effect
    float pulse = 0.9f + 0.1f * std::sin(pulseAmount * juce::MathConstants<float>::pi * 2.0f);

    // Badge shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillEllipse(badgeBounds.translated(1.0f, 2.0f));

    // Yellow badge background
    juce::Colour badgeColor { 0xFFFFD700 };  // Gold/Yellow
    g.setColour(badgeColor.withAlpha(pulse));
    g.fillEllipse(badgeBounds);

    // Badge border (slightly darker)
    g.setColour(badgeColor.darker(0.15f));
    g.drawEllipse(badgeBounds.reduced(0.5f), 1.5f);

    // Draw checkmark
    float checkCenterX = badgeBounds.getCentreX();
    float checkCenterY = badgeBounds.getCentreY();
    float checkScale = badgeSize * 0.35f;

    juce::Path checkmark;
    // Checkmark path (starting from left, going down to bottom, then up to right)
    checkmark.startNewSubPath(checkCenterX - checkScale * 0.6f, checkCenterY);
    checkmark.lineTo(checkCenterX - checkScale * 0.1f, checkCenterY + checkScale * 0.5f);
    checkmark.lineTo(checkCenterX + checkScale * 0.7f, checkCenterY - checkScale * 0.4f);

    // Draw checkmark with dark color for contrast
    g.setColour(juce::Colour(0xFF1A1A1A));  // Dark gray/black
    g.strokePath(checkmark, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
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

void PokerCard::drawCorrectAnswerGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float glowAmount)
{
    // Draw bright green glow for correct answer reveal in Q&A mode
    // This should be very prominent to clearly indicate the correct answer
    juce::Colour correctColor(0xFF00FF00);  // Bright green

    // Draw multiple layers for a rich glow effect
    for (int i = 4; i >= 1; --i)
    {
        float expansion = 20.0f * (static_cast<float>(i) / 4.0f) * glowAmount;
        float layerAlpha = 0.3f * glowAmount / static_cast<float>(i);
        g.setColour(correctColor.withAlpha(layerAlpha));
        g.fillRoundedRectangle(bounds.expanded(expansion),
                               static_cast<float>(kCornerRadius) + expansion * 0.3f);
    }

    // Draw inner bright border
    g.setColour(correctColor.withAlpha(0.8f * glowAmount));
    g.drawRoundedRectangle(bounds.reduced(1.0f), static_cast<float>(kCornerRadius), 3.0f);
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

    // Star rating: overlapping the bottom of the card (matching original design)
    if (starRating)
    {
        int starY = kDefaultHeight - StarRating::kTotalHeight / 2 - 8;  // Overlap with card bottom
        auto starBounds = juce::Rectangle<int>(
            (bounds.getWidth() - StarRating::kTotalWidth) / 2,
            starY,
            StarRating::kTotalWidth,
            StarRating::kTotalHeight
        );
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

    // Q&A button: below the card
    if (qaSelectButton)
    {
        auto buttonBounds = bounds;
        buttonBounds.removeFromTop(kDefaultHeight + 2);  // Position below card with small gap
        buttonBounds = buttonBounds.withHeight(CardLayout::qaButtonHeight);
        buttonBounds = buttonBounds.withSizeKeepingCentre(CardLayout::qaButtonWidth, CardLayout::qaButtonHeight);
        qaSelectButton->setBounds(buttonBounds);
    }

    // Name editor: positioned in the center name area of the card
    if (nameEditor)
    {
        auto cardBounds = bounds.toFloat();
        auto nameBounds = cardBounds;
        nameBounds.removeFromTop(55.0f);
        nameBounds.removeFromBottom(55.0f);
        nameBounds = nameBounds.reduced(15.0f, 0.0f);
        nameEditor->setBounds(nameBounds.toNearestInt());
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
                    && correctAnswerGlow.isComplete()
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

//==============================================================================
// Standalone mode implementation

void PokerCard::setStandaloneMode(bool enabled)
{
    if (standaloneMode != enabled)
    {
        standaloneMode = enabled;
        repaint();
    }
}

void PokerCard::setLoadedAudioFile(const juce::File& file, double durationSeconds)
{
    loadedAudioFile = file;
    audioDurationSeconds = durationSeconds;

    // Update track name to show filename
    if (file.existsAsFile())
    {
        cardData.trackName = file.getFileNameWithoutExtension().toStdString();
    }
    else
    {
        cardData.trackName.clear();
    }

    repaint();
}

//==============================================================================
// FileDragAndDropTarget implementation

bool PokerCard::isInterestedInFileDrag(const juce::StringArray& files)
{
    if (!standaloneMode)
        return false;

    // Check if any file is a valid audio format
    for (const auto& path : files)
    {
        if (isValidAudioFile(path))
            return true;
    }
    return false;
}

void PokerCard::fileDragEnter(const juce::StringArray& /*files*/, int /*x*/, int /*y*/)
{
    isDragHovering = true;
    repaint();
}

void PokerCard::fileDragExit(const juce::StringArray& /*files*/)
{
    isDragHovering = false;
    repaint();
}

void PokerCard::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    isDragHovering = false;

    // Find first valid audio file
    for (const auto& path : files)
    {
        if (isValidAudioFile(path))
        {
            juce::File file(path);
            if (onFileDropped)
                onFileDropped(file);
            break;
        }
    }

    repaint();
}

bool PokerCard::isValidAudioFile(const juce::String& path) const
{
    juce::File file(path);
    auto ext = file.getFileExtension().toLowerCase();
    return ext == ".wav" || ext == ".mp3" || ext == ".aiff" || ext == ".aif";
}

//==============================================================================
// Standalone mode drawing

void PokerCard::drawEmptyCardState(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();

    // Draw faded card back
    g.saveState();
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, static_cast<float>(kCornerRadius));
    g.reduceClipRegion(clipPath);

    // Darker background for empty state
    g.setColour(theme.isDark() ? juce::Colour(0xFF2A2A2A) : juce::Colour(0xFFE0E0E0));
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));

    g.restoreState();

    // Dashed border
    juce::Path borderPath;
    borderPath.addRoundedRectangle(bounds.reduced(1.0f), static_cast<float>(kCornerRadius));

    float dashLengths[] = { 8.0f, 4.0f };
    juce::PathStrokeType strokeType(2.0f);
    strokeType.createDashedStroke(borderPath, borderPath, dashLengths, 2);

    g.setColour(theme.isDark() ? juce::Colour(0xFF5A5A5A) : juce::Colour(0xFFAAAAAA));
    g.strokePath(borderPath, strokeType);

    // Position label
    drawPositionLabel(g, bounds);

    // "Drop audio file" text
    auto& fonts = FontManager::getInstance();
    g.setColour(theme.getColour(ColourId::TextPrimary).withAlpha(0.5f));

    // Use CJK-compatible font for non-English languages
    if (isCJKLanguage())
        g.setFont(fonts.getBold(16.0f));
    else
        g.setFont(fonts.getBebasNeue(18.0f));

    auto textBounds = bounds.reduced(10.0f);
    g.drawFittedText(LOCALIZE(CardDragFile), textBounds.toNearestInt(),
                     juce::Justification::centred, 2);
}

void PokerCard::drawDragHoverOverlay(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Semi-transparent blue overlay
    juce::Colour overlayColor(0xFF3B82F6);  // Blue

    g.setColour(overlayColor.withAlpha(0.3f));
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));

    // Blue border
    g.setColour(overlayColor);
    g.drawRoundedRectangle(bounds.reduced(1.0f), static_cast<float>(kCornerRadius), 2.0f);

    // "Drop Here" text with music note
    auto& fonts = FontManager::getInstance();
    g.setColour(juce::Colours::white);

    // Use CJK-compatible font for non-English languages
    if (isCJKLanguage())
        g.setFont(fonts.getBold(14.0f));
    else
        g.setFont(fonts.getBebasNeue(14.0f));

    auto textBounds = bounds.reduced(10.0f);
    g.drawFittedText(LOCALIZE(CardDropHere) + "\n\u266B", textBounds.toNearestInt(),
                     juce::Justification::centred, 2);
}

void PokerCard::drawLoadedAudioInfo(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Draw loaded file info on the card (below the track name area)
    if (!loadedAudioFile.existsAsFile())
        return;

    auto& fonts = FontManager::getInstance();

    // Format duration
    int totalSeconds = static_cast<int>(audioDurationSeconds);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    juce::String durationText = juce::String::formatted("[%d:%02d]", minutes, seconds);

    // Draw at bottom of card
    auto infoBounds = bounds;
    infoBounds.removeFromTop(bounds.getHeight() - 30.0f);
    infoBounds = infoBounds.reduced(8.0f, 4.0f);

    g.setColour(kBlackSuitColor.withAlpha(0.5f));
    g.setFont(fonts.getRegular(10.0f));
    g.drawText(durationText, infoBounds, juce::Justification::centred);
}

void PokerCard::drawRemoveButton(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (!standaloneMode || !loadedAudioFile.existsAsFile())
        return;

    // Small X button in bottom-right corner
    float buttonSize = 20.0f;
    auto buttonBounds = juce::Rectangle<float>(
        bounds.getRight() - buttonSize - 4.0f,
        bounds.getBottom() - buttonSize - 4.0f,
        buttonSize, buttonSize);

    // Store for hit detection
    removeButtonBounds = buttonBounds.toNearestInt();

    // Background circle
    g.setColour(juce::Colour(0xFFFF4444).withAlpha(0.8f));
    g.fillEllipse(buttonBounds);

    // X symbol
    auto& fonts = FontManager::getInstance();
    g.setColour(juce::Colours::white);
    g.setFont(fonts.getBebasNeue(12.0f));
    g.drawText(juce::String::fromUTF8("\u2715"), buttonBounds, juce::Justification::centred);
}

} // namespace BlindCard
