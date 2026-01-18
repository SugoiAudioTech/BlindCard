/*
  ==============================================================================

    GuessDropdown.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the dropdown component for track guessing.

  ==============================================================================
*/

#include "GuessDropdown.h"

namespace BlindCard
{

//==============================================================================
// Layout and design constants
namespace DropdownLayout
{
    // Arrow drawing
    constexpr float arrowWidth = 8.0f;
    constexpr float arrowHeight = 5.0f;

    // PopupMenu item height
    constexpr int menuItemHeight = 28;
}

//==============================================================================
GuessDropdown::GuessDropdown()
{
    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);

    // Enable mouse tracking for hover effects
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

GuessDropdown::~GuessDropdown()
{
    ThemeManager::getInstance().removeChangeListener(this);
}

//==============================================================================
void GuessDropdown::setTracks(const std::vector<std::string>& trackNames)
{
    tracks = trackNames;
    selectedIndex = -1;  // Reset selection when tracks change
    repaint();
}

void GuessDropdown::setSelectedIndex(int index)
{
    // Clamp index to valid range or -1
    if (index < -1)
        index = -1;
    else if (index >= static_cast<int>(tracks.size()))
        index = static_cast<int>(tracks.size()) - 1;

    if (selectedIndex != index)
    {
        selectedIndex = index;
        repaint();
    }
}

std::string GuessDropdown::getSelectedTrackName() const
{
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(tracks.size()))
        return tracks[static_cast<size_t>(selectedIndex)];
    return "";
}

void GuessDropdown::setEnabled(bool shouldBeEnabled)
{
    if (enabledState != shouldBeEnabled)
    {
        enabledState = shouldBeEnabled;

        // Update cursor
        setMouseCursor(shouldBeEnabled
            ? juce::MouseCursor::PointingHandCursor
            : juce::MouseCursor::NormalCursor);

        // Reset hover state when disabled
        if (!shouldBeEnabled)
        {
            isHovered = false;
        }

        repaint();
    }
}

//==============================================================================
void GuessDropdown::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Apply disabled opacity
    if (!enabledState)
    {
        g.setOpacity(kDisabledOpacity);
    }

    // Draw background
    drawBackground(g, bounds);

    // Draw border
    drawBorder(g, bounds);

    // Draw text
    drawText(g, bounds);

    // Draw arrow
    drawArrow(g, bounds);
}

void GuessDropdown::resized()
{
    // No child components to layout
}

//==============================================================================
void GuessDropdown::mouseEnter(const juce::MouseEvent&)
{
    if (!enabledState)
        return;

    isHovered = true;
    repaint();
}

void GuessDropdown::mouseExit(const juce::MouseEvent&)
{
    if (!enabledState)
        return;

    isHovered = false;
    repaint();
}

void GuessDropdown::mouseDown(const juce::MouseEvent&)
{
    if (!enabledState)
        return;

    showPopupMenu();
}

//==============================================================================
void GuessDropdown::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        repaint();
    }
}

//==============================================================================
void GuessDropdown::drawBackground(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(getBackgroundColour());
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));
}

void GuessDropdown::drawBorder(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto borderColour = getBorderColour();

    // Lighten border on hover
    if (isHovered)
    {
        borderColour = borderColour.brighter(0.3f);
    }

    // Highlight border when menu is open
    if (isMenuOpen)
    {
        borderColour = ThemeManager::getInstance().getColour(ColourId::Primary);
    }

    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(kCornerRadius),
                           static_cast<float>(kBorderWidth));
}

void GuessDropdown::drawText(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto textColour = getTextColour();

    // Use muted color for placeholder
    if (selectedIndex < 0)
    {
        textColour = ThemeManager::getInstance().getColour(ColourId::TextMuted);
    }

    g.setColour(textColour);
    g.setFont(juce::Font(kFontSize));

    // Calculate text bounds (leave room for arrow)
    auto textBounds = bounds;
    textBounds.removeFromLeft(static_cast<float>(kTextPadding));
    textBounds.removeFromRight(static_cast<float>(kArrowPadding) + DropdownLayout::arrowWidth +
                               static_cast<float>(kArrowPadding));

    // Draw text
    juce::String displayText;
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(tracks.size()))
    {
        displayText = juce::String(tracks[static_cast<size_t>(selectedIndex)]);
    }
    else
    {
        displayText = placeholderText;
    }

    g.drawText(displayText, textBounds, juce::Justification::centredLeft, true);
}

