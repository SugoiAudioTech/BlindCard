/*
  ==============================================================================

    ChipButton.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the 3D poker chip button component.

  ==============================================================================
*/

#include "ChipButton.h"
#include <cmath>

namespace BlindCard
{

//==============================================================================
// Layout and design constants
namespace ChipLayout
{
    // Chip proportions (relative to chip size)
    constexpr float outerRingRatio = 1.0f;        // Full size
    constexpr float stripeOuterRatio = 0.95f;     // Stripes start at 95%
    constexpr float stripeInnerRatio = 0.75f;     // Stripes end at 75%
    constexpr float middleRingOuterRatio = 0.75f; // Middle ring outer at 75%
    constexpr float middleRingInnerRatio = 0.60f; // Middle ring inner at 60%
    constexpr float goldBorderRatio = 0.60f;      // Gold border at 60%
    constexpr float innerCircleRatio = 0.58f;     // Inner circle at 58%

    // Stripe parameters
    constexpr int numStripes = 12;
    constexpr float stripeWidthDegrees = 15.0f;   // Width of each stripe in degrees

    // Visual effects
    constexpr float shadowOffsetY = 4.0f;
    constexpr float shadowBlurRadius = 8.0f;
    constexpr float glowRadius = 12.0f;
    constexpr float hoverTranslateY = -2.0f;
    constexpr float pressedScale = 0.95f;

    // Border
    constexpr float goldBorderWidth = 1.5f;

    // Colors
    const juce::Colour stripeColor { 0xFFF5F5DC };  // Cream
    const juce::Colour middleRingColor { 0xFF000000 }; // Black
    const juce::Colour goldBorderColor { 0xFFFFD700 }; // Gold
    const juce::Colour iconColor { 0xFFFFFFFF };       // White

    // Typography
    constexpr float labelFontSize = 11.0f;
    constexpr float iconFontSize = 20.0f;

    // Disabled state
    constexpr float disabledOpacity = 0.4f;
}

// Default icons for each variant
namespace ChipIcons
{
    // Unicode icons
    const juce::String Shuffle = juce::String::charToString(0x21C4);  // Arrows left-right
    const juce::String Next    = juce::String::charToString(0x27A1);  // Right arrow
    const juce::String Reset   = juce::String::charToString(0x21BA);  // Counterclockwise arrow
    const juce::String Reveal  = juce::String::charToString(0x1F441); // Eye
}

//==============================================================================
ChipButton::ChipButton(const juce::String& labelText, ChipVariant chipVariant)
    : label(labelText)
    , variant(chipVariant)
    , hoverProgress(0.0f)
    , pressProgress(0.0f)
{
    // Set default icon based on variant
    switch (variant)
    {
        case ChipVariant::Red:   icon = ChipIcons::Shuffle; break;
        case ChipVariant::Gold:  icon = ChipIcons::Next; break;
        case ChipVariant::Black: icon = ChipIcons::Reset; break;
        case ChipVariant::Blue:  icon = ChipIcons::Reveal; break;
    }

    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);

    // Enable mouse tracking for hover effects
    setMouseCursor(juce::MouseCursor::PointingHandCursor);

    // Create animation timer
    animationTimer = std::make_unique<AnimationTimer>(*this);
}

ChipButton::~ChipButton()
{
    ThemeManager::getInstance().removeChangeListener(this);

    if (animationTimer)
        animationTimer->stopTimer();
}

//==============================================================================
void ChipButton::setEnabled(bool shouldBeEnabled)
{
    if (enabledState != shouldBeEnabled)
    {
        enabledState = shouldBeEnabled;

        // Update cursor
        setMouseCursor(shouldBeEnabled
            ? juce::MouseCursor::PointingHandCursor
            : juce::MouseCursor::NormalCursor);

        // Reset hover/press state when disabled
        if (!shouldBeEnabled)
        {
            isHovered = false;
            isPressed = false;
            hoverProgress.setTarget(0.0f, kAnimationDurationMs);
            pressProgress.setTarget(0.0f, kAnimationDurationMs);
            startAnimationTimer();
        }

        repaint();
    }
}

void ChipButton::setIcon(const juce::String& iconText)
{
    if (icon != iconText)
    {
        icon = iconText;
        repaint();
    }
}

