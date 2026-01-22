/*
  ==============================================================================

    StandalonePresetManager.cpp
    Created: 2025-01-21
    Author:  BlindCard

  ==============================================================================
*/

#include "StandalonePresetManager.h"

namespace BlindCard
{

//==============================================================================
StandalonePresetManager::StandalonePresetManager()
{
    ensurePresetsDirectoryExists();
}

//==============================================================================
bool StandalonePresetManager::savePreset(const juce::String& name, const PresetData& data)
{
    if (name.isEmpty())
        return false;

    auto file = getPresetFile(name);

    // Create JSON object
    juce::DynamicObject::Ptr preset = new juce::DynamicObject();
    preset->setProperty("name", data.name);
    preset->setProperty("cardCount", data.cardCount);

    // Create slots array
    juce::Array<juce::var> slotsArray;
    for (const auto& slot : data.slots)
    {
        juce::DynamicObject::Ptr slotObj = new juce::DynamicObject();
        slotObj->setProperty("cardIndex", slot.cardIndex);
        slotObj->setProperty("filePath", slot.filePath);
        slotObj->setProperty("hasFile", slot.hasFile);
        slotsArray.add(juce::var(slotObj.get()));
    }
    preset->setProperty("slots", slotsArray);

    // Write to file
    juce::var presetVar(preset.get());
    juce::String jsonString = juce::JSON::toString(presetVar, true);

    return file.replaceWithText(jsonString);
}

bool StandalonePresetManager::loadPreset(const juce::String& name, PresetData& data)
{
    auto file = getPresetFile(name);

    if (!file.existsAsFile())
        return false;

    // Parse JSON
    juce::String jsonString = file.loadFileAsString();
    juce::var parsed = juce::JSON::parse(jsonString);

    if (!parsed.isObject())
        return false;

    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr)
        return false;

    // Read data
    data.name = obj->getProperty("name").toString();
    data.cardCount = static_cast<int>(obj->getProperty("cardCount"));

    // Read slots
    data.slots.clear();
    auto slotsVar = obj->getProperty("slots");
    if (slotsVar.isArray())
    {
        for (int i = 0; i < slotsVar.size(); ++i)
        {
            auto slotVar = slotsVar[i];
            if (slotVar.isObject())
            {
                auto* slotObj = slotVar.getDynamicObject();
                if (slotObj != nullptr)
                {
                    SlotData slot;
                    slot.cardIndex = static_cast<int>(slotObj->getProperty("cardIndex"));
                    slot.filePath = slotObj->getProperty("filePath").toString();
                    slot.hasFile = static_cast<bool>(slotObj->getProperty("hasFile"));
                    data.slots.push_back(slot);
                }
            }
        }
    }

    return true;
}

bool StandalonePresetManager::deletePreset(const juce::String& name)
{
    auto file = getPresetFile(name);

    if (!file.existsAsFile())
        return false;

    return file.deleteFile();
}

bool StandalonePresetManager::renamePreset(const juce::String& oldName, const juce::String& newName)
{
    if (oldName.isEmpty() || newName.isEmpty())
        return false;

    auto oldFile = getPresetFile(oldName);
    auto newFile = getPresetFile(newName);

    if (!oldFile.existsAsFile() || newFile.existsAsFile())
        return false;

    // Load, update name, save with new name, delete old
    PresetData data;
    if (!loadPreset(oldName, data))
        return false;

    data.name = newName;

    if (!savePreset(newName, data))
        return false;

    return oldFile.deleteFile();
}

juce::StringArray StandalonePresetManager::getPresetNames() const
{
    juce::StringArray names;

    auto dir = getPresetsDirectory();
    if (dir.isDirectory())
    {
        auto files = dir.findChildFiles(juce::File::findFiles, false, "*.json");
        for (const auto& file : files)
        {
            names.add(file.getFileNameWithoutExtension());
        }
    }

    names.sort(true);  // Case-insensitive sort
    return names;
}

bool StandalonePresetManager::presetExists(const juce::String& name) const
{
    return getPresetFile(name).existsAsFile();
}

//==============================================================================
juce::File StandalonePresetManager::getPresetsDirectory() const
{
    // Store presets in ~/Library/Application Support/BlindCard/Presets/
    auto appDataDir = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);

#if JUCE_MAC
    appDataDir = appDataDir.getChildFile("Application Support");
#endif

    return appDataDir.getChildFile("BlindCard").getChildFile("Presets");
}

juce::File StandalonePresetManager::getPresetFile(const juce::String& name) const
{
    // Sanitize filename
    juce::String safeName = name.removeCharacters("\\/:*?\"<>|");
    return getPresetsDirectory().getChildFile(safeName + ".json");
}

void StandalonePresetManager::ensurePresetsDirectoryExists()
{
    auto dir = getPresetsDirectory();
    if (!dir.isDirectory())
    {
        dir.createDirectory();
    }
}

} // namespace BlindCard