void GuessDropdown::drawArrow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(getArrowColour());

    // Calculate arrow position (right side, vertically centered)
    float arrowX = bounds.getRight() - static_cast<float>(kArrowPadding) - DropdownLayout::arrowWidth;
    float arrowY = bounds.getCentreY() - DropdownLayout::arrowHeight / 2.0f;

    // Create arrow path
    juce::Path arrowPath;

    if (isMenuOpen)
    {
        // Up arrow (^) when menu is open
        arrowPath.startNewSubPath(arrowX, arrowY + DropdownLayout::arrowHeight);
        arrowPath.lineTo(arrowX + DropdownLayout::arrowWidth / 2.0f, arrowY);
        arrowPath.lineTo(arrowX + DropdownLayout::arrowWidth, arrowY + DropdownLayout::arrowHeight);
    }
    else
    {
        // Down arrow (v) when menu is closed
        arrowPath.startNewSubPath(arrowX, arrowY);
        arrowPath.lineTo(arrowX + DropdownLayout::arrowWidth / 2.0f, arrowY + DropdownLayout::arrowHeight);
        arrowPath.lineTo(arrowX + DropdownLayout::arrowWidth, arrowY);
    }

    g.strokePath(arrowPath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
}

//==============================================================================
void GuessDropdown::showPopupMenu()
{
    if (tracks.empty())
        return;

    isMenuOpen = true;
    repaint();

    // Create popup menu
    juce::PopupMenu menu;

    // Style the popup menu with dark theme colors
    auto& theme = ThemeManager::getInstance();
    juce::PopupMenu::LookAndFeelMethods* laf = nullptr;

    // Set custom colors for the popup
    auto bgColour = theme.getColour(ColourId::Surface);
    auto textColour = theme.getColour(ColourId::TextPrimary);
    auto highlightColour = theme.getColour(ColourId::Primary);

    // Add track items
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        int itemId = static_cast<int>(i) + 1;  // PopupMenu IDs must be > 0
        bool isTicked = (static_cast<int>(i) == selectedIndex);

        menu.addItem(itemId, juce::String(tracks[i]), true, isTicked);
    }

    // Calculate menu position (below the dropdown)
    auto screenBounds = getScreenBounds();

    // PopupMenu options for dark styling
    juce::PopupMenu::Options options;
    options = options.withTargetComponent(this)
                     .withMinimumWidth(getWidth())
                     .withMaximumNumColumns(1)
                     .withStandardItemHeight(DropdownLayout::menuItemHeight);

    // Show menu asynchronously
    menu.showMenuAsync(options, [this](int result) {
        handleMenuResult(result);
    });
}

void GuessDropdown::handleMenuResult(int result)
{
    isMenuOpen = false;
    repaint();

    if (result > 0)
    {
        int newIndex = result - 1;  // Convert back from PopupMenu ID

        if (newIndex != selectedIndex)
        {
            selectedIndex = newIndex;
            repaint();

            if (onSelectionChanged)
            {
                onSelectionChanged(selectedIndex);
            }
        }
    }
}

//==============================================================================
juce::Colour GuessDropdown::getBackgroundColour() const
{
    // Use #2A2A2A as specified, or fall back to theme surface
    return juce::Colour(0xFF2A2A2A);
}

juce::Colour GuessDropdown::getBorderColour() const
{
    auto& theme = ThemeManager::getInstance();

    if (theme.isDark())
    {
        return juce::Colour(0xFF3A3A3A);  // Subtle border for dark theme
    }
    else
    {
        return juce::Colour(0xFFD4D4D4);  // Light theme border
    }
}

juce::Colour GuessDropdown::getTextColour() const
{
    return ThemeManager::getInstance().getColour(ColourId::TextPrimary);
}

juce::Colour GuessDropdown::getArrowColour() const
{
    return ThemeManager::getInstance().getColour(ColourId::TextSecondary);
}

} // namespace BlindCard
