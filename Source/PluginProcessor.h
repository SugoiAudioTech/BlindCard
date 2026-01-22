#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/BlindCardManager.h"

class BlindCardProcessor final : public juce::AudioProcessor,
                                  public juce::ChangeListener
{
public:
    BlindCardProcessor();
    ~BlindCardProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // TrackProperties - Get track name from DAW
    void updateTrackProperties (const TrackProperties& properties) override;

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    // Public for Editor use
    blindcard::BlindCardManager& getManager() { return *manager; }
    int getCardId() const { return cardId; }
    bool isRegistered() const { return cardId >= 0; }

    // Called by Manager when card IDs are renumbered
    void updateCardId (int newId) { cardId = newId; }

    // Level Matching
    void startMeasurement (float durationSeconds = 5.0f);
    void stopMeasurement();
    bool isMeasuring() const { return measuring.load(); }
    float getMeasurementProgress() const;

    // Real-time RMS (for UI display)
    float getCurrentRMSdB() const { return currentRMSdB.load(); }

private:
    blindcard::SharedBlindCardManager manager;
    int cardId = -1;
    std::atomic<bool> shouldMute { false };

    // Level Matching
    std::atomic<bool> measuring { false };
    std::atomic<float> currentGainLinear { 1.0f };
    double sampleRate = 44100.0;
    double sumSquared = 0.0;
    int64_t sampleCount = 0;
    int64_t targetSampleCount = 0;

    // Real-time RMS calculation
    std::atomic<float> currentRMSdB { -100.0f };
    float rmsSmoothed = 0.0f;
    static constexpr float kRMSSmoothingCoeff = 0.1f;  // Smoothing coefficient

    // Track switch fade in/out (prevent clicks/pops)
    float muteGain = 0.0f;          // Current mute gain (0=muted, 1=normal)
    float targetMuteGain = 0.0f;    // Target mute gain
    float muteGainStep = 0.0f;      // Gain change per sample
    static constexpr float kFadeTimeMs = 10.0f;  // 10ms fade time

    // Cache track name from DAW (may be received before prepareToPlay)
    juce::String cachedTrackName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardProcessor)
};
