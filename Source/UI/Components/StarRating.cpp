/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    StarRating.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the 5-star rating component with animations.

  ==============================================================================
*/

#include "StarRating.h"
#include <cmath>

namespace BlindCard
{

//==============================================================================
// Star Unicode characters
namespace StarSymbols
{
    // Empty star: White Star U+2606
    const juce::String Empty = juce::String::charToString(0x2606);
    // Filled star: Black Star U+2605
    const juce::String Filled = juce::String::charToString(0x2605);
}

//==============================================================================
StarRating::StarRating()
{
    // Initialize all fill progress to 0 (empty)
    for (int i = 0; i < kMaxStars; ++i)
    {
        fillProgress[i].setImmediate(0.0f);
        scaleProgress[i].setImmediate(1.0f);
    }

    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);

    // Enable mouse tracking for hover effects
    setMouseCursor(juce::MouseCursor::PointingHandCursor);

    // Create animation timer
    animationTimer = std::make_unique<AnimationTimer>(*this);
}

StarRating::~StarRating()
{
    ThemeManager::getInstance().removeChangeListener(this);

    if (animationTimer)
        animationTimer->stopTimer();
}

//==============================================================================
void StarRating::setRating(int rating, bool animate)
{
    // Clamp rating to valid range
    rating = juce::jlimit(0, kStarCount, rating);

    if (currentRating == rating)
        return;

    int previousRating = currentRating;
    currentRating = rating;

    if (animate)
    {
        triggerFillAnimation(rating);
    }
    else
    {
        // Set immediately without animation
        for (int i = 0; i < kMaxStars; ++i)
        {
            float targetFill = (i < rating) ? 1.0f : 0.0f;
            fillProgress[i].setImmediate(targetFill);
        }
        repaint();
    }
}

void StarRating::setInteractive(bool interactive)
{
    if (interactiveState != interactive)
    {
        interactiveState = interactive;

        // Update cursor
        setMouseCursor(interactive
            ? juce::MouseCursor::PointingHandCursor
            : juce::MouseCursor::NormalCursor);

        // Clear hover state when disabled
        if (!interactive)
        {
            hoverRating = 0;
            isHovered = false;
        }

        repaint();
    }
}

//==============================================================================
void StarRating::paint(juce::Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    auto localBounds = getLocalBounds().toFloat();

    // Draw dark pill-shaped background (matching original design)
    float bgPadding = 8.0f;
    float totalStarsWidth = static_cast<float>(kTotalWidth);
    float bgWidth = totalStarsWidth + bgPadding * 2.0f;
    float bgHeight = static_cast<float>(kStarSize) + bgPadding;
    float bgX = (localBounds.getWidth() - bgWidth) / 2.0f;
    float bgY = (localBounds.getHeight() - bgHeight) / 2.0f;

    auto bgBounds = juce::Rectangle<float>(bgX, bgY, bgWidth, bgHeight);

    // Dark background with rounded corners (pill shape)
    g.setColour(juce::Colour(0xFF1A1A1A).withAlpha(0.9f));
    g.fillRoundedRectangle(bgBounds, bgHeight / 2.0f);

    // Subtle gold border
    g.setColour(juce::Colour(0xFFD4AF37).withAlpha(0.4f));
    g.drawRoundedRectangle(bgBounds, bgHeight / 2.0f, 1.0f);

    // Determine which rating to display (hover preview or actual)
    int displayRating = (isHovered && interactiveState && hoverRating > 0)
        ? hoverRating
        : currentRating;

    // Draw each star
    for (int i = 0; i < kStarCount; ++i)
    {
        auto bounds = getStarBounds(i);
        bool shouldBeFilled = (i < displayRating);

        // Get animation values
        float fill = fillProgress[i].getValue();
        float scale = scaleProgress[i].getValue();

        // If hovering, use immediate fill state for preview
        if (isHovered && interactiveState)
        {
            fill = shouldBeFilled ? 1.0f : 0.0f;
        }

        drawStar(g, i, bounds, shouldBeFilled, fill, scale);
    }
}

void StarRating::resized()
{
    // No child components to layout
}

//==============================================================================
void StarRating::mouseEnter(const juce::MouseEvent& event)
{
    if (!interactiveState)
        return;

    isHovered = true;
    hoverRating = getStarIndexAtPosition(event.x);
    if (hoverRating >= 0)
        hoverRating = ratingFromStarIndex(hoverRating);
    else
        hoverRating = 0;

    repaint();
}

