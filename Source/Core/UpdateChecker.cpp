/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    UpdateChecker.cpp
    Created: 2026-02-21
    Author:  BlindCard

    Background thread queries GitHub Releases API and compares semver.

  ==============================================================================
*/

#include "UpdateChecker.h"
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace BlindCard
{

//==============================================================================
class UpdateChecker::CheckThread : public juce::Thread
{
public:
    CheckThread(UpdateChecker& o) : Thread("BlindCard-UpdateCheck"), owner(o) {}

    void run() override
    {
        juce::URL url("https://api.github.com/repos/SugoiAudioTech/BlindCard/releases/latest");

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(5000)
            .withExtraHeaders("Accept: application/vnd.github.v3+json");

        auto stream = url.createInputStream(options);

        if (stream == nullptr || threadShouldExit())
            return;

        auto response = stream->readEntireStreamAsString();

        if (threadShouldExit() || response.isEmpty())
            return;

        auto json = juce::JSON::parse(response);
        auto tagName = json.getProperty("tag_name", "").toString();
        auto htmlUrl = json.getProperty("html_url", "").toString();

        if (tagName.isEmpty())
            return;

        // Strip leading 'v' or 'V' (e.g., "v1.0.1" -> "1.0.1")
        if (tagName.startsWithChar('v') || tagName.startsWithChar('V'))
            tagName = tagName.substring(1);

        auto currentVer = UpdateChecker::getCurrentVersion();
        bool newer = UpdateChecker::isNewerVersion(tagName, currentVer);

        juce::MessageManager::callAsync([safeOwner = &owner, tagName, newer]()
        {
            {
                juce::ScopedLock sl(safeOwner->resultLock);
                safeOwner->latestVersion = tagName;
                safeOwner->releaseUrl = "https://sugoiaudio.com/downloads/blindcard.html";
            }
            safeOwner->updateAvailable.store(newer);
            safeOwner->checkCompleted.store(true);
            safeOwner->sendChangeMessage();
        });
    }

private:
    UpdateChecker& owner;
};

//==============================================================================
UpdateChecker& UpdateChecker::getInstance()
{
    static UpdateChecker instance;
    return instance;
}

UpdateChecker::~UpdateChecker()
{
    if (checkThread)
        checkThread->stopThread(2000);
}

void UpdateChecker::checkForUpdate()
{
    bool expected = false;
    if (!hasChecked.compare_exchange_strong(expected, true))
        return;

    checkThread = std::make_unique<CheckThread>(*this);
    checkThread->startThread(juce::Thread::Priority::low);
}

juce::String UpdateChecker::getLatestVersion() const
{
    juce::ScopedLock sl(resultLock);
    return latestVersion;
}

juce::String UpdateChecker::getReleaseURL() const
{
    juce::ScopedLock sl(resultLock);
    return releaseUrl;
}

juce::String UpdateChecker::getCurrentVersion()
{
    return JucePlugin_VersionString;
}

bool UpdateChecker::isNewerVersion(const juce::String& remote, const juce::String& local)
{
    auto parseSemver = [](const juce::String& ver) -> std::tuple<int, int, int>
    {
        auto parts = juce::StringArray::fromTokens(ver, ".", "");
        int major = parts.size() > 0 ? parts[0].getIntValue() : 0;
        int minor = parts.size() > 1 ? parts[1].getIntValue() : 0;
        int patch = parts.size() > 2 ? parts[2].getIntValue() : 0;
        return { major, minor, patch };
    };

    auto [rMaj, rMin, rPat] = parseSemver(remote);
    auto [lMaj, lMin, lPat] = parseSemver(local);

    if (rMaj != lMaj) return rMaj > lMaj;
    if (rMin != lMin) return rMin > lMin;
    return rPat > lPat;
}

} // namespace BlindCard
