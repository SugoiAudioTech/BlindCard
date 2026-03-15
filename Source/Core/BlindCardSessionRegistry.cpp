/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "BlindCardSessionRegistry.h"

namespace blindcard
{

BlindCardSessionRegistry& BlindCardSessionRegistry::getInstance()
{
    static BlindCardSessionRegistry instance;
    return instance;
}

SharedBlindCardManager BlindCardSessionRegistry::acquireManager(const juce::String& preferredSessionId,
                                                                juce::String& assignedSessionId)
{
    juce::ScopedLock sl(lock);
    cleanupExpiredSessions();

    if (preferredSessionId.isNotEmpty())
    {
        assignedSessionId = preferredSessionId;

        if (auto existing = sessions[preferredSessionId].lock())
            return existing;

        return createSession(preferredSessionId);
    }

    for (auto& [sessionId, weakManager] : sessions)
    {
        if (auto manager = weakManager.lock())
        {
            if (manager->canJoinGame())
            {
                assignedSessionId = sessionId;
                return manager;
            }
        }
    }

    assignedSessionId = juce::Uuid().toString();
    return createSession(assignedSessionId);
}

void BlindCardSessionRegistry::cleanupExpiredSessions()
{
    for (auto it = sessions.begin(); it != sessions.end();)
    {
        if (it->second.expired())
            it = sessions.erase(it);
        else
            ++it;
    }
}

SharedBlindCardManager BlindCardSessionRegistry::createSession(const juce::String& sessionId)
{
    auto manager = std::make_shared<BlindCardManager>();
    sessions[sessionId] = manager;
    return manager;
}

} // namespace blindcard