void StarRating::mouseExit(const juce::MouseEvent&)
{
    if (!interactiveState)
        return;

    isHovered = false;
    hoverRating = 0;
    repaint();
}

void StarRating::mouseMove(const juce::MouseEvent& event)
{
    if (!interactiveState)
        return;

    int starIndex = getStarIndexAtPosition(event.x);
    int newHoverRating = (starIndex >= 0) ? ratingFromStarIndex(starIndex) : 0;

    if (newHoverRating != hoverRating)
    {
        hoverRating = newHoverRating;
        repaint();
    }
}

void StarRating::mouseDown(const juce::MouseEvent& event)
{
    if (!interactiveState)
        return;

    int starIndex = getStarIndexAtPosition(event.x);
    if (starIndex < 0)
        return;

    int newRating = ratingFromStarIndex(starIndex);

    // Trigger click animation on the clicked star
    triggerClickAnimation(starIndex);

    // Update rating if changed
    if (newRating != currentRating)
    {
        int previousRating = currentRating;
        currentRating = newRating;

        // Trigger fill animation
        triggerFillAnimation(newRating);

        // Notify listener
        if (onRatingChanged)
            onRatingChanged(newRating);
    }
}

//==============================================================================
void StarRating::updateAnimations()
{
    auto currentTime = juce::Time::currentTimeMillis();
    float deltaMs = static_cast<float>(currentTime - lastUpdateTime);
    lastUpdateTime = currentTime;

    // Clamp delta to reasonable range (handle first frame)
    if (deltaMs > 100.0f || deltaMs < 0.0f)
        deltaMs = 16.0f;

    bool needsRepaint = false;

    // Update fill animations with staggered timing
    if (pendingFillRating >= 0)
    {
        float elapsedSinceFillStart = static_cast<float>(currentTime - fillAnimationStartTime);

        for (int i = 0; i < kMaxStars; ++i)
        {
            float starDelay = static_cast<float>(i) * kFillDelayMs;
            float targetFill = (i < pendingFillRating) ? 1.0f : 0.0f;

            // Only start animation after delay
            if (elapsedSinceFillStart >= starDelay)
            {
                if (std::abs(fillProgress[i].getTarget() - targetFill) > 0.001f)
                {
                    fillProgress[i].setTarget(targetFill, kFillAnimationDurationMs, EasingType::EaseOut);
                }
            }
        }

        // Check if all fill animations are complete
        float totalDelay = static_cast<float>(kMaxStars - 1) * kFillDelayMs + kFillAnimationDurationMs;
        if (elapsedSinceFillStart > totalDelay)
        {
            pendingFillRating = -1;
        }
    }

    // Update click scale animation
    if (clickedStarIndex >= 0)
    {
        float elapsedSinceClick = static_cast<float>(currentTime - clickAnimationStartTime);
        float halfDuration = kClickAnimationDurationMs / 2.0f;

        if (elapsedSinceClick < halfDuration)
        {
            // Scale up phase
            float progress = elapsedSinceClick / halfDuration;
            float easedProgress = 1.0f - std::pow(1.0f - progress, 2.0f); // Ease out
            float scale = 1.0f + (kClickScaleMax - 1.0f) * easedProgress;
            scaleProgress[clickedStarIndex].setImmediate(scale);
        }
        else if (elapsedSinceClick < kClickAnimationDurationMs)
        {
            // Scale down phase
            float progress = (elapsedSinceClick - halfDuration) / halfDuration;
            float easedProgress = 1.0f - std::pow(1.0f - progress, 2.0f); // Ease out
            float scale = kClickScaleMax - (kClickScaleMax - 1.0f) * easedProgress;
            scaleProgress[clickedStarIndex].setImmediate(scale);
        }
        else
        {
            // Animation complete
            scaleProgress[clickedStarIndex].setImmediate(1.0f);
            clickedStarIndex = -1;
        }
        needsRepaint = true;
    }

    // Update all animated values
    for (int i = 0; i < kMaxStars; ++i)
    {
        if (!fillProgress[i].isComplete())
        {
            fillProgress[i].update(deltaMs);
            needsRepaint = true;
        }
    }

    if (needsRepaint)
        repaint();

    stopAnimationTimerIfIdle();
}

void StarRating::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        repaint();
    }
}

