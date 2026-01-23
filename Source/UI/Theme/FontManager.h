/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    FontManager.h
    Created: 2026-01-20
    Author:  BlindCard

    Manages embedded custom fonts for the plugin.
    Provides Noto Sans TC font family (Regular, Medium, Bold) for consistent
    cross-platform typography with Chinese support.

  ==============================================================================
*/

#pragma once

#include <juce_graphics/juce_graphics.h>
#include "BinaryData.h"

namespace BlindCard
{

/**
 * Singleton class that manages custom embedded fonts.
 *
 * Usage:
 *   auto& fonts = FontManager::getInstance();
 *   g.setFont(fonts.getBold(14.0f));    // Bold
 *   g.setFont(fonts.getMedium(12.0f));  // Medium
 *   g.setFont(fonts.getRegular(11.0f)); // Regular
 *
 * Legacy function names (getInterXxx) are kept for backward compatibility.
 */
class FontManager
{
public:
    static FontManager& getInstance()
    {
        static FontManager instance;
        return instance;
    }

    /**
     * Get Regular font at specified size.
     * Use for body text, secondary labels.
     */
    juce::Font getRegular(float height) const
    {
        if (regularTypeface != nullptr)
            return juce::Font(regularTypeface).withHeight(height);
        return juce::Font(juce::FontOptions().withHeight(height));
    }

    /**
     * Get Medium font at specified size.
     * Use for labels, UI elements.
     */
    juce::Font getMedium(float height) const
    {
        if (mediumTypeface != nullptr)
            return juce::Font(mediumTypeface).withHeight(height);
        return juce::Font(juce::FontOptions().withHeight(height));
    }

    /**
     * Get Bold font at specified size.
     * Use for headings, emphasis, important text.
     */
    juce::Font getBold(float height) const
    {
        if (boldTypeface != nullptr)
            return juce::Font(boldTypeface).withHeight(height);
        return juce::Font(juce::FontOptions().withHeight(height).withStyle("Bold"));
    }

    // Legacy function names for backward compatibility
    juce::Font getInterRegular(float height) const { return getRegular(height); }
    juce::Font getInterMedium(float height) const { return getMedium(height); }
    juce::Font getInterBold(float height) const { return getBold(height); }

    //==========================================================================
    // AirCheck-style convenience functions
    // Note: Sizes are scaled up from web (1.2-1.4x) for native UI readability
    //==========================================================================

    /**
     * Get label font (AirCheck style).
     * For labels like "TRACKS", "ROUNDS", "LEVEL-MATCH"
     * Note: Text should be drawn in uppercase manually.
     */
    juce::Font getLabel(float height = 14.0f) const
    {
        return getRegular(height).withExtraKerningFactor(0.08f);  // Wide tracking
    }

    /**
     * Get value font (AirCheck style).
     * For numeric values like track count, round counter
     */
    juce::Font getValue(float height = 28.0f) const
    {
        return getRegular(height);
    }

    /**
     * Get large value font (AirCheck style).
     * For prominent numbers like round display "1/3"
     */
    juce::Font getLargeValue(float height = 38.0f) const
    {
        return getBold(height);
    }

    /**
     * Get button font (AirCheck style).
     * Bold, for button labels like "REVEAL", "RESET"
     */
    juce::Font getButton(float height = 16.0f) const
    {
        return getBold(height).withExtraKerningFactor(0.05f);
    }

    //==========================================================================
    // Casino-style display fonts
    //==========================================================================

    /**
     * Get Bebas Neue font (Casino button style).
     * All-caps, bold, neon sign feel.
     * Use for: REVEAL, SHUFFLE, RESET buttons
     * @param height Font height in pixels
     * @param horizontalScale Width scale (1.0 = normal, 1.2 = 20% wider)
     */
    juce::Font getBebasNeue(float height, float horizontalScale = 1.2f) const
    {
        if (bebasNeueTypeface != nullptr)
            return juce::Font(bebasNeueTypeface)
                .withHeight(height)
                .withHorizontalScale(horizontalScale);
        return juce::Font(juce::FontOptions().withHeight(height).withStyle("Bold"))
            .withHorizontalScale(horizontalScale);
    }

