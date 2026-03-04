/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    UpdateChecker.h
    Created: 2026-02-21
    Author:  BlindCard

    Singleton that checks the Sugoi manifest (sugoiaudio.com/api/version.json)
    for newer versions. Results are cached locally for 24 hours.

  ==============================================================================
*/

#pragma once

#include <juce_events/juce_events.h>
#include <juce_core/juce_core.h>

namespace BlindCard
{

class UpdateChecker : public juce::ChangeBroadcaster
{
public:
    static UpdateChecker& getInstance();

    void checkForUpdate();

    bool isUpdateAvailable() const { return updateAvailable.load(); }
    bool hasCheckCompleted() const { return checkCompleted.load(); }

    juce::String getLatestVersion() const;
    juce::String getReleaseURL() const;
    juce::String getChangelog() const;

    static juce::String getCurrentVersion();

private:
    UpdateChecker();
    ~UpdateChecker() override;

    std::atomic<bool> checkCompleted { false };
    std::atomic<bool> updateAvailable { false };

    juce::CriticalSection resultLock;
    juce::String latestVersion;
    juce::String releaseUrl;
    juce::String changelog;
    juce::int64 lastCheckTimestamp_ = 0;

    class CheckThread;
    std::unique_ptr<CheckThread> checkThread;

    // Cache system
    bool shouldCheck() const;
    void loadCache();
    void saveCache();
    static juce::File getCacheFile();

    static bool isNewerVersion(const juce::String& remote, const juce::String& local);

    // Manifest constants
    static constexpr const char* MANIFEST_URL = "https://sugoiaudio.com/api/version.json";
    static constexpr const char* PRODUCT_KEY = "blindcard";
    static constexpr juce::int64 CACHE_DURATION_MS = 24 * 60 * 60 * 1000; // 24 hours

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UpdateChecker)
};

} // namespace BlindCard
