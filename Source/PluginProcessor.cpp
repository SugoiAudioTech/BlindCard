/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "PluginProcessor.h"
#include "UI/BlindCardEditor.h"
#include <cmath>
#include <fstream>

BlindCardProcessor::BlindCardProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Delay registration to prepareToPlay to avoid AU validation issues
}

BlindCardProcessor::~BlindCardProcessor()
{
    if (cardId >= 0)
    {
        manager->removeChangeListener (this);
        manager->unregisterInstance (this);
    }
}

void BlindCardProcessor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // Update mute state
    auto phase = manager->getPhase();
    int selectedId = manager->getSelectedCardId();
    bool bypassAll = manager->isBypassAll();

    // Update gain
    float gainDb = manager->getGainForCard (cardId);
    currentGainLinear = std::pow (10.0f, gainDb / 20.0f);

    // Bypass All mode: all tracks play
    if (bypassAll)
    {
        shouldMute = false;
        return;
    }

    // Strict solo mode: only one track plays at a time
    if (selectedId >= 0)
    {
        // When a card is selected, only the selected one plays
        shouldMute = (cardId != selectedId);
    }
    else
    {
        // When no card is selected, mute all (prevent multiple tracks playing)
        shouldMute = true;
    }
}

const juce::String BlindCardProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BlindCardProcessor::acceptsMidi() const { return false; }
bool BlindCardProcessor::producesMidi() const { return false; }
bool BlindCardProcessor::isMidiEffect() const { return false; }
double BlindCardProcessor::getTailLengthSeconds() const { return 0.0; }

int BlindCardProcessor::getNumPrograms() { return 1; }
int BlindCardProcessor::getCurrentProgram() { return 0; }
void BlindCardProcessor::setCurrentProgram (int) {}
const juce::String BlindCardProcessor::getProgramName (int) { return {}; }
void BlindCardProcessor::changeProgramName (int, const juce::String&) {}

void BlindCardProcessor::prepareToPlay (double newSampleRate, int)
{
    sampleRate = newSampleRate;

    // Calculate fade in/out gain step (change per sample)
    // e.g. 10ms @ 44100Hz = 441 samples, so step = 1.0 / 441
    float fadeSamples = static_cast<float> (sampleRate * kFadeTimeMs / 1000.0);
    muteGainStep = 1.0f / fadeSamples;

    // Only register on first prepareToPlay call
    if (cardId < 0)
    {
        // Use cached track name (if DAW already sent it)
        // If not, Manager will auto-number (Track 1, Track 2, ...)
        cardId = manager->registerInstance (this, cachedTrackName);

        if (cardId >= 0)
            manager->addChangeListener (this);
    }
}
void BlindCardProcessor::releaseResources() {}

