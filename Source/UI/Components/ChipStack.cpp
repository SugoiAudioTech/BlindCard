/*
  ==============================================================================

    ChipStack.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Poker chip stack with top-down perspective view.
    Matches the original UI/UX design with alternating red/black chips,
    white edge markers, and gold ring details.

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

    // Chip dimensions - larger for better detail
    float chipDiameter = juce::jmin(bounds.getWidth() * 0.95f, 70.0f);
    float chipSpacing = chipDiameter * 0.22f;  // Vertical spacing between chips

    // Calculate total stack height
    float totalStackHeight = chipDiameter + (numChips - 1) * chipSpacing;

    // Start position (bottom of stack)
    float centerX = bounds.getCentreX();
    float startY = bounds.getCentreY() + totalStackHeight / 2.0f - chipDiameter / 2.0f;

    // Draw chips from bottom to top (so top chips overlap bottom ones)
    for (int i = 0; i < numChips; ++i)
    {
        bool isRed = startRed ? (i % 2 == 0) : (i % 2 != 0);

        // Position for this chip
        float chipY = startY - (i * chipSpacing);

        juce::Rectangle<float> chipBounds(
            centerX - chipDiameter / 2.0f,
            chipY - chipDiameter / 2.0f,
            chipDiameter,
            chipDiameter
        );

        // Draw chip with 3D edge effect
        drawChipWithEdge(g, chipBounds, isRed, i);
    }
}

void ChipStack::drawChipWithEdge(juce::Graphics& g, juce::Rectangle<float> bounds,
                                  bool isRed, int stackIndex)
{
    float diameter = bounds.getWidth();
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();

    // Draw 3D edge (thickness) - darker layers below the top surface
    float edgeThickness = diameter * 0.08f;
    for (int layer = 3; layer >= 0; --layer)
    {
        float yOffset = layer * (edgeThickness / 3.0f);
        auto edgeColor = isRed ? juce::Colour(0xFF5C1515) : juce::Colour(0xFF151515);
        edgeColor = edgeColor.darker(layer * 0.15f);

        g.setColour(edgeColor);
        g.fillEllipse(bounds.getX(), bounds.getY() + yOffset,
                      diameter, diameter);
    }

    // Draw main chip top surface
    drawChipTop(g, bounds, isRed);
}

void ChipStack::drawChipTop(juce::Graphics& g, juce::Rectangle<float> bounds, bool isRed)
{
    float diameter = bounds.getWidth();
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();
    float radius = diameter / 2.0f;

    // Colors
    juce::Colour mainColor = isRed ? juce::Colour(0xFF8B2942) : juce::Colour(0xFF1A1A1A);
    juce::Colour darkColor = isRed ? juce::Colour(0xFF5C1525) : juce::Colour(0xFF0D0D0D);
    juce::Colour goldColor(0xFFD4A84B);
    juce::Colour creamColor(0xFFF5F0E1);

    // 1. Main chip body with gradient
    {
        juce::ColourGradient gradient(
            mainColor.brighter(0.15f),
            centerX, bounds.getY(),
            darkColor,
            centerX, bounds.getBottom(),
            false
        );
        g.setGradientFill(gradient);
        g.fillEllipse(bounds);
    }

    // 2. Subtle inner shadow ring
    {
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.drawEllipse(bounds.reduced(2.0f), 2.0f);
    }

    // 3. White/cream edge markers - 8 markers around the perimeter
    {
        int numMarkers = 8;
        float markerWidth = diameter * 0.09f;
        float markerHeight = diameter * 0.22f;
        float markerRadius = radius * 0.82f;  // Distance from center

        for (int i = 0; i < numMarkers; ++i)
        {
            float angle = (i / static_cast<float>(numMarkers)) * juce::MathConstants<float>::twoPi
                          - juce::MathConstants<float>::halfPi;  // Start from top

            float markerX = centerX + markerRadius * std::cos(angle);
            float markerY = centerY + markerRadius * std::sin(angle);

            // Save graphics state
            g.saveState();

            // Translate to marker position and rotate
            juce::AffineTransform transform = juce::AffineTransform::rotation(
                angle + juce::MathConstants<float>::halfPi,
                markerX, markerY
            );

            // Draw rounded rectangle marker (pill shape)
            juce::Path markerPath;
            markerPath.addRoundedRectangle(
                markerX - markerWidth / 2.0f,
                markerY - markerHeight / 2.0f,
                markerWidth,
                markerHeight,
                markerWidth / 2.0f
            );
            markerPath.applyTransform(transform);

            // Gradient for 3D effect on markers
            g.setColour(creamColor);
            g.fillPath(markerPath);

            // Subtle shadow on markers
            g.setColour(juce::Colours::black.withAlpha(0.15f));
            g.strokePath(markerPath, juce::PathStrokeType(0.5f));

            g.restoreState();
        }
    }

    // 4. Outer gold ring
    {
        float outerRingRadius = radius * 0.62f;
        auto outerRingBounds = bounds.reduced((radius - outerRingRadius));

        // Gold glow
        g.setColour(goldColor.withAlpha(0.3f));
        g.drawEllipse(outerRingBounds.expanded(1.5f), 3.0f);

        // Main gold ring
        g.setColour(goldColor);
        g.drawEllipse(outerRingBounds, 2.5f);

        // Highlight on gold ring
        g.setColour(goldColor.brighter(0.4f));
        juce::Path highlightArc;
        highlightArc.addCentredArc(centerX, centerY,
                                    outerRingRadius, outerRingRadius,
                                    0.0f,
                                    -juce::MathConstants<float>::pi * 0.7f,
                                    -juce::MathConstants<float>::pi * 0.3f,
                                    true);
        g.strokePath(highlightArc, juce::PathStrokeType(1.5f));
    }

    // 5. Inner black/colored circle
    {
        float innerRadius = radius * 0.52f;
        auto innerBounds = bounds.reduced(radius - innerRadius);

        juce::ColourGradient innerGradient(
            juce::Colour(0xFF2A2A2A),
            centerX, innerBounds.getY(),
            juce::Colour(0xFF0A0A0A),
            centerX, innerBounds.getBottom(),
            false
        );
        g.setGradientFill(innerGradient);
        g.fillEllipse(innerBounds);
    }

    // 6. Inner gold ring
    {
        float innerRingRadius = radius * 0.38f;
        auto innerRingBounds = bounds.reduced(radius - innerRingRadius);

        g.setColour(goldColor.withAlpha(0.9f));
        g.drawEllipse(innerRingBounds, 2.0f);

        // Highlight
        g.setColour(goldColor.brighter(0.3f));
        juce::Path innerHighlight;
        innerHighlight.addCentredArc(centerX, centerY,
                                      innerRingRadius, innerRingRadius,
                                      0.0f,
                                      -juce::MathConstants<float>::pi * 0.7f,
                                      -juce::MathConstants<float>::pi * 0.3f,
                                      true);
        g.strokePath(innerHighlight, juce::PathStrokeType(1.0f));
    }

    // 7. Center highlight/reflection
    {
        auto centerSpot = bounds.reduced(radius * 0.75f);
        centerSpot = centerSpot.translated(-diameter * 0.05f, -diameter * 0.08f);

        juce::ColourGradient shineGradient(
            juce::Colours::white.withAlpha(0.15f),
            centerSpot.getCentreX(), centerSpot.getY(),
            juce::Colours::transparentWhite,
            centerSpot.getCentreX(), centerSpot.getBottom(),
            false
        );
        g.setGradientFill(shineGradient);
        g.fillEllipse(centerSpot);
    }
}

void ChipStack::resized()
{
    // Nothing to do - all drawing is relative to bounds
}

//==============================================================================
juce::Colour ChipStack::getChipColor(bool isRed) const
{
    if (isRed)
        return juce::Colour(0xFF8B2942);  // Burgundy red
    else
        return juce::Colour(0xFF1A1A1A);  // True black
}

juce::Colour ChipStack::getDarkChipColor(bool isRed) const
{
    if (isRed)
        return juce::Colour(0xFF5C1525);  // Dark burgundy
    else
        return juce::Colour(0xFF0D0D0D);  // Darker black
}

} // namespace BlindCard
