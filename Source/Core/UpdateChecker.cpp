/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    UpdateChecker.cpp
    Created: 2026-02-21
    Author:  BlindCard

    Background thread queries the Sugoi manifest and compares semver.
    Results are cached locally for 24 hours.

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
        juce::URL url(MANIFEST_URL);

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(5000);

        auto stream = url.createInputStream(options);

        if (stream == nullptr || threadShouldExit())
            return;

        auto response = stream->readEntireStreamAsString();

        if (threadShouldExit() || response.isEmpty())
            return;

        auto json = juce::JSON::parse(response);

        if (!json.isObject())
            return;

        auto product = json.getProperty(PRODUCT_KEY, juce::var());

        if (!product.isObject())
            return;

        auto tagName = product.getProperty("latest", "").toString();
        auto htmlUrl = product.getProperty("url", "").toString();
        auto changelogText = product.getProperty("changelog", "").toString();

        if (tagName.isEmpty())
            return;

        auto currentVer = UpdateChecker::getCurrentVersion();
        bool newer = UpdateChecker::isNewerVersion(tagName, currentVer);

        DBG("[UpdateChecker] Current: " + currentVer +
            " Latest: " + tagName +
            " Update: " + juce::String(newer ? "YES" : "NO"));

        juce::MessageManager::callAsync([safeOwner = &owner, tagName, htmlUrl, changelogText, newer]()
        {
            {
                juce::ScopedLock sl(safeOwner->resultLock);
                safeOwner->latestVersion = tagName;
                safeOwner->releaseUrl = htmlUrl;
                safeOwner->changelog = changelogText;
            }
            safeOwner->updateAvailable.store(newer);
            safeOwner->checkCompleted.store(true);
            safeOwner->lastCheckTimestamp_ = juce::Time::currentTimeMillis();
            safeOwner->saveCache();
            safeOwner->sendChangeMessage();
        });
    }

private:
    UpdateChecker& owner;
};

//==============================================================================
UpdateChecker::UpdateChecker()
{
    loadCache();
}

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
    if (!shouldCheck())
    {
        // Cache is still valid — notify listeners with cached data
        if (checkCompleted.load())
            sendChangeMessage();
        return;
    }

    // Prevent launching multiple threads
    if (checkThread && checkThread->isThreadRunning())
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

juce::String UpdateChecker::getChangelog() const
{
    juce::ScopedLock sl(resultLock);
    return changelog;
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

//==============================================================================
// Cache system

bool UpdateChecker::shouldCheck() const
{
    if (lastCheckTimestamp_ == 0) return true;

    auto now = juce::Time::currentTimeMillis();
    return (now - lastCheckTimestamp_) > CACHE_DURATION_MS;
}

juce::File UpdateChecker::getCacheFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("BlindCard")
               .getChildFile("update-cache.json");
}

void UpdateChecker::loadCache()
{
    auto cacheFile = getCacheFile();
    if (!cacheFile.existsAsFile()) return;

    auto content = cacheFile.loadFileAsString();
    auto parsed = juce::JSON::parse(content);

    if (!parsed.isObject()) return;

    lastCheckTimestamp_ = static_cast<juce::int64>(parsed.getProperty("lastCheck", 0));

    juce::ScopedLock sl(resultLock);
    latestVersion = parsed.getProperty("latestVersion", "").toString();
    releaseUrl = parsed.getProperty("downloadUrl", "").toString();
    changelog = parsed.getProperty("changelog", "").toString();

    bool cached = static_cast<bool>(parsed.getProperty("updateAvailable", false));
    updateAvailable.store(cached);
    checkCompleted.store(true);
}

void UpdateChecker::saveCache()
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("lastCheck", lastCheckTimestamp_);

    {
        juce::ScopedLock sl(resultLock);
        obj->setProperty("updateAvailable", updateAvailable.load());
        obj->setProperty("latestVersion", latestVersion);
        obj->setProperty("downloadUrl", releaseUrl);
        obj->setProperty("changelog", changelog);
    }

    auto cacheFile = getCacheFile();
    cacheFile.getParentDirectory().createDirectory();
    cacheFile.replaceWithText(juce::JSON::toString(juce::var(obj)));
}

} // namespace BlindCard