bool BlindCardProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void BlindCardProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // LUFS measurement (for calibration) - must be before mute check, measure original input signal
    if (measuring.load())
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                sumSquared += static_cast<double> (data[i] * data[i]);
            }
        }
        sampleCount += numSamples * numChannels;

        // Check if measurement is complete
        if (sampleCount >= targetSampleCount)
        {
            measuring = false;
            // Calculate LUFS (simplified - RMS based)
            double meanSquared = sumSquared / static_cast<double> (sampleCount);
            float lufs = static_cast<float> (10.0 * std::log10 (meanSquared + 1e-10) - 0.691);
            manager->setMeasuredLUFS (cardId, lufs);
        }
    }

    // Solo logic: set target gain (0=muted, 1=normal playback)
    targetMuteGain = shouldMute.load() ? 0.0f : 1.0f;

    // Real-time RMS calculation (for UI display) - only calculate for unmuted tracks
    {
        float blockSumSquared = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
                blockSumSquared += data[i] * data[i];
        }
        float blockRMS = std::sqrt (blockSumSquared / static_cast<float> (numSamples * numChannels));

        // Exponential smoothing (avoid jumping too fast)
        rmsSmoothed = rmsSmoothed + kRMSSmoothingCoeff * (blockRMS - rmsSmoothed);

        // Convert to dB
        float rmsDb = (rmsSmoothed > 1e-10f)
            ? 20.0f * std::log10 (rmsSmoothed)
            : -100.0f;
        currentRMSdB = rmsDb;
    }

    // Apply gain: combine Level-Match compensation gain and mute fade in/out
    float gainLinear = currentGainLinear.load();

    // Apply smoothed mute gain per sample (prevent clicks when switching)
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer (ch);

        // Reset muteGain to block start value for each channel (don't reset on first channel)
        float localMuteGain = muteGain;

        for (int i = 0; i < numSamples; ++i)
        {
            // Smooth transition to target gain
            if (localMuteGain < targetMuteGain)
            {
                localMuteGain += muteGainStep;
                if (localMuteGain > targetMuteGain)
                    localMuteGain = targetMuteGain;
            }
            else if (localMuteGain > targetMuteGain)
            {
                localMuteGain -= muteGainStep;
                if (localMuteGain < targetMuteGain)
                    localMuteGain = targetMuteGain;
            }

            // Apply total gain (Level-Match * mute fade)
            data[i] *= gainLinear * localMuteGain;
        }

        // Only update member variable on first channel
        if (ch == 0)
            muteGain = localMuteGain;
    }

    // If completely muted, reset RMS
    if (muteGain < 0.0001f)
    {
        currentRMSdB = -100.0f;
        rmsSmoothed = 0.0f;
    }
}

bool BlindCardProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BlindCardProcessor::createEditor()
{
    // File logging for AU sandbox debugging
    {
        std::ofstream logFile("/tmp/blindcard_debug.log", std::ios::app);
        auto now = juce::Time::getCurrentTime().toString(true, true, true, true);
        logFile << "[" << now.toStdString() << "] createEditor() called, wrapperType="
                << static_cast<int>(wrapperType) << std::endl;
        logFile.flush();
    }

    auto* editor = new BlindCard::BlindCardEditor(*this);

    {
        std::ofstream logFile("/tmp/blindcard_debug.log", std::ios::app);
        auto now = juce::Time::getCurrentTime().toString(true, true, true, true);
        logFile << "[" << now.toStdString() << "] createEditor() returning editor "
                << (editor != nullptr ? "OK" : "NULL")
                << " size=" << editor->getWidth() << "x" << editor->getHeight() << std::endl;
        logFile.flush();
    }

    return editor;
}

void BlindCardProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save Q&A question count setting
    juce::ValueTree state ("BlindCardState");
    state.setProperty ("qaQuestionCount", manager->getQAQuestionCount(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml != nullptr)
        copyXmlToBinary (*xml, destData);
}

void BlindCardProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Load Q&A question count setting
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr)
    {
        juce::ValueTree state = juce::ValueTree::fromXml (*xml);
        if (state.isValid() && state.hasType ("BlindCardState"))
        {
            int qaCount = state.getProperty ("qaQuestionCount", 5);
            manager->setQAQuestionCount (qaCount);
        }
    }
}

void BlindCardProcessor::updateTrackProperties (const TrackProperties& properties)
{
    // When DAW sends track name
    if (properties.name.has_value())
    {
        // Cache name (in case prepareToPlay hasn't been called yet)
        cachedTrackName = *properties.name;

        // If already registered, update card name in Manager
        if (cardId >= 0)
        {
            manager->setTrackName (cardId, cachedTrackName);
        }
    }
}

void BlindCardProcessor::startMeasurement (float durationSeconds)
{
    sumSquared = 0.0;
    sampleCount = 0;
    targetSampleCount = static_cast<int64_t> (sampleRate * durationSeconds * 2.0); // stereo
    measuring = true;
}

void BlindCardProcessor::stopMeasurement()
{
    measuring = false;
}

float BlindCardProcessor::getMeasurementProgress() const
{
    if (!measuring.load() || targetSampleCount == 0)
        return 0.0f;
    return static_cast<float> (sampleCount) / static_cast<float> (targetSampleCount);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BlindCardProcessor();
}
