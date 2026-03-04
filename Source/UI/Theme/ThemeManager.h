/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    ThemeManager.h
    Created: 2026-01-19
    Author:  BlindCard

    Singleton class that manages theme switching and provides color access.
    All UI components should add themselves as ChangeListener to receive
    theme change notifications.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemeColors.h"

namespace BlindCard
{

//==============================================================================
/**
 * Enumeration of all color identifiers used throughout the UI.
 * Use these with ThemeManager::getColour() to get theme-appropriate colors.
 */
enum class ColourId
{
    // Backgrounds
    Background,
    Surface,
    SurfaceAlt,

    // Primary Colors
    Primary,
    PrimaryHover,
    Accent,
    AccentMuted,

    // Text
    TextPrimary,
    TextSecondary,
    TextMuted,

    // Poker Table
    TableFelt,
    TableFeltCenter,
    TableFrameOuter,
    TableFrameInner,
    TableRail,
    NeonRed,

    // Status
    Success,
    Error,

    // Components
    SliderTrack,
    SliderFill,
    ToggleOff,
    ToggleOn,
    CardFront,
    StarEmpty,
    StarFilled
};

//==============================================================================
/**
 * ThemeManager is a singleton class that manages the application's color theme.
 *
 * Usage:
 * 1. Get the instance: auto& theme = ThemeManager::getInstance();
 * 2. Add as listener: theme.addChangeListener(this);
 * 3. Get colors: theme.getColour(ColourId::Primary);
 * 4. In changeListenerCallback(): repaint();
 */
class ThemeManager : public juce::ChangeBroadcaster
{
public:
    //==========================================================================
    /** Available themes */
    enum class Theme
    {
        Dark,
        Light
    };

    //==========================================================================
    /** Returns the singleton instance of ThemeManager */
    static ThemeManager& getInstance();

    //==========================================================================
    /** Sets the current theme and notifies all listeners */
    void setTheme(Theme theme);

    /** Returns the current theme */
    Theme getTheme() const { return currentTheme; }

    /** Toggles between Dark and Light themes */
    void toggleTheme();

    /** Returns true if the current theme is Dark */
    bool isDark() const { return currentTheme == Theme::Dark; }

    //==========================================================================
    /** Returns the colour for the given ColourId based on current theme */
    juce::Colour getColour(ColourId id) const;

private:
    //==========================================================================
    ThemeManager() = default;
    ~ThemeManager() override = default;

    Theme currentTheme = Theme::Light;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};

} // namespace BlindCard
