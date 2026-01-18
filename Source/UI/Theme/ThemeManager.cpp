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
    if (currentTheme == Theme::Dark)
    {
        switch (id)
        {
            // Backgrounds
            case ColourId::Background:      return Theme::Dark::Background;
            case ColourId::Surface:         return Theme::Dark::Surface;
            case ColourId::SurfaceAlt:      return Theme::Dark::SurfaceAlt;

            // Primary Colors
            case ColourId::Primary:         return Theme::Dark::Primary;
            case ColourId::PrimaryHover:    return Theme::Dark::PrimaryHover;
            case ColourId::Accent:          return Theme::Dark::Accent;
            case ColourId::AccentMuted:     return Theme::Dark::AccentMuted;

            // Text
            case ColourId::TextPrimary:     return Theme::Dark::TextPrimary;
            case ColourId::TextSecondary:   return Theme::Dark::TextSecondary;
            case ColourId::TextMuted:       return Theme::Dark::TextMuted;

            // Poker Table
            case ColourId::TableFelt:       return Theme::Dark::TableFelt;
            case ColourId::TableFeltCenter: return Theme::Dark::TableFeltCenter;
            case ColourId::TableWoodLight:  return Theme::Dark::TableWoodLight;
            case ColourId::TableWoodDark:   return Theme::Dark::TableWoodDark;
            case ColourId::TableRail:       return Theme::Dark::TableRail;
            case ColourId::NeonRed:         return Theme::Dark::NeonRed;

            // Status
            case ColourId::Success:         return Theme::Dark::Success;
            case ColourId::Error:           return Theme::Dark::Error;

            // Components
            case ColourId::SliderTrack:     return Theme::Dark::SliderTrack;
            case ColourId::SliderFill:      return Theme::Dark::SliderFill;
            case ColourId::ToggleOff:       return Theme::Dark::ToggleOff;
            case ColourId::ToggleOn:        return Theme::Dark::ToggleOn;
            case ColourId::CardFront:       return Theme::Dark::CardFront;
            case ColourId::StarEmpty:       return Theme::Dark::StarEmpty;
            case ColourId::StarFilled:      return Theme::Dark::StarFilled;

            default:                        return Theme::Dark::Background;
        }
    }
    else // Light theme
    {
        switch (id)
        {
            // Backgrounds
            case ColourId::Background:      return Theme::Light::Background;
            case ColourId::Surface:         return Theme::Light::Surface;
            case ColourId::SurfaceAlt:      return Theme::Light::SurfaceAlt;

            // Primary Colors
            case ColourId::Primary:         return Theme::Light::Primary;
            case ColourId::PrimaryHover:    return Theme::Light::PrimaryHover;
            case ColourId::Accent:          return Theme::Light::Accent;
            case ColourId::AccentMuted:     return Theme::Light::AccentMuted;

            // Text
            case ColourId::TextPrimary:     return Theme::Light::TextPrimary;
            case ColourId::TextSecondary:   return Theme::Light::TextSecondary;
            case ColourId::TextMuted:       return Theme::Light::TextMuted;

            // Poker Table
            case ColourId::TableFelt:       return Theme::Light::TableFelt;
            case ColourId::TableFeltCenter: return Theme::Light::TableFeltCenter;
            case ColourId::TableWoodLight:  return Theme::Light::TableWoodLight;
            case ColourId::TableWoodDark:   return Theme::Light::TableWoodDark;
            case ColourId::TableRail:       return Theme::Light::TableRail;
            case ColourId::NeonRed:         return Theme::Light::NeonRed;

            // Status
            case ColourId::Success:         return Theme::Light::Success;
            case ColourId::Error:           return Theme::Light::Error;

            // Components
            case ColourId::SliderTrack:     return Theme::Light::SliderTrack;
            case ColourId::SliderFill:      return Theme::Light::SliderFill;
            case ColourId::ToggleOff:       return Theme::Light::ToggleOff;
            case ColourId::ToggleOn:        return Theme::Light::ToggleOn;
            case ColourId::CardFront:       return Theme::Light::CardFront;
            case ColourId::StarEmpty:       return Theme::Light::StarEmpty;
            case ColourId::StarFilled:      return Theme::Light::StarFilled;

            default:                        return Theme::Light::Background;
        }
    }
}

} // namespace BlindCard
