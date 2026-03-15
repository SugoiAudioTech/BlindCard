/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include "BlindCardManager.h"
#include <map>

namespace blindcard
{

class BlindCardSessionRegistry
{
public:
    static BlindCardSessionRegistry& getInstance();

    SharedBlindCardManager acquireManager(const juce::String& preferredSessionId,
                                          juce::String& assignedSessionId);

private:
    BlindCardSessionRegistry() = default;

    void cleanupExpiredSessions();
    SharedBlindCardManager createSession(const juce::String& sessionId);

    juce::CriticalSection lock;
    std::map<juce::String, std::weak_ptr<BlindCardManager>> sessions;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlindCardSessionRegistry)
};

} // namespace blindcard
