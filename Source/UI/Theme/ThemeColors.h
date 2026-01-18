/*
  ==============================================================================

    ThemeColors.h
    Created: 2026-01-19
    Author:  BlindCard

    Color constants for Dark and Light themes.
    All UI components should reference these colors through the ThemeManager.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace BlindCard
{
namespace Theme
{

//==============================================================================
// Dark Theme Colors
//==============================================================================
namespace Dark
{
    // Backgrounds
    inline const juce::Colour Background        { 0xFF0A0A0A };
    inline const juce::Colour Surface           { 0xFF1F1F1F };
    inline const juce::Colour SurfaceAlt        { 0xFF252525 };

    // Primary Colors
    inline const juce::Colour Primary           { 0xFFFF3B4E };
    inline const juce::Colour PrimaryHover      { 0xFFFF5A6A };
    inline const juce::Colour Accent            { 0xFFFFD700 };
    inline const juce::Colour AccentMuted       { 0xFFD4AF37 };

    // Text
    inline const juce::Colour TextPrimary       { 0xFFF5F5F0 };
    inline const juce::Colour TextSecondary     { 0xFF9CA3AF };
    inline const juce::Colour TextMuted         { 0xFF6B7280 };

    // Poker Table (Casino green felt design)
    inline const juce::Colour TableFelt         { 0xFF1A7A5A };  // Emerald green felt (edge)
    inline const juce::Colour TableFeltCenter   { 0xFF2AAA7A };  // Brighter emerald center
    inline const juce::Colour TableFrameOuter   { 0xFFE8A07A };  // Salmon/coral frame outer
    inline const juce::Colour TableFrameInner   { 0xFFD4907A };  // Salmon/coral frame inner
    inline const juce::Colour TableRail         { 0xFFC08060 };  // Darker coral rail
    inline const juce::Colour NeonRed           { 0xFFFF3B4E };  // Bright red neon glow

    // Status
    inline const juce::Colour Success           { 0xFF22C55E };
    inline const juce::Colour Error             { 0xFFEF4444 };

    // Components
    inline const juce::Colour SliderTrack       { 0xFF333333 };
    inline const juce::Colour SliderFill        { 0xFFFF3B4E };
    inline const juce::Colour ToggleOff         { 0xFF333333 };
    inline const juce::Colour ToggleOn          { 0xFFFF3B4E };
    inline const juce::Colour CardFront         { 0xFFF5F5F0 };
    inline const juce::Colour StarEmpty         { 0xFF4A4A4A };
    inline const juce::Colour StarFilled        { 0xFFFFD700 };

} // namespace Dark

//==============================================================================
// Light Theme Colors
//==============================================================================
namespace Light
{
    // Backgrounds
    inline const juce::Colour Background        { 0xFFF5F0E8 };
    inline const juce::Colour Surface           { 0xFFFFFFFF };
    inline const juce::Colour SurfaceAlt        { 0xFFF0F0F0 };

    // Primary Colors
    inline const juce::Colour Primary           { 0xFFC41E3A };
    inline const juce::Colour PrimaryHover      { 0xFFD42E4A };
    inline const juce::Colour Accent            { 0xFFD4AF37 };
    inline const juce::Colour AccentMuted       { 0xFFB8962E };

    // Text
    inline const juce::Colour TextPrimary       { 0xFF1A1A1A };
    inline const juce::Colour TextSecondary     { 0xFF6B7280 };
    inline const juce::Colour TextMuted         { 0xFF9CA3AF };

    // Poker Table (Light theme - similar green felt)
    inline const juce::Colour TableFelt         { 0xFF1A7A5A };  // Emerald green felt (edge)
    inline const juce::Colour TableFeltCenter   { 0xFF2D9B7A };  // Brighter emerald center
    inline const juce::Colour TableFrameOuter   { 0xFFF0B090 };  // Lighter salmon frame outer
    inline const juce::Colour TableFrameInner   { 0xFFE0A080 };  // Lighter salmon frame inner
    inline const juce::Colour TableRail         { 0xFFD09070 };  // Lighter coral rail
    inline const juce::Colour NeonRed           { 0xFFC41E3A };  // Darker red neon glow

    // Status
    inline const juce::Colour Success           { 0xFF16A34A };
    inline const juce::Colour Error             { 0xFFDC2626 };

    // Components
    inline const juce::Colour SliderTrack       { 0xFFD4D4D4 };
    inline const juce::Colour SliderFill        { 0xFFC41E3A };
    inline const juce::Colour ToggleOff         { 0xFFD4D4D4 };
    inline const juce::Colour ToggleOn          { 0xFFC41E3A };
    inline const juce::Colour CardFront         { 0xFFFFFFFF };
    inline const juce::Colour StarEmpty         { 0xFFD4D4D4 };
    inline const juce::Colour StarFilled        { 0xFFD4AF37 };

} // namespace Light

} // namespace Theme
} // namespace BlindCard