void ChipButton::setLabel(const juce::String& newLabel)
{
    if (label != newLabel)
    {
        label = newLabel;
        repaint();
    }
}

void ChipButton::setVariant(ChipVariant newVariant)
{
    if (variant != newVariant)
    {
        variant = newVariant;
        repaint();
    }
}

//==============================================================================
void ChipButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Apply disabled opacity
    if (!enabledState)
    {
        g.setOpacity(ChipLayout::disabledOpacity);
    }

    // Calculate chip bounds (centered horizontally, at top)
    float chipSize = static_cast<float>(kChipSize);
    float chipX = (bounds.getWidth() - chipSize) / 2.0f;
    float chipY = 0.0f;
    auto chipBounds = juce::Rectangle<float>(chipX, chipY, chipSize, chipSize);

    // Apply hover/press transformations
    float hover = hoverProgress.getValue();
    float press = pressProgress.getValue();

    // Calculate translation (hover moves up)
    float translateY = ChipLayout::hoverTranslateY * hover;

    // Calculate scale (press shrinks)
    float scale = 1.0f - (1.0f - ChipLayout::pressedScale) * press;

    // Transform chip bounds
    auto transformedChipBounds = chipBounds.translated(0, translateY);
    auto center = transformedChipBounds.getCentre();

    // Apply scale around center
    float scaledSize = chipSize * scale;
    transformedChipBounds = juce::Rectangle<float>(
        center.x - scaledSize / 2.0f,
        center.y - scaledSize / 2.0f,
        scaledSize,
        scaledSize
    );

    // Calculate shadow amount (reduced when pressed)
    float shadowAmount = 1.0f - press * 0.5f;

    // Draw shadow (before other elements)
    drawChipShadow(g, transformedChipBounds, shadowAmount);

    // Draw glow (if hovering)
    if (hover > 0.0f)
    {
        drawChipGlow(g, transformedChipBounds, hover);
    }

    // Calculate center and radius for drawing
    auto chipCenter = transformedChipBounds.getCentre();
    float radius = transformedChipBounds.getWidth() / 2.0f;

    // Draw chip layers
    drawOuterRing(g, transformedChipBounds);
    drawCreamStripes(g, chipCenter, radius);
    drawMiddleRing(g, chipCenter, radius * ChipLayout::middleRingOuterRatio, radius * ChipLayout::middleRingInnerRatio);
    drawGoldBorder(g, chipCenter, radius * ChipLayout::goldBorderRatio);
    drawInnerCircle(g, chipCenter, radius * ChipLayout::innerCircleRatio);
    drawIcon(g, chipCenter, radius * ChipLayout::innerCircleRatio);

    // Draw label below chip
    auto labelBounds = bounds.removeFromBottom(static_cast<float>(kLabelHeight));
    drawLabel(g, labelBounds);
}

void ChipButton::resized()
{
    // No child components to layout
}

//==============================================================================
void ChipButton::mouseEnter(const juce::MouseEvent&)
{
    if (!enabledState)
        return;

    isHovered = true;
    hoverProgress.setTarget(1.0f, kAnimationDurationMs);
    startAnimationTimer();
}

void ChipButton::mouseExit(const juce::MouseEvent&)
{
    if (!enabledState)
        return;

    isHovered = false;
    isPressed = false;
    hoverProgress.setTarget(0.0f, kAnimationDurationMs);
    pressProgress.setTarget(0.0f, kAnimationDurationMs);
    startAnimationTimer();
}

void ChipButton::mouseDown(const juce::MouseEvent&)
{
    if (!enabledState)
        return;

    isPressed = true;
    pressProgress.setTarget(1.0f, kAnimationDurationMs * 0.5f); // Faster press animation
    startAnimationTimer();
}

void ChipButton::mouseUp(const juce::MouseEvent& event)
{
    if (!enabledState)
        return;

    bool wasPressed = isPressed;
    isPressed = false;
    pressProgress.setTarget(0.0f, kAnimationDurationMs);
    startAnimationTimer();

    // Trigger click if released within bounds
    if (wasPressed && getLocalBounds().contains(event.getPosition()))
    {
        if (onClick)
            onClick();
    }
}

