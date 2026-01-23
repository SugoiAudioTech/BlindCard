/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    CardCountControl.cpp
    Created: 2025-01-21
    Author:  BlindCard

  ==============================================================================
*/

#include "CardCountControl.h"
#include "../Theme/FontManager.h"

namespace BlindCard
{

//==============================================================================
CardCountControl::CardCountControl()
{
    ThemeManager::getInstance().addChangeListener(this);
}

CardCountControl::~CardCountControl()
{
    ThemeManager::getInstance().removeChangeListener(this);
}

//==============================================================================
void CardCountControl::setCount(int count)
{
    int newCount = juce::jlimit(minCount, maxCount, count);
    if (currentCount != newCount)
    {
        currentCount = newCount;
        repaint();
    }
}

//==============================================================================
void CardCountControl::paint(juce::Graphics& g)
{
    auto& theme = ThemeManager::getInstance();

    // Draw "Cards:" label - readable font
    auto& fonts = FontManager::getInstance();
    g.setColour(theme.getColour(ColourId::TextPrimary).withAlpha(0.7f));
    g.setFont(fonts.getMedium(13.0f));
    g.drawText("Cards:", labelBounds, juce::Justification::centredRight);

    // Draw decrement button
    {
        bool isHovered = currentHover == HoverState::Decrement;
        bool canDecrement = currentCount > minCount;

        juce::Colour bgColor = isHovered && canDecrement
            ? theme.getColour(ColourId::Accent).withAlpha(0.2f)
            : (theme.isDark() ? juce::Colour(0xFF3A3A3A) : juce::Colour(0xFFE0E0E0));

        g.setColour(bgColor);
        g.fillRoundedRectangle(decrementBounds.toFloat(), 4.0f);

        g.setColour(canDecrement
            ? (isHovered ? theme.getColour(ColourId::Accent) : theme.getColour(ColourId::TextPrimary))
            : theme.getColour(ColourId::TextPrimary).withAlpha(0.3f));
        g.setFont(fonts.getBebasNeue(16.0f));
        g.drawText(juce::String::fromUTF8("－"), decrementBounds, juce::Justification::centred);
    }

    // Draw count value - readable font
    g.setColour(theme.getColour(ColourId::TextPrimary));
    g.setFont(fonts.getMedium(15.0f));
    g.drawText(juce::String(currentCount), valueBounds, juce::Justification::centred);

    // Draw increment button
    {
        bool isHovered = currentHover == HoverState::Increment;
        bool canIncrement = currentCount < maxCount;

        juce::Colour bgColor = isHovered && canIncrement
            ? theme.getColour(ColourId::Accent).withAlpha(0.2f)
            : (theme.isDark() ? juce::Colour(0xFF3A3A3A) : juce::Colour(0xFFE0E0E0));

        g.setColour(bgColor);
        g.fillRoundedRectangle(incrementBounds.toFloat(), 4.0f);

        g.setColour(canIncrement
            ? (isHovered ? theme.getColour(ColourId::Accent) : theme.getColour(ColourId::TextPrimary))
            : theme.getColour(ColourId::TextPrimary).withAlpha(0.3f));
        g.setFont(fonts.getBebasNeue(16.0f));
        g.drawText(juce::String::fromUTF8("＋"), incrementBounds, juce::Justification::centred);
    }
}

void CardCountControl::resized()
{
    auto bounds = getLocalBounds();

    const int buttonSize = 24;
    const int valueWidth = 24;
    const int spacing = 4;

    // Calculate from right to left
    int x = bounds.getWidth();

    // Increment button
    x -= buttonSize;
    incrementBounds = juce::Rectangle<int>(x, (bounds.getHeight() - buttonSize) / 2,
                                            buttonSize, buttonSize);

    // Value display
    x -= valueWidth + spacing;
    valueBounds = juce::Rectangle<int>(x, 0, valueWidth, bounds.getHeight());

    // Decrement button
    x -= buttonSize + spacing;
    decrementBounds = juce::Rectangle<int>(x, (bounds.getHeight() - buttonSize) / 2,
                                            buttonSize, buttonSize);

    // Label takes remaining space
    labelBounds = juce::Rectangle<int>(0, 0, x - spacing, bounds.getHeight());
}

void CardCountControl::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();

    if (decrementBounds.contains(pos))
    {
        decrement();
    }
    else if (incrementBounds.contains(pos))
    {
        increment();
    }
}

void CardCountControl::mouseMove(const juce::MouseEvent& event)
{
    auto newHover = getHoverStateAt(event.getPosition());
    if (newHover != currentHover)
    {
        currentHover = newHover;
        repaint();
    }
}

void CardCountControl::mouseExit(const juce::MouseEvent& /*event*/)
{
    if (currentHover != HoverState::None)
    {
        currentHover = HoverState::None;
        repaint();
    }
}

void CardCountControl::changeListenerCallback(juce::ChangeBroadcaster* /*source*/)
{
    repaint();
}

//==============================================================================
void CardCountControl::increment()
{
    if (currentCount < maxCount)
    {
        currentCount++;
        repaint();

        if (onCountChanged)
            onCountChanged(currentCount);
    }
}

void CardCountControl::decrement()
{
    if (currentCount > minCount)
    {
        currentCount--;
        repaint();

        if (onCountChanged)
            onCountChanged(currentCount);
    }
}

CardCountControl::HoverState CardCountControl::getHoverStateAt(juce::Point<int> position) const
{
    if (decrementBounds.contains(position))
        return HoverState::Decrement;
    if (incrementBounds.contains(position))
        return HoverState::Increment;
    return HoverState::None;
}

} // namespace BlindCard
