/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    StandalonePresetManager.h
    Created: 2025-01-21
    Author:  BlindCard

    Preset management for Standalone mode.
    Saves and loads audio file configurations.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <functional>

namespace BlindCard
{

//==============================================================================
/**
 * Manages presets for Standalone mode.
 * Presets store which audio files are loaded on which cards.
 */
class StandalonePresetManager
{
public:
    //==========================================================================
    /** Data for a single card slot in a preset */
    struct SlotData
    {
        int cardIndex = -1;
        juce::String filePath;
        bool hasFile = false;
    };

    /** Complete preset data */
    struct PresetData
    {
        juce::String name;
        int cardCount = 2;
        std::vector<SlotData> slots;
    };

    //==========================================================================
    /** Constructor */
    StandalonePresetManager();

    /** Destructor */
    ~StandalonePresetManager() = default;

    //==========================================================================
    // Preset management

    /**
     * Save current configuration as a preset.
     * @param name The preset name
     * @param data The preset data to save
     * @return true if saved successfully
     */
    bool savePreset(const juce::String& name, const PresetData& data);

    /**
     * Load a preset by name.
     * @param name The preset name to load
     * @param data Output: the loaded preset data
     * @return true if loaded successfully
     */
    bool loadPreset(const juce::String& name, PresetData& data);

    /**
     * Delete a preset by name.
     * @param name The preset name to delete
     * @return true if deleted successfully
     */
    bool deletePreset(const juce::String& name);

    /**
     * Rename a preset.
     * @param oldName The current preset name
     * @param newName The new preset name
     * @return true if renamed successfully
     */
    bool renamePreset(const juce::String& oldName, const juce::String& newName);

    /**
     * Get list of available preset names.
     * @return Array of preset names
     */
    juce::StringArray getPresetNames() const;

    /**
     * Check if a preset exists.
     * @param name The preset name to check
     * @return true if the preset exists
     */
    bool presetExists(const juce::String& name) const;

    //==========================================================================
    // Paths

    /** Get the presets directory */
    juce::File getPresetsDirectory() const;

private:
    //==========================================================================
    juce::File getPresetFile(const juce::String& name) const;
    void ensurePresetsDirectoryExists();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandalonePresetManager)
};

} // namespace BlindCard