//==============================================================================
void StarRating::drawStar(juce::Graphics& g, int index, juce::Rectangle<float> bounds,
                          bool filled, float fillAmount, float scale)
{
    auto& theme = ThemeManager::getInstance();

    // Get colors from theme
    auto emptyColor = theme.getColour(ColourId::StarEmpty);
    auto filledColor = theme.getColour(ColourId::StarFilled);

    // Apply scale transformation
    if (std::abs(scale - 1.0f) > 0.001f)
    {
        auto center = bounds.getCentre();
        float scaledWidth = bounds.getWidth() * scale;
        float scaledHeight = bounds.getHeight() * scale;
        bounds = juce::Rectangle<float>(
            center.x - scaledWidth / 2.0f,
            center.y - scaledHeight / 2.0f,
            scaledWidth,
            scaledHeight
        );
    }

    // Calculate font size based on bounds
    float fontSize = bounds.getHeight() * 0.9f;

    // Determine which symbol and color to use based on fill amount
    juce::String symbol;
    juce::Colour color;

    if (fillAmount >= 0.5f)
    {
        // Filled star
        symbol = StarSymbols::Filled;
        color = filledColor;

        // Apply fade-in alpha for animation
        if (fillAmount < 1.0f)
        {
            float alpha = fillAmount * 2.0f - 1.0f; // 0.5->1.0 maps to 0->1
            color = color.withAlpha(alpha);
        }
    }
    else
    {
        // Empty star with potential fade-out of filled version
        if (fillAmount > 0.0f)
        {
            // Draw fading filled star first
            float alpha = fillAmount * 2.0f; // 0->0.5 maps to 0->1
            g.setColour(filledColor.withAlpha(alpha));
            g.setFont(juce::Font(fontSize));
            g.drawText(StarSymbols::Filled, bounds, juce::Justification::centred);
        }

        symbol = StarSymbols::Empty;
        color = emptyColor;
    }

    // Draw the star
    g.setColour(color);
    g.setFont(juce::Font(fontSize));
    g.drawText(symbol, bounds, juce::Justification::centred);
}

int StarRating::getStarIndexAtPosition(int x) const
{
    for (int i = 0; i < kStarCount; ++i)
    {
        auto bounds = getStarBounds(i);
        // Expand hit area slightly for better usability
        auto hitBounds = bounds.expanded(2.0f, 0.0f);
        if (hitBounds.contains(static_cast<float>(x), bounds.getCentreY()))
        {
            return i;
        }
    }
    return -1;
}

juce::Rectangle<float> StarRating::getStarBounds(int index) const
{
    auto localBounds = getLocalBounds().toFloat();

    // Calculate total width of all stars
    float totalWidth = static_cast<float>(kStarCount * kStarSize + (kStarCount - 1) * kStarSpacing);

    // Center stars horizontally
    float startX = (localBounds.getWidth() - totalWidth) / 2.0f;

    // Calculate this star's position
    float x = startX + static_cast<float>(index) * (kStarSize + kStarSpacing);
    float y = (localBounds.getHeight() - kStarSize) / 2.0f;

    return juce::Rectangle<float>(x, y, static_cast<float>(kStarSize), static_cast<float>(kStarSize));
}

//==============================================================================
void StarRating::startAnimationTimer()
{
    if (animationTimer && !animationTimer->isTimerRunning())
    {
        lastUpdateTime = juce::Time::currentTimeMillis();
        animationTimer->startTimerHz(60); // 60 FPS
    }
}

void StarRating::stopAnimationTimerIfIdle()
{
    bool allComplete = true;

    // Check fill animations
    for (int i = 0; i < kMaxStars; ++i)
    {
        if (!fillProgress[i].isComplete())
        {
            allComplete = false;
            break;
        }
    }

    // Check click animation
    if (clickedStarIndex >= 0)
        allComplete = false;

    // Check pending fill animation
    if (pendingFillRating >= 0)
        allComplete = false;

    if (allComplete && animationTimer)
    {
        animationTimer->stopTimer();
    }
}

void StarRating::triggerFillAnimation(int targetRating)
{
    pendingFillRating = targetRating;
    fillAnimationStartTime = juce::Time::currentTimeMillis();
    startAnimationTimer();
}

void StarRating::triggerClickAnimation(int starIndex)
{
    if (starIndex >= 0 && starIndex < kMaxStars)
    {
        clickedStarIndex = starIndex;
        clickAnimationStartTime = juce::Time::currentTimeMillis();
        scaleProgress[starIndex].setImmediate(1.0f);
        startAnimationTimer();
    }
}

} // namespace BlindCard