//==============================================================================
void ChipButton::updateAnimations()
{
    auto currentTime = juce::Time::currentTimeMillis();
    float deltaMs = static_cast<float>(currentTime - lastUpdateTime);
    lastUpdateTime = currentTime;

    // Clamp delta to reasonable range (handle first frame)
    if (deltaMs > 100.0f || deltaMs < 0.0f)
        deltaMs = 16.0f;

    hoverProgress.update(deltaMs);
    pressProgress.update(deltaMs);

    repaint();

    stopAnimationTimerIfIdle();
}

void ChipButton::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        repaint();
    }
}

//==============================================================================
void ChipButton::drawChipShadow(juce::Graphics& g, juce::Rectangle<float> chipBounds, float shadowAmount)
{
    // Create shadow ellipse below chip
    float shadowOpacity = 0.4f * shadowAmount;
    float offsetY = ChipLayout::shadowOffsetY * shadowAmount;

    auto shadowBounds = chipBounds.translated(0, offsetY);

    // Draw multiple shadow layers for blur effect
    for (int i = 3; i >= 0; --i)
    {
        float expansion = static_cast<float>(i) * 2.0f;
        float alpha = shadowOpacity / static_cast<float>(i + 1);
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.fillEllipse(shadowBounds.expanded(expansion));
    }
}

void ChipButton::drawChipGlow(juce::Graphics& g, juce::Rectangle<float> chipBounds, float glowAmount)
{
    auto glowColor = getGlowColor().withAlpha(0.5f * glowAmount);

    // Draw multiple glow layers
    for (int i = 3; i >= 0; --i)
    {
        float expansion = ChipLayout::glowRadius * (static_cast<float>(i + 1) / 4.0f);
        float alpha = glowAmount * 0.2f / static_cast<float>(i + 1);
        g.setColour(glowColor.withAlpha(alpha));
        g.fillEllipse(chipBounds.expanded(expansion));
    }
}

void ChipButton::drawOuterRing(juce::Graphics& g, juce::Rectangle<float> chipBounds)
{
    auto center = chipBounds.getCentre();
    float radius = chipBounds.getWidth() / 2.0f;

    // Create radial gradient from base color to darker version
    auto baseColor = getBaseColor();
    auto darkColor = getDarkColor();

    juce::ColourGradient gradient(
        baseColor, center.x, center.y,
        darkColor, center.x + radius, center.y,
        true  // radial
    );

    g.setGradientFill(gradient);
    g.fillEllipse(chipBounds);
}

void ChipButton::drawCreamStripes(juce::Graphics& g, juce::Point<float> center, float radius)
{
    float stripeOuterRadius = radius * ChipLayout::stripeOuterRatio;
    float stripeInnerRadius = radius * ChipLayout::stripeInnerRatio;

    g.setColour(ChipLayout::stripeColor);

    // Draw 12 stripes evenly distributed
    float degreesPerStripe = 360.0f / static_cast<float>(ChipLayout::numStripes);
    float halfStripeWidth = ChipLayout::stripeWidthDegrees / 2.0f;

    for (int i = 0; i < ChipLayout::numStripes; ++i)
    {
        float centerAngle = static_cast<float>(i) * degreesPerStripe;

        // Create a path for the stripe (pie slice)
        juce::Path stripePath;

        // Convert to radians
        float startAngleRad = juce::degreesToRadians(centerAngle - halfStripeWidth - 90.0f);
        float endAngleRad = juce::degreesToRadians(centerAngle + halfStripeWidth - 90.0f);

        // Add outer arc
        stripePath.addArc(
            center.x - stripeOuterRadius,
            center.y - stripeOuterRadius,
            stripeOuterRadius * 2.0f,
            stripeOuterRadius * 2.0f,
            startAngleRad,
            endAngleRad,
            true
        );

        // Add inner arc (reversed direction)
        stripePath.addArc(
            center.x - stripeInnerRadius,
            center.y - stripeInnerRadius,
            stripeInnerRadius * 2.0f,
            stripeInnerRadius * 2.0f,
            endAngleRad,
            startAngleRad,
            false
        );

        stripePath.closeSubPath();

        g.fillPath(stripePath);
    }
}

