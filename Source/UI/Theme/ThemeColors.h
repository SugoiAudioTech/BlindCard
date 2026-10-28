/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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
    inline const juce::Colour Background        { 0xFF404241 };  // Neutral charcoal canvas
    inline const juce::Colour HeaderSurface     { 0xFF393B3A };  // Defined, related masthead
    inline const juce::Colour Surface           { 0xFF575A57 };  // Raised panels and mode selector
    inline const juce::Colour SurfaceAlt        { 0xFF626562 };  // Secondary controls

    // Primary Colors
    inline const juce::Colour Primary           { 0xFFA64B57 };  // Restrained action red
    inline const juce::Colour PrimaryHover      { 0xFFB45B65 };
    inline const juce::Colour Accent            { 0xFFD6B67E };  // Soft gold, used sparingly
    inline const juce::Colour AccentMuted       { 0xFFC2A875 };

    // Text
    inline const juce::Colour TextPrimary       { 0xFFF2F3F1 };
    inline const juce::Colour TextSecondary     { 0xFFD4D8D3 };
    inline const juce::Colour TextMuted         { 0xFFCFD4CE };

    // Poker Table (Casino green felt design)
    inline const juce::Colour TableFelt         { 0xFF1A7A5A };  // Emerald green felt (edge)
    inline const juce::Colour TableFeltCenter   { 0xFF2AAA7A };  // Brighter emerald center
    inline const juce::Colour TableFrameOuter   { 0xFF777D78 };  // Neutral rim keeps felt central
    inline const juce::Colour TableFrameInner   { 0xFF5D635E };  // Shaded rim
    inline const juce::Colour TableRail         { 0xFF505752 };  // Padded rail
    inline const juce::Colour NeonRed           { 0xFF826D70 };  // Quiet red keyline

    // Status
    inline const juce::Colour Success           { 0xFF9AC8AB };
    inline const juce::Colour Error             { 0xFFE7989B };

    // Components
    inline const juce::Colour SliderTrack       { 0xFF797E79 };
    inline const juce::Colour SliderFill        { 0xFFA64B57 };
    inline const juce::Colour ToggleOff         { 0xFF797E79 };
    inline const juce::Colour ToggleOn          { 0xFFA64B57 };
    inline const juce::Colour CardFront         { 0xFFFBFAF5 };
    inline const juce::Colour StarEmpty         { 0xFFA8B0A8 };
    inline const juce::Colour StarFilled        { 0xFFD6B67E };

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
    inline const juce::Colour TextMuted         { 0xFF667080 };

    // Poker Table (Light theme - similar green felt)
    inline const juce::Colour TableFelt         { 0xFF1A7A5A };  // Emerald green felt (edge)
    inline const juce::Colour TableFeltCenter   { 0xFF2D9B7A };  // Brighter emerald center
    inline const juce::Colour TableFrameOuter   { 0xFFB5A99B };  // Warm taupe outer frame
    inline const juce::Colour TableFrameInner   { 0xFF978A7C };  // Soft inner frame
    inline const juce::Colour TableRail         { 0xFF756C61 };  // Subdued padded rail
    inline const juce::Colour NeonRed           { 0xFFA45B56 };  // Muted red table keyline

    // Status
    inline const juce::Colour Success           { 0xFF16A34A };
    inline const juce::Colour Error             { 0xFFDC2626 };

    // Components
    inline const juce::Colour SliderTrack       { 0xFF8A8A8A };  // Darker for visibility on light bg
    inline const juce::Colour SliderFill        { 0xFFC41E3A };
    inline const juce::Colour ToggleOff         { 0xFFD4D4D4 };
    inline const juce::Colour ToggleOn          { 0xFFC41E3A };
    inline const juce::Colour CardFront         { 0xFFFFFFFF };
    inline const juce::Colour StarEmpty         { 0xFFD4D4D4 };
    inline const juce::Colour StarFilled        { 0xFFD4AF37 };

} // namespace Light

} // namespace Theme
} // namespace BlindCard
