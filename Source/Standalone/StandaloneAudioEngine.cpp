/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    StandaloneAudioEngine.cpp
    Created: 2025-01-21
    Author:  BlindCard

  ==============================================================================
*/

#include "StandaloneAudioEngine.h"

namespace BlindCard
{

//==============================================================================
StandaloneAudioEngine::StandaloneAudioEngine()
{
    // Register audio formats: WAV, AIFF, MP3
    formatManager.registerBasicFormats();

    // Initialize slots
    slots.resize(kMaxSlots);
    for (int i = 0; i < kMaxSlots; ++i)
        slots[static_cast<size_t>(i)].cardId = i;
}

StandaloneAudioEngine::~StandaloneAudioEngine()
{
    stopAudioDevice();
    releaseResources();
}

//==============================================================================
void StandaloneAudioEngine::initialize()
{
    if (deviceInitialized)
        return;

    // Initialize audio device with default settings
    auto result = deviceManager.initialiseWithDefaultDevices(0, 2); // 0 inputs, 2 outputs (stereo)

    if (result.isNotEmpty())
    {
        DBG("Audio device initialization failed: " + result);
        return;
    }

    // Connect this audio source to the source player
    sourcePlayer.setSource(this);

    // Add the source player as a callback to the audio device
    deviceManager.addAudioCallback(&sourcePlayer);

    deviceInitialized = true;
    DBG("Audio device initialized successfully");
}

void StandaloneAudioEngine::startAudioDevice()
{
    if (!deviceInitialized)
        initialize();
}

void StandaloneAudioEngine::stopAudioDevice()
{
    if (deviceInitialized)
    {
        deviceManager.removeAudioCallback(&sourcePlayer);
        sourcePlayer.setSource(nullptr);
        deviceManager.closeAudioDevice();
        deviceInitialized = false;
    }
}

//==============================================================================
void StandaloneAudioEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;

    // Allocate resampling buffer
    resampleBuffer.setSize(2, samplesPerBlockExpected * 4); // Extra space for resampling

    // Allocate crossfade buffer
    crossfadeBuffer.setSize(2, samplesPerBlockExpected);

    updateTotalLength();
}

void StandaloneAudioEngine::releaseResources()
{
    playing = false;
    playheadPosition.store(0);
    previousCardId.store(-1);
    crossfadeProgress = 1.0f;
    currentRMSdB.store(-100.0f);

    resampleBuffer.setSize(0, 0);
    crossfadeBuffer.setSize(0, 0);
}

//==============================================================================
bool StandaloneAudioEngine::loadFile(int cardId, const juce::File& file)
{
    if (cardId < 0 || cardId >= kMaxSlots)
        return false;

    {
        juce::ScopedLock sl(slotsLock);
        if (!file.existsAsFile())
        {
            slots[static_cast<size_t>(cardId)].errorMessage = "File not found";
            return false;
        }

        // Check file size
        if (file.getSize() > kMaxFileSizeBytes)
        {
            slots[static_cast<size_t>(cardId)].errorMessage = "File too large (max 1GB)";
            return false;
        }

        // Check format
        if (!isFormatSupported(file))
        {
            slots[static_cast<size_t>(cardId)].errorMessage = "Unsupported format (use WAV/MP3/AIFF)";
            return false;
        }
    }

    // Create reader
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
    {
        slots[static_cast<size_t>(cardId)].errorMessage = "Cannot read file";
        return false;
    }

    // Unload previous file if any
    {
        juce::ScopedLock sl(slotsLock);

        auto& slot = slots[static_cast<size_t>(cardId)];
        slot.reader.reset(reader);
        slot.file = file;
        slot.lengthInSamples = reader->lengthInSamples;
        slot.lengthInSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
        slot.isLoaded = true;
        slot.errorMessage.clear();

        updateTotalLengthLocked();
    }

    // Notify listeners
    if (onFileStateChanged)
        onFileStateChanged(cardId, true);

    sendChangeMessage();
    return true;
}