    /**
     * Get Cinzel Regular font (Art Deco style).
     * Elegant, classic, luxury feel.
     * Use for: Round display, decorative titles
     */
    juce::Font getCinzel(float height) const
    {
        if (cinzelRegularTypeface != nullptr)
            return juce::Font(cinzelRegularTypeface).withHeight(height);
        return juce::Font(juce::FontOptions().withHeight(height));
    }

    /**
     * Get Cinzel Bold font (Art Deco style).
     * For emphasized titles and headings.
     */
    juce::Font getCinzelBold(float height) const
    {
        if (cinzelBoldTypeface != nullptr)
            return juce::Font(cinzelBoldTypeface).withHeight(height);
        return juce::Font(juce::FontOptions().withHeight(height).withStyle("Bold"));
    }

    /**
     * Get casino-style button font.
     * Bebas Neue with tracking for chip buttons.
     */
    juce::Font getCasinoButton(float height = 18.0f) const
    {
        return getBebasNeue(height).withExtraKerningFactor(0.08f);
    }

    /**
     * Get casino-style title font.
     * Cinzel Bold for round indicators, section headers.
     */
    juce::Font getCasinoTitle(float height = 24.0f) const
    {
        return getCinzelBold(height).withExtraKerningFactor(0.05f);
    }

    /**
     * Get small label font (AirCheck style).
     * For scale markers and tiny labels
     */
    juce::Font getSmallLabel(float height = 12.0f) const
    {
        return getRegular(height);
    }

    /**
     * Check if custom fonts loaded successfully.
     */
    bool isLoaded() const
    {
        return regularTypeface != nullptr &&
               mediumTypeface != nullptr &&
               boldTypeface != nullptr;
    }

    /**
     * Check if casino fonts loaded successfully.
     */
    bool isCasinoFontsLoaded() const
    {
        return bebasNeueTypeface != nullptr &&
               cinzelRegularTypeface != nullptr &&
               cinzelBoldTypeface != nullptr;
    }

private:
    FontManager()
    {
        loadFonts();
    }

    ~FontManager() = default;

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    void loadFonts()
    {
        // Load Noto Sans TC Regular
        regularTypeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::NotoSansTCRegular_otf,
            BinaryData::NotoSansTCRegular_otfSize);

        // Load Noto Sans TC Medium
        mediumTypeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::NotoSansTCMedium_otf,
            BinaryData::NotoSansTCMedium_otfSize);

        // Load Noto Sans TC Bold
        boldTypeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::NotoSansTCBold_otf,
            BinaryData::NotoSansTCBold_otfSize);

        DBG("FontManager: Noto Sans TC fonts loaded - Regular: " +
            juce::String(regularTypeface != nullptr ? "OK" : "FAIL") +
            ", Medium: " +
            juce::String(mediumTypeface != nullptr ? "OK" : "FAIL") +
            ", Bold: " +
            juce::String(boldTypeface != nullptr ? "OK" : "FAIL"));

        // Load Bebas Neue (casino button font)
        bebasNeueTypeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::BebasNeueRegular_ttf,
            BinaryData::BebasNeueRegular_ttfSize);

        // Load Cinzel (Art Deco title font)
        cinzelRegularTypeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::CinzelRegular_ttf,
            BinaryData::CinzelRegular_ttfSize);

        cinzelBoldTypeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::CinzelBold_ttf,
            BinaryData::CinzelBold_ttfSize);

        DBG("FontManager: Casino fonts loaded - BebasNeue: " +
            juce::String(bebasNeueTypeface != nullptr ? "OK" : "FAIL") +
            ", Cinzel: " +
            juce::String(cinzelRegularTypeface != nullptr ? "OK" : "FAIL") +
            ", CinzelBold: " +
            juce::String(cinzelBoldTypeface != nullptr ? "OK" : "FAIL"));
    }

    // Base fonts (Noto Sans TC)
    juce::Typeface::Ptr regularTypeface;
    juce::Typeface::Ptr mediumTypeface;
    juce::Typeface::Ptr boldTypeface;

    // Casino display fonts
    juce::Typeface::Ptr bebasNeueTypeface;
    juce::Typeface::Ptr cinzelRegularTypeface;
    juce::Typeface::Ptr cinzelBoldTypeface;
};

} // namespace BlindCard
