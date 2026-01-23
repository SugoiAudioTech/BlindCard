/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

namespace blindcard
{

/**
 * Glow asset management class
 * Loads and caches PNG texture resources
 */
class GlowAssets
{
public:
    static GlowAssets& getInstance()
    {
        static GlowAssets instance;
        return instance;
    }

    /**
     * Get warm gold glow image (for flip flash)
     * @param use2x Whether to use @2x version
     */
    const juce::Image& getGlowWarm (bool use2x = false) const
    {
        return use2x ? glowWarm2x : glowWarm1x;
    }

    /**
     * Get cool white-gold glow image (for selection state)
     * @param use2x Whether to use @2x version
     */
    const juce::Image& getGlowCool (bool use2x = false) const
    {
        return use2x ? glowCool2x : glowCool1x;
    }

    /**
     * Get Compact mode warm gold glow
     */
    const juce::Image& getGlowWarmCompact (bool use2x = false) const
    {
        return use2x ? glowWarmCompact2x : glowWarmCompact1x;
    }

    /**
     * Get Compact mode cool white-gold glow
     */
    const juce::Image& getGlowCoolCompact (bool use2x = false) const
    {
        return use2x ? glowCoolCompact2x : glowCoolCompact1x;
    }

    /**
     * Check if assets are loaded
     */
    bool isLoaded() const { return loaded; }

private:
    GlowAssets()
    {
        loadAssets();
    }

    void loadAssets()
    {
        // Load Standard size assets
        // JUCE BinaryData converts @ to underscore
        glowWarm1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm1x_png,
            static_cast<size_t> (BinaryData::glow_warm1x_pngSize));

        glowWarm2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm2x_png,
            static_cast<size_t> (BinaryData::glow_warm2x_pngSize));

        glowCool1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool1x_png,
            static_cast<size_t> (BinaryData::glow_cool1x_pngSize));

        glowCool2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool2x_png,
            static_cast<size_t> (BinaryData::glow_cool2x_pngSize));

        // Load Compact size assets
        glowWarmCompact1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm_compact1x_png,
            static_cast<size_t> (BinaryData::glow_warm_compact1x_pngSize));

        glowWarmCompact2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_warm_compact2x_png,
            static_cast<size_t> (BinaryData::glow_warm_compact2x_pngSize));

        glowCoolCompact1x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool_compact1x_png,
            static_cast<size_t> (BinaryData::glow_cool_compact1x_pngSize));

        glowCoolCompact2x = juce::ImageFileFormat::loadFrom (
            BinaryData::glow_cool_compact2x_png,
            static_cast<size_t> (BinaryData::glow_cool_compact2x_pngSize));

        loaded = glowWarm1x.isValid() && glowCool1x.isValid();
    }

    juce::Image glowWarm1x;
    juce::Image glowWarm2x;
    juce::Image glowCool1x;
    juce::Image glowCool2x;

    juce::Image glowWarmCompact1x;
    juce::Image glowWarmCompact2x;
    juce::Image glowCoolCompact1x;
    juce::Image glowCoolCompact2x;

    bool loaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlowAssets)
};

} // namespace blindcard