void StandaloneAudioEngine::unloadFile(int cardId)
{
    if (cardId < 0 || cardId >= kMaxSlots)
        return;

    {
        juce::ScopedLock sl(slotsLock);

        auto& slot = slots[static_cast<size_t>(cardId)];
        if (!slot.isLoaded)
            return;

        slot.reader.reset();
        slot.file = juce::File();
        slot.lengthInSamples = 0;
        slot.lengthInSeconds = 0.0;
        slot.isLoaded = false;
        slot.errorMessage.clear();

        updateTotalLengthLocked();
    }

    // Notify listeners
    if (onFileStateChanged)
        onFileStateChanged(cardId, false);

    sendChangeMessage();
}

bool StandaloneAudioEngine::isFormatSupported(const juce::File& file) const
{
    auto extension = file.getFileExtension().toLowerCase();
    return extension == ".wav" || extension == ".mp3" || extension == ".aiff" || extension == ".aif";
}

std::optional<StandaloneAudioEngine::AudioSlotInfo> StandaloneAudioEngine::getSlotInfo(int cardId) const
{
    if (cardId < 0 || cardId >= kMaxSlots)
        return std::nullopt;

    juce::ScopedLock sl(slotsLock);
    const auto& slot = slots[static_cast<size_t>(cardId)];

    return AudioSlotInfo {
        slot.cardId,
        slot.file,
        slot.lengthInSeconds,
        slot.lengthInSamples,
        slot.isLoaded,
        slot.errorMessage
    };
}

bool StandaloneAudioEngine::hasAnyAudioLoaded() const
{
    juce::ScopedLock sl(slotsLock);
    for (const auto& slot : slots)
    {
        if (slot.isLoaded)
            return true;
    }
    return false;
}

//==============================================================================
void StandaloneAudioEngine::play()
{
    if (!hasAnyAudioLoaded())
        return;

    playing = true;

    if (onPlaybackStateChanged)
        onPlaybackStateChanged(true);

    sendChangeMessage();
}

void StandaloneAudioEngine::pause()
{
    playing = false;

    if (onPlaybackStateChanged)
        onPlaybackStateChanged(false);

    sendChangeMessage();
}

void StandaloneAudioEngine::togglePlayPause()
{
    if (playing)
        pause();
    else
        play();
}

void StandaloneAudioEngine::seek(double positionSeconds)
{
    auto newPosition = static_cast<juce::int64>(positionSeconds * currentSampleRate);
    newPosition = juce::jlimit<juce::int64>(0, totalLengthSamples.load(), newPosition);
    playheadPosition = newPosition;

    notifyPositionChanged();
    sendChangeMessage();
}

void StandaloneAudioEngine::skip(double deltaSeconds)
{
    seek(getCurrentPositionSeconds() + deltaSeconds);
}

//==============================================================================
void StandaloneAudioEngine::switchToCard(int cardId)
{
    if (cardId < 0 || cardId >= kMaxSlots)
        return;

    {
        juce::ScopedLock sl(slotsLock);
        if (!slots[static_cast<size_t>(cardId)].isLoaded || slots[static_cast<size_t>(cardId)].reader == nullptr)
            return;
    }

    int currentCard = activeCardId.load();
    if (cardId == currentCard)
        return;

    // Start crossfade: remember old card, reset progress
    previousCardId.store(currentCard);
    crossfadeProgress = 0.0f;

    // Calculate per-sample step based on crossfade time and sample rate
    float fadeMs = crossfadeTimeMs.load();
    float fadeSamples = static_cast<float>(currentSampleRate * fadeMs / 1000.0);
    crossfadeStep = (fadeSamples > 0.0f) ? 1.0f / fadeSamples : 1.0f;

    activeCardId = cardId;
    sendChangeMessage();
}

//==============================================================================
double StandaloneAudioEngine::getCurrentPositionSeconds() const
{
    return static_cast<double>(playheadPosition.load()) / currentSampleRate;
}

double StandaloneAudioEngine::getTotalLengthSeconds() const
{
    return static_cast<double>(totalLengthSamples.load()) / currentSampleRate;
}

