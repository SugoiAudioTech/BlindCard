/*
  ==============================================================================

    ChipStack.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "ChipStack.h"

namespace BlindCard
{

//==============================================================================
ChipStack::ChipStack(int chipCount, bool startWithRed)
    : numChips(juce::jlimit(1, 10, chipCount))
    , startRed(startWithRed)
{
    ThemeManager::getInstance().addChangeListener(this);
}

ChipStack::~ChipStack()
{
    ThemeManager::getInstance().removeChangeListener(this);
}

//==============================================================================
void ChipStack::setChipCount(int count)
{
    numChips = juce::jlimit(1, 10, count);
    repaint();
}

void ChipStack::setStartsWithRed(bool redFirst)
{
    if (startRed != redFirst)
    {
        startRed = redFirst;
        repaint();
    }
}

//==============================================================================
void ChipStack::changeListenerCallback(juce::ChangeBroadcaster*)
{
    repaint();
}

//==============================================================================
void ChipStack::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Calculate the stacking arrangement
    // Chips are stacked bottom to top with slight overlap
    // Each chip shows its edge (thickness) and top surface

    float totalHeight = bounds.getHeight();
    float chipDiameter = juce::jmin(bounds.getWidth() * 0.9f, static_cast<float>(kChipDiameter));
    float thickness = kChipThickness;

    // Calculate the vertical spacing for the stack
    // Each chip overlaps about 60% with the one below
    float overlapRatio = 0.6f;
    float visibleHeight = thickness * (1.0f - overlapRatio);
    float stackHeight = thickness + (numChips - 1) * visibleHeight;

    // Center the stack horizontally and vertically
    float startY = bounds.getCentreY() + stackHeight / 2.0f - thickness;
    float centerX = bounds.getCentreX();

    // Draw chips from bottom to top (for correct overlap)
    for (int i = 0; i < numChips; ++i)
    {
        bool isRed = startRed ? (i % 2 == 0) : (i % 2 != 0);

        // Slight perspective: bottom chips are a tiny bit larger
        float perspectiveScale = 1.0f - (i * 0.02f);
        float scaledDiameter = chipDiameter * perspectiveScale;

        // Position for this chip (bottom chip is at i=0)
        float chipY = startY - (i * visibleHeight);

        juce::Rectangle<float> chipBounds(
            centerX - scaledDiameter / 2.0f,
            chipY,
            scaledDiameter,
            thickness
        );

        // Draw the edge (3D side) first
        drawChipEdge(g, chipBounds, isRed, perspectiveScale);

        // Draw the top surface
        drawChip(g, chipBounds, isRed, perspectiveScale);
    }
}

void ChipStack::drawChipEdge(juce::Graphics& g, juce::Rectangle<float> bounds, bool isRed, float scale)
{
    auto baseColor = getDarkChipColor(isRed);

    // Draw the 3D edge as a slightly darker ellipse below the main chip
    auto edgeBounds = bounds.translated(0.0f, 2.0f);

    g.setColour(baseColor.darker(0.3f));
    g.fillEllipse(edgeBounds.withHeight(edgeBounds.getWidth() * 0.3f)
                            .withY(edgeBounds.getY() + edgeBounds.getHeight() * 0.7f));
}

void ChipStack::drawChip(juce::Graphics& g, juce::Rectangle<float> bounds, bool isRed, float scale)
{
    auto& tm = ThemeManager::getInstance();
    auto baseColor = getChipColor(isRed);
    auto darkColor = getDarkChipColor(isRed);

    // The chip is drawn as an ellipse (top-down view at an angle)
    // Aspect ratio ~0.3 gives a nice 3D perspective
    float aspectRatio = 0.3f;
    float width = bounds.getWidth();
    float height = width * aspectRatio;

    auto chipBounds = juce::Rectangle<float>(
        bounds.getX(),
        bounds.getY(),
        width,
        height
    );

    // 1. Outer ring - base color with gradient
    {
        juce::ColourGradient gradient(
            baseColor.brighter(0.1f), chipBounds.getCentreX(), chipBounds.getY(),
            darkColor, chipBounds.getCentreX(), chipBounds.getBottom(),
            false
        );
        g.setGradientFill(gradient);
        g.fillEllipse(chipBounds);
    }

    // 2. Cream stripes around the edge
    {
        juce::Colour stripeColor(0xFFF5F5DC); // Cream
        g.setColour(stripeColor.withAlpha(0.8f));

        float stripeWidth = width * 0.08f;
        float stripeInset = width * 0.05f;

        // Draw stripes at regular intervals around the ellipse
        for (int i = 0; i < kStripeCount; ++i)
        {
            float angle = (i / static_cast<float>(kStripeCount)) * juce::MathConstants<float>::twoPi;

            // Position on the ellipse edge
            float rx = (width / 2.0f) - stripeInset;
            float ry = (height / 2.0f) - stripeInset * aspectRatio;

            float stripeX = chipBounds.getCentreX() + rx * std::cos(angle);
            float stripeY = chipBounds.getCentreY() + ry * std::sin(angle);

            // Draw small stripe marker
            float stripeHeight = height * 0.15f;
            g.fillEllipse(stripeX - stripeWidth / 2.0f,
                         stripeY - stripeHeight / 2.0f,
                         stripeWidth,
                         stripeHeight);
        }
    }

    // 3. Gold border ring
    {
        g.setColour(tm.getColour(ColourId::Accent)); // Gold
        auto innerBounds = chipBounds.reduced(width * 0.15f, height * 0.15f);
        g.drawEllipse(innerBounds, 1.5f * scale);
    }

    // 4. Inner circle - darker shade
    {
        auto innerBounds = chipBounds.reduced(width * 0.2f, height * 0.2f);

        juce::ColourGradient gradient(
            darkColor.brighter(0.05f), innerBounds.getCentreX(), innerBounds.getY(),
            darkColor.darker(0.2f), innerBounds.getCentreX(), innerBounds.getBottom(),
            false
        );
        g.setGradientFill(gradient);
        g.fillEllipse(innerBounds);
    }

    // 5. Subtle highlight on top
    {
        auto highlightBounds = chipBounds.reduced(width * 0.25f, height * 0.3f)
                                         .translated(0.0f, -height * 0.1f);
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.fillEllipse(highlightBounds);
    }
}

void ChipStack::resized()
{
    // Nothing to do - all drawing is relative to bounds
}

//==============================================================================
juce::Colour ChipStack::getChipColor(bool isRed) const
{
    auto& tm = ThemeManager::getInstance();

    if (isRed)
    {
        // Red chip
        return tm.isDark() ? juce::Colour(0xFFCC0000) : juce::Colour(0xFFB91C1C);
    }
    else
    {
        // Black chip
        return tm.isDark() ? juce::Colour(0xFF2A2A2A) : juce::Colour(0xFF333333);
    }
}

juce::Colour ChipStack::getDarkChipColor(bool isRed) const
{
    auto& tm = ThemeManager::getInstance();

    if (isRed)
    {
        // Darker red
        return tm.isDark() ? juce::Colour(0xFF8B0000) : juce::Colour(0xFF7F1D1D);
    }
    else
    {
        // Darker black
        return tm.isDark() ? juce::Colour(0xFF1A1A1A) : juce::Colour(0xFF1F1F1F);
    }
}

} // namespace BlindCard
