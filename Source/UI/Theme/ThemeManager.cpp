/*
  ==============================================================================

    ThemeManager.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the ThemeManager singleton.

  ==============================================================================
*/

#include "ThemeManager.h"

namespace BlindCard
{

//==============================================================================
ThemeManager& ThemeManager::getInstance()
{
    static ThemeManager instance;
    return instance;
}

//==============================================================================
void ThemeManager::setTheme(Theme theme)
{
    if (currentTheme != theme)
    {
        currentTheme = theme;
        sendChangeMessage();
    }
}

//==============================================================================
void ThemeManager::toggleTheme()
{
    setTheme(currentTheme == Theme::Dark ? Theme::Light : Theme::Dark);
}

//==============================================================================
juce::Colour ThemeManager::getColour(ColourId id) const
{
    // Use namespace aliases to avoid collision with ThemeManager::Theme enum
    namespace DarkTheme = ::BlindCard::Theme::Dark;
    namespace LightTheme = ::BlindCard::Theme::Light;

    if (currentTheme == Theme::Dark)
    {
        switch (id)
        {
            // Backgrounds
            case ColourId::Background:      return DarkTheme::Background;
            case ColourId::Surface:         return DarkTheme::Surface;
            case ColourId::SurfaceAlt:      return DarkTheme::SurfaceAlt;

            // Primary Colors
            case ColourId::Primary:         return DarkTheme::Primary;
            case ColourId::PrimaryHover:    return DarkTheme::PrimaryHover;
            case ColourId::Accent:          return DarkTheme::Accent;
            case ColourId::AccentMuted:     return DarkTheme::AccentMuted;

            // Text
            case ColourId::TextPrimary:     return DarkTheme::TextPrimary;
            case ColourId::TextSecondary:   return DarkTheme::TextSecondary;
            case ColourId::TextMuted:       return DarkTheme::TextMuted;

            // Poker Table
            case ColourId::TableFelt:       return DarkTheme::TableFelt;
            case ColourId::TableFeltCenter: return DarkTheme::TableFeltCenter;
            case ColourId::TableWoodLight:  return DarkTheme::TableWoodLight;
            case ColourId::TableWoodDark:   return DarkTheme::TableWoodDark;
            case ColourId::TableRail:       return DarkTheme::TableRail;
            case ColourId::NeonRed:         return DarkTheme::NeonRed;

            // Status
            case ColourId::Success:         return DarkTheme::Success;
            case ColourId::Error:           return DarkTheme::Error;

            // Components
            case ColourId::SliderTrack:     return DarkTheme::SliderTrack;
            case ColourId::SliderFill:      return DarkTheme::SliderFill;
            case ColourId::ToggleOff:       return DarkTheme::ToggleOff;
            case ColourId::ToggleOn:        return DarkTheme::ToggleOn;
            case ColourId::CardFront:       return DarkTheme::CardFront;
            case ColourId::StarEmpty:       return DarkTheme::StarEmpty;
            case ColourId::StarFilled:      return DarkTheme::StarFilled;

            default:                        return DarkTheme::Background;
        }
    }
    else // Light theme
    {
        switch (id)
        {
            // Backgrounds
            case ColourId::Background:      return LightTheme::Background;
            case ColourId::Surface:         return LightTheme::Surface;
            case ColourId::SurfaceAlt:      return LightTheme::SurfaceAlt;

            // Primary Colors
            case ColourId::Primary:         return LightTheme::Primary;
            case ColourId::PrimaryHover:    return LightTheme::PrimaryHover;
            case ColourId::Accent:          return LightTheme::Accent;
            case ColourId::AccentMuted:     return LightTheme::AccentMuted;

            // Text
            case ColourId::TextPrimary:     return LightTheme::TextPrimary;
            case ColourId::TextSecondary:   return LightTheme::TextSecondary;
            case ColourId::TextMuted:       return LightTheme::TextMuted;

            // Poker Table
            case ColourId::TableFelt:       return LightTheme::TableFelt;
            case ColourId::TableFeltCenter: return LightTheme::TableFeltCenter;
            case ColourId::TableWoodLight:  return LightTheme::TableWoodLight;
            case ColourId::TableWoodDark:   return LightTheme::TableWoodDark;
            case ColourId::TableRail:       return LightTheme::TableRail;
            case ColourId::NeonRed:         return LightTheme::NeonRed;

            // Status
            case ColourId::Success:         return LightTheme::Success;
            case ColourId::Error:           return LightTheme::Error;

            // Components
            case ColourId::SliderTrack:     return LightTheme::SliderTrack;
            case ColourId::SliderFill:      return LightTheme::SliderFill;
            case ColourId::ToggleOff:       return LightTheme::ToggleOff;
            case ColourId::ToggleOn:        return LightTheme::ToggleOn;
            case ColourId::CardFront:       return LightTheme::CardFront;
            case ColourId::StarEmpty:       return LightTheme::StarEmpty;
            case ColourId::StarFilled:      return LightTheme::StarFilled;

            default:                        return LightTheme::Background;
        }
    }
}

} // namespace BlindCard