//==============================================================================
void StandaloneAudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto& buffer = *bufferToFill.buffer;
    int numSamples = bufferToFill.numSamples;
    int startSample = bufferToFill.startSample;

    // Clear the buffer first
    bufferToFill.clearActiveBufferRegion();

    if (!playing || !hasAnyAudioLoaded())
    {
        // Reset RMS when not playing
        currentRMSdB.store(-100.0f);
        return;
    }

    int cardId = activeCardId.load();
    if (cardId < 0 || cardId >= kMaxSlots)
        return;

    juce::ScopedLock sl(slotsLock);

    const auto& slot = slots[static_cast<size_t>(cardId)];
    if (!slot.isLoaded || slot.reader == nullptr)
        return;

    juce::int64 currentPos = playheadPosition.load();

    // Check if we've reached the end
    const auto totalLength = totalLengthSamples.load();
    if (currentPos >= totalLength)
    {
        if (looping)
        {
            // Loop back to the beginning
            playheadPosition = 0;
            currentPos = 0;
        }
        else
        {
            playing = false;
            // Notify on message thread
            juce::MessageManager::callAsync([this]()
            {
                if (onPlaybackStateChanged)
                    onPlaybackStateChanged(false);
            });
            return;
        }
    }

    // Check if this slot has audio at current position
    if (currentPos >= slot.lengthInSamples)
    {
        // This file has ended, output silence
        playheadPosition = currentPos + numSamples;
        return;
    }

    // Calculate how many samples we can read
    int samplesToRead = numSamples;
    if (currentPos + samplesToRead > slot.lengthInSamples)
        samplesToRead = static_cast<int>(slot.lengthInSamples - currentPos);

    // Read audio from file
    // Handle sample rate conversion if needed
    if (std::abs(slot.reader->sampleRate - currentSampleRate) < 1.0)
    {
        // Same sample rate, read directly
        slot.reader->read(&buffer, startSample, samplesToRead, currentPos, true, true);
    }
    else
    {
        // Different sample rate - simple linear interpolation resampling
        double ratio = slot.reader->sampleRate / currentSampleRate;
        int sourceSamples = static_cast<int>(std::ceil(numSamples * ratio)) + 2;

        resampleBuffer.setSize(buffer.getNumChannels(), sourceSamples, false, false, true);
        resampleBuffer.clear();

        juce::int64 sourcePos = static_cast<juce::int64>(currentPos * ratio);
        int actualSourceSamples = std::min(sourceSamples,
            static_cast<int>(slot.lengthInSamples - sourcePos));

        if (actualSourceSamples > 0)
        {
            slot.reader->read(&resampleBuffer, 0, actualSourceSamples, sourcePos, true, true);

            // Simple linear interpolation
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* dest = buffer.getWritePointer(ch, startSample);
                auto* src = resampleBuffer.getReadPointer(std::min(ch, resampleBuffer.getNumChannels() - 1));

                for (int i = 0; i < numSamples; ++i)
                {
                    double srcIndex = i * ratio;
                    int srcI = static_cast<int>(srcIndex);
                    float frac = static_cast<float>(srcIndex - srcI);

                    if (srcI + 1 < actualSourceSamples)
                        dest[i] = src[srcI] * (1.0f - frac) + src[srcI + 1] * frac;
                    else if (srcI < actualSourceSamples)
                        dest[i] = src[srcI];
                    else
                        dest[i] = 0.0f;
                }
            }
        }
    }

    // Apply auto-gain from Level-Match if available
    if (getGainForCard)
    {
        float gainDb = getGainForCard(cardId);
        if (std::abs(gainDb) > 0.001f)
        {
            float linearGain = std::pow(10.0f, gainDb / 20.0f);
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.applyGain(ch, startSample, numSamples, linearGain);
        }
    }

    // === Crossfade mixing (if transitioning between cards) ===
    int prevCard = previousCardId.load();
    if (prevCard >= 0 && crossfadeProgress < 1.0f)
    {
        const auto& prevSlot = slots[static_cast<size_t>(prevCard)];
        if (prevSlot.isLoaded && prevSlot.reader != nullptr && currentPos < prevSlot.lengthInSamples)
        {
            // Read old card audio into crossfadeBuffer
            crossfadeBuffer.clear();
            int prevSamplesToRead = std::min(samplesToRead,
                static_cast<int>(prevSlot.lengthInSamples - currentPos));

            if (std::abs(prevSlot.reader->sampleRate - currentSampleRate) < 1.0)
            {
                prevSlot.reader->read(&crossfadeBuffer, 0, prevSamplesToRead, currentPos, true, true);
            }
            else
            {
                // Resample old card audio (same logic as main read)
                double ratio = prevSlot.reader->sampleRate / currentSampleRate;
                juce::int64 sourcePos = static_cast<juce::int64>(currentPos * ratio);
                int sourceSamples = static_cast<int>(std::ceil(prevSamplesToRead * ratio)) + 2;
                int actualSourceSamples = std::min(sourceSamples,
                    static_cast<int>(prevSlot.lengthInSamples - sourcePos));

                if (actualSourceSamples > 0)
                {
                    // Use a temporary buffer for resampling (avoid clobbering main resampleBuffer)
                    juce::AudioBuffer<float> tempResample(buffer.getNumChannels(), sourceSamples);
                    tempResample.clear();
                    prevSlot.reader->read(&tempResample, 0, actualSourceSamples, sourcePos, true, true);

                    for (int ch = 0; ch < crossfadeBuffer.getNumChannels(); ++ch)
                    {
                        auto* dest = crossfadeBuffer.getWritePointer(ch);
                        auto* src = tempResample.getReadPointer(std::min(ch, tempResample.getNumChannels() - 1));

                        for (int i = 0; i < prevSamplesToRead; ++i)
                        {
                            double srcIndex = i * ratio;
                            int srcI = static_cast<int>(srcIndex);
                            float frac = static_cast<float>(srcIndex - srcI);

                            if (srcI + 1 < actualSourceSamples)
                                dest[i] = src[srcI] * (1.0f - frac) + src[srcI + 1] * frac;
                            else if (srcI < actualSourceSamples)
                                dest[i] = src[srcI];
                        }
                    }
                }
            }

            // Apply auto-gain to previous card too
            if (getGainForCard)
            {
                float prevGainDb = getGainForCard(prevCard);
                if (std::abs(prevGainDb) > 0.001f)
                {
                    float prevGainLinear = std::pow(10.0f, prevGainDb / 20.0f);
                    for (int ch = 0; ch < crossfadeBuffer.getNumChannels(); ++ch)
                        crossfadeBuffer.applyGain(ch, 0, numSamples, prevGainLinear);
                }
            }

            // Per-sample crossfade mix
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                float* dest = buffer.getWritePointer(ch, startSample);
                const float* prev = crossfadeBuffer.getReadPointer(
                    std::min(ch, crossfadeBuffer.getNumChannels() - 1));

                float localProgress = crossfadeProgress;
                for (int i = 0; i < numSamples; ++i)
                {
                    float newGain = std::min(localProgress, 1.0f);
                    float oldGain = 1.0f - newGain;
                    dest[i] = dest[i] * newGain + prev[i] * oldGain;
                    localProgress += crossfadeStep;
                }
                // Update shared progress on first channel only
                if (ch == 0)
                    crossfadeProgress = std::min(localProgress, 1.0f);
            }
        }
        else
        {
            // Old card can't be read, finish crossfade immediately
            crossfadeProgress = 1.0f;
        }

        // Crossfade complete
        if (crossfadeProgress >= 1.0f)
            previousCardId.store(-1);
    }

    // Apply master volume
    float masterDb = masterVolumeDb.load();
    if (masterDb <= -59.9f)
    {
        // Treat as mute
        buffer.clear(startSample, numSamples);
    }
    else if (std::abs(masterDb) > 0.001f)
    {
        float masterGain = std::pow(10.0f, masterDb / 20.0f);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.applyGain(ch, startSample, numSamples, masterGain);
    }

    // Calculate RMS level
    float sumSquares = 0.0f;
    int totalSamples = 0;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const float* channelData = buffer.getReadPointer(ch, startSample);
        for (int i = 0; i < numSamples; ++i)
        {
            sumSquares += channelData[i] * channelData[i];
        }
        totalSamples += numSamples;
    }

    if (totalSamples > 0)
    {
        float rms = std::sqrt(sumSquares / static_cast<float>(totalSamples));
        float rmsDb = (rms > 0.0001f) ? 20.0f * std::log10(rms) : -100.0f;

        // Smooth the RMS value
        float prevRms = currentRMSdB.load();
        float smoothedRms = prevRms + kRMSSmoothingCoeff * (rmsDb - prevRms);
        currentRMSdB.store(smoothedRms);
    }

    // Update playhead position
    playheadPosition = currentPos + numSamples;
}

