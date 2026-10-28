/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    GuessDropdown.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the dropdown component for track guessing.

  ==============================================================================
*/

#include "GuessDropdown.h"
#include "../Theme/FontManager.h"

#include <memory>

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
    constexpr int menuItemHeight = 32;
    constexpr float arrowGap = 5.0f;
}

namespace
{
    class GuessMenuLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        explicit GuessMenuLookAndFeel(bool darkTheme) : dark(darkTheme)
        {
            setColour(juce::PopupMenu::backgroundColourId,
                      juce::Colour(dark ? 0xFF343937 : 0xFFFAF9F6));
            setColour(juce::PopupMenu::textColourId,
                      juce::Colour(dark ? 0xFFF5F2EA : 0xFF303431));
            setColour(juce::PopupMenu::highlightedBackgroundColourId,
                      juce::Colour(dark ? 0xFF506058 : 0xFFDDE5DE));
            setColour(juce::PopupMenu::highlightedTextColourId,
                      juce::Colour(dark ? 0xFFFFFFFF : 0xFF1F2923));
        }

        juce::Font getPopupMenuFont() override
        {
            return FontManager::getInstance().getMedium(13.5f);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               bool isSeparator, bool isActive, bool isHighlighted,
                               bool isTicked, bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* textColour) override
        {
            if (!isSeparator && isTicked && !isHighlighted)
            {
                g.setColour(juce::Colour(dark ? 0xFF49524B : 0xFFEDE5D8));
                g.fillRoundedRectangle(area.reduced(2).toFloat(), 4.0f);
            }

            juce::LookAndFeel_V4::drawPopupMenuItem(g, area, isSeparator, isActive,
                isHighlighted, isTicked, hasSubMenu, text, shortcutKeyText, icon,
                textColour);
        }

    private:
        bool dark;
    };

    float textWidth(const juce::Font& font, const juce::String& text)
    {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(font, text, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, glyphs.getNumGlyphs(), true).getWidth();
    }
}

//==============================================================================
GuessDropdown::GuessDropdown()
{
    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);
    LocalizationManager::getInstance().addListener(this);

    // Enable mouse tracking for hover effects
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

GuessDropdown::~GuessDropdown()
{
    ThemeManager::getInstance().removeChangeListener(this);
    LocalizationManager::getInstance().removeListener(this);
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

    // Apply opacity to the complete field, including colours set by helpers.
    if (!enabledState)
        g.beginTransparencyLayer(kDisabledOpacity);

    // Draw background
    drawBackground(g, bounds);

    // Draw border
    drawBorder(g, bounds);

    // Draw text
    drawText(g, bounds);

    // Draw arrow
    drawArrow(g, bounds);

    if (!enabledState)
        g.endTransparencyLayer();
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

void GuessDropdown::languageChanged()
{
    repaint();
}

//==============================================================================
void GuessDropdown::drawBackground(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto colour = getBackgroundColour();
    if (enabledState && isHovered)
        colour = juce::Colour(ThemeManager::getInstance().isDark()
                                  ? 0xFF535B56 : 0xFFE8EBE6);
    g.setColour(colour);
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));
}

void GuessDropdown::drawBorder(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto borderColour = getBorderColour();

    if (enabledState && isHovered)
        borderColour = juce::Colour(ThemeManager::getInstance().isDark()
                                        ? 0xFFB3BDB3 : 0xFF738B78);

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
    auto textColour = selectedIndex < 0
        ? juce::Colour(ThemeManager::getInstance().isDark()
                           ? 0xFFD4D8D1 : 0xFF4E544E)
        : getTextColour();

    g.setColour(textColour);
    g.setFont(FontManager::getInstance().getMedium(kFontSize));

    // Calculate text bounds (leave room for arrow)
    auto textBounds = bounds;
    textBounds.removeFromLeft(static_cast<float>(kTextPadding));
    textBounds.removeFromRight(static_cast<float>(kArrowPadding) + DropdownLayout::arrowWidth +
                               DropdownLayout::arrowGap);

    // Draw text
    juce::String displayText;
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(tracks.size()))
    {
        displayText = juce::String(tracks[static_cast<size_t>(selectedIndex)]);
    }
    else
    {
        displayText = LOCALIZE(GuessSelect);
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

    // Keep the menu's look and feel alive through its asynchronous callback.
    juce::PopupMenu menu;
    auto menuLookAndFeel = std::make_shared<GuessMenuLookAndFeel>(
        ThemeManager::getInstance().isDark());
    menu.setLookAndFeel(menuLookAndFeel.get());

    float widestTrack = 0.0f;
    const auto menuFont = menuLookAndFeel->getPopupMenuFont();
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        int itemId = static_cast<int>(i) + 1;  // PopupMenu IDs must be > 0
        bool isTicked = (static_cast<int>(i) == selectedIndex);
        auto name = juce::String(tracks[i]);
        widestTrack = juce::jmax(widestTrack, textWidth(menuFont, name));
        menu.addItem(itemId, name, true, isTicked);
    }

    const int menuWidth = juce::jlimit(240, 400,
        juce::roundToInt(widestTrack) + 56);
    juce::PopupMenu::Options options;
    options = options.withTargetComponent(this)
                     .withMinimumWidth(menuWidth)
                     .withMaximumNumColumns(1)
                     .withStandardItemHeight(DropdownLayout::menuItemHeight);
    if (selectedIndex >= 0)
        options = options.withItemThatMustBeVisible(selectedIndex + 1);

    juce::Component::SafePointer<GuessDropdown> safeThis(this);
    menu.showMenuAsync(options, [safeThis, menuLookAndFeel](int result) {
        juce::ignoreUnused(menuLookAndFeel);
        if (safeThis != nullptr)
            safeThis->handleMenuResult(result);
    });
}

void GuessDropdown::handleMenuResult(int result)
{
    isMenuOpen = false;
    repaint();

    if (result > 0)
    {
        int newIndex = result - 1;  // Convert back from PopupMenu ID

        if (newIndex < static_cast<int>(tracks.size()) && newIndex != selectedIndex)
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
    auto& theme = ThemeManager::getInstance();

    if (theme.isDark())
    {
        return juce::Colour(0xFF484C49);
    }
    else
    {
        return juce::Colour(0xFFF4F2ED);
    }
}

juce::Colour GuessDropdown::getBorderColour() const
{
    auto& theme = ThemeManager::getInstance();

    if (theme.isDark())
    {
        return juce::Colour(0xFF868D86);
    }
    else
    {
        return juce::Colour(0xFFA9AAA4);
    }
}

juce::Colour GuessDropdown::getTextColour() const
{
    return ThemeManager::getInstance().isDark()
        ? juce::Colour(0xFFF5F2EA) : juce::Colour(0xFF30332F);
}

juce::Colour GuessDropdown::getArrowColour() const
{
    return ThemeManager::getInstance().isDark()
        ? juce::Colour(0xFFE0E5DD) : juce::Colour(0xFF59605A);
}

} // namespace BlindCard