void ChipButton::drawMiddleRing(juce::Graphics& g, juce::Point<float> center, float outerRadius, float innerRadius)
{
    // Draw black ring between outer edge decorations and inner circle
    g.setColour(ChipLayout::middleRingColor);

    juce::Path ringPath;
    ringPath.addEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2.0f, outerRadius * 2.0f);
    ringPath.addEllipse(center.x - innerRadius, center.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
    ringPath.setUsingNonZeroWinding(false);

    g.fillPath(ringPath);
}

void ChipButton::drawGoldBorder(juce::Graphics& g, juce::Point<float> center, float radius)
{
    g.setColour(ChipLayout::goldBorderColor);
    g.drawEllipse(
        center.x - radius,
        center.y - radius,
        radius * 2.0f,
        radius * 2.0f,
        ChipLayout::goldBorderWidth
    );
}

void ChipButton::drawInnerCircle(juce::Graphics& g, juce::Point<float> center, float radius)
{
    // Inner circle uses darker version of base color
    auto darkColor = getDarkColor();

    // Create subtle gradient for 3D effect
    juce::ColourGradient gradient(
        darkColor.brighter(0.1f), center.x - radius * 0.3f, center.y - radius * 0.3f,
        darkColor.darker(0.1f), center.x + radius * 0.5f, center.y + radius * 0.5f,
        true
    );

    g.setGradientFill(gradient);
    g.fillEllipse(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);
}

void ChipButton::drawIcon(juce::Graphics& g, juce::Point<float> center, float radius)
{
    g.setColour(ChipLayout::iconColor);
    g.setFont(juce::Font(ChipLayout::iconFontSize));

    auto iconBounds = juce::Rectangle<float>(
        center.x - radius,
        center.y - radius,
        radius * 2.0f,
        radius * 2.0f
    );

    g.drawText(icon, iconBounds, juce::Justification::centred);
}

void ChipButton::drawLabel(juce::Graphics& g, juce::Rectangle<float> labelBounds)
{
    auto& theme = ThemeManager::getInstance();

    g.setColour(theme.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(ChipLayout::labelFontSize).boldened());
    g.drawText(label, labelBounds, juce::Justification::centredTop);
}

//==============================================================================
juce::Colour ChipButton::getBaseColor() const
{
    switch (variant)
    {
        case ChipVariant::Red:   return juce::Colour(0xFFCC0000);
        case ChipVariant::Gold:  return juce::Colour(0xFFDAA520);
        case ChipVariant::Black: return juce::Colour(0xFF333333);
        case ChipVariant::Blue:  return juce::Colour(0xFF2E6EB0);
        default:                 return juce::Colour(0xFFCC0000);
    }
}

juce::Colour ChipButton::getDarkColor() const
{
    switch (variant)
    {
        case ChipVariant::Red:   return juce::Colour(0xFF8B0000);
        case ChipVariant::Gold:  return juce::Colour(0xFF8B7300);
        case ChipVariant::Black: return juce::Colour(0xFF1A1A1A);
        case ChipVariant::Blue:  return juce::Colour(0xFF1A4A7A);
        default:                 return juce::Colour(0xFF8B0000);
    }
}

juce::Colour ChipButton::getGlowColor() const
{
    switch (variant)
    {
        case ChipVariant::Red:   return juce::Colour(0xFFFF3B4E);  // Bright red
        case ChipVariant::Gold:  return juce::Colour(0xFFFFD700);  // Gold
        case ChipVariant::Black: return juce::Colour(0xFF888888);  // Gray
        case ChipVariant::Blue:  return juce::Colour(0xFF4A9EFF);  // Bright blue
        default:                 return juce::Colour(0xFFFF3B4E);
    }
}

//==============================================================================
void ChipButton::startAnimationTimer()
{
    if (animationTimer && !animationTimer->isTimerRunning())
    {
        lastUpdateTime = juce::Time::currentTimeMillis();
        animationTimer->startTimerHz(60); // 60 FPS
    }
}

void ChipButton::stopAnimationTimerIfIdle()
{
    if (hoverProgress.isComplete() && pressProgress.isComplete())
    {
        if (animationTimer)
            animationTimer->stopTimer();
    }
}

} // namespace BlindCard