//==============================================================================
void StandaloneAudioEngine::updateTotalLength()
{
    juce::ScopedLock sl(slotsLock);
    updateTotalLengthLocked();
}

void StandaloneAudioEngine::updateTotalLengthLocked()
{
    juce::int64 newTotalLengthSamples = 0;

    for (const auto& slot : slots)
    {
        if (slot.isLoaded)
        {
            // Convert to common sample rate for comparison
            juce::int64 samplesAtCurrentRate = static_cast<juce::int64>(
                slot.lengthInSeconds * currentSampleRate);
            newTotalLengthSamples = std::max(newTotalLengthSamples, samplesAtCurrentRate);
        }
    }

    totalLengthSamples.store(newTotalLengthSamples);
}

void StandaloneAudioEngine::notifyPositionChanged()
{
    if (onPositionChanged)
        onPositionChanged(getCurrentPositionSeconds());
}

//==============================================================================
void StandaloneAudioEngine::setMasterVolume(float db)
{
    masterVolumeDb.store(juce::jlimit(-60.0f, 12.0f, db));
}

float StandaloneAudioEngine::getMasterVolume() const
{
    return masterVolumeDb.load();
}

//==============================================================================
void StandaloneAudioEngine::measureLUFSForAllSlots()
{
    // Offline LUFS measurement: read up to 10 seconds of each loaded file,
    // compute RMS, convert to LUFS-like value, report via callback.
    // This runs on the message thread (non-realtime).

    static constexpr double kMaxScanSeconds = 10.0;
    static constexpr int kScanBlockSize = 4096;

    std::vector<juce::File> filesToScan;
    filesToScan.reserve(kMaxSlots);

    {
        juce::ScopedLock sl(slotsLock);
        for (const auto& slot : slots)
        {
            filesToScan.push_back(slot.isLoaded ? slot.file : juce::File());
        }
    }

    for (int i = 0; i < kMaxSlots; ++i)
    {
        const auto& file = filesToScan[static_cast<size_t>(i)];
        if (!file.existsAsFile())
            continue;

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr)
            continue;

        double fileSampleRate = reader->sampleRate;
        juce::int64 scanSamples = static_cast<juce::int64>(
            std::min(static_cast<double>(reader->lengthInSamples) / fileSampleRate, kMaxScanSeconds) * fileSampleRate);

        if (scanSamples <= 0)
            continue;

        int numChannels = static_cast<int>(reader->numChannels);
        if (numChannels < 1) numChannels = 1;

        juce::AudioBuffer<float> scanBuffer(numChannels, kScanBlockSize);
        double sumSquares = 0.0;
        juce::int64 totalSamplesRead = 0;
        juce::int64 pos = 0;

        while (pos < scanSamples)
        {
            int toRead = static_cast<int>(std::min(static_cast<juce::int64>(kScanBlockSize),
                                                    scanSamples - pos));
            scanBuffer.clear();
            reader->read(&scanBuffer, 0, toRead, pos, true, true);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* data = scanBuffer.getReadPointer(ch);
                for (int s = 0; s < toRead; ++s)
                    sumSquares += static_cast<double>(data[s]) * static_cast<double>(data[s]);
            }

            totalSamplesRead += static_cast<juce::int64>(toRead) * numChannels;
            pos += toRead;
        }

        if (totalSamplesRead > 0)
        {
            double rms = std::sqrt(sumSquares / static_cast<double>(totalSamplesRead));
            float lufs = (rms > 0.00001) ? static_cast<float>(20.0 * std::log10(rms))
                                          : -100.0f;

            DBG("StandaloneAudioEngine: Slot " + juce::String(i)
                + " LUFS = " + juce::String(lufs, 1) + " dB");

            if (onLUFSMeasured)
                onLUFSMeasured(i, lufs);
        }
    }
}

//==============================================================================
void StandaloneAudioEngine::setCrossfadeTime(float ms)
{
    crossfadeTimeMs.store(juce::jlimit(1.0f, 100.0f, ms));
}

float StandaloneAudioEngine::getCrossfadeTime() const
{
    return crossfadeTimeMs.load();
}

} // namespace BlindCard
