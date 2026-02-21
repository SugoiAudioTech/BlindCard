/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    UpdateChecker.h
    Created: 2026-02-21
    Author:  BlindCard

    Singleton that checks GitHub Releases API for newer versions.
    Runs a single background HTTP request per process lifetime.

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

    static juce::String getCurrentVersion();

private:
    UpdateChecker() = default;
    ~UpdateChecker() override;

    std::atomic<bool> hasChecked { false };
    std::atomic<bool> checkCompleted { false };
    std::atomic<bool> updateAvailable { false };

    juce::CriticalSection resultLock;
    juce::String latestVersion;
    juce::String releaseUrl;

    class CheckThread;
    std::unique_ptr<CheckThread> checkThread;

    static bool isNewerVersion(const juce::String& remote, const juce::String& local);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UpdateChecker)
};

} // namespace BlindCard
