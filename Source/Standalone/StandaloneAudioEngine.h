/*
  ==============================================================================

    StandaloneAudioEngine.h
    Created: 2025-01-21
    Author:  BlindCard

    Audio engine for Standalone mode.
    Manages multiple audio files with sample-accurate synchronized playback.

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <vector>
#include <memory>
#include <functional>

namespace BlindCard
{

//==============================================================================
/**
 * StandaloneAudioEngine manages audio file playback for Standalone mode.
 *
 * Features:
 * - Load individual audio files per card slot
 * - Sample-accurate synchronized playback
 * - Seamless switching between cards (changes audio source without changing position)
 * - Supports WAV, MP3, AIFF formats
 *
 * Usage:
 *   StandaloneAudioEngine engine;
 *   engine.prepareToPlay(44100.0, 512);
 *   engine.loadFile(0, File("/path/to/audio.wav"));
 *   engine.play();
 *   engine.switchToCard(1);  // Seamless switch
 */
class StandaloneAudioEngine : public juce::AudioSource,
                               public juce::ChangeBroadcaster
{
public:
    //==========================================================================
    /** Audio slot representing one card's audio file */
    struct AudioSlot
    {
        int cardId = -1;
        juce::File file;
        std::unique_ptr<juce::AudioFormatReader> reader;
        double lengthInSeconds = 0.0;
        juce::int64 lengthInSamples = 0;
        bool isLoaded = false;
        juce::String errorMessage;
    };

    //==========================================================================
    /** Constructor */
    StandaloneAudioEngine();

    /** Destructor */
    ~StandaloneAudioEngine();

    //==========================================================================
    // Initialization

    /**
     * Initialize audio engine and start audio device.
     * Call this after construction.
     */
    void initialize();

    //==========================================================================
    // File management

    /**
     * Load an audio file into a card slot.
     * @param cardId The card index (0-7)
     * @param file The audio file to load
     * @return true if loaded successfully
     */
    bool loadFile(int cardId, const juce::File& file);

    /**
     * Unload the audio file from a card slot.
     * @param cardId The card index to unload
     */
    void unloadFile(int cardId);

    /**
     * Check if a file format is supported.
     * @param file The file to check
     * @return true if the format is supported (WAV, MP3, AIFF)
     */
    bool isFormatSupported(const juce::File& file) const;

    /**
     * Get the audio slot for a card.
     * @param cardId The card index
     * @return Pointer to the slot, or nullptr if invalid
     */
    const AudioSlot* getSlot(int cardId) const;

    /**
     * Check if any audio is loaded.
     * @return true if at least one slot has audio
     */
    bool hasAnyAudioLoaded() const;

    //==========================================================================
    // Transport controls

    /** Start playback */
    void play();

    /** Pause playback */
    void pause();

    /** Toggle play/pause */
    void togglePlayPause();

    /** Check if currently playing */
    bool isPlaying() const { return playing; }

    /**
     * Seek to a position.
     * @param positionSeconds The position in seconds
     */
    void seek(double positionSeconds);

    /**
     * Skip forward or backward.
     * @param deltaSeconds Seconds to skip (negative for backward)
     */
    void skip(double deltaSeconds);

    //==========================================================================
    // Card switching

    /**
     * Switch to a different card's audio (seamless, no position change).
     * @param cardId The card to switch to
     */
    void switchToCard(int cardId);

    /** Get the currently active card ID */
    int getActiveCardId() const { return activeCardId; }

    //==========================================================================
    // Position and length

    /** Get the current playback position in seconds */
    double getCurrentPositionSeconds() const;

    /** Get the current playback position in samples */
    juce::int64 getCurrentPositionSamples() const { return playheadPosition; }

    /** Get the total length in seconds (longest loaded file) */
    double getTotalLengthSeconds() const;

    /** Get the total length in samples */
    juce::int64 getTotalLengthSamples() const { return totalLengthSamples; }

    /** Get the current RMS level in dB (smoothed) */
    float getCurrentRMSdB() const { return currentRMSdB.load(); }

    //==========================================================================
    // AudioSource interface

    /** Prepare to play - called by audio device */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /** Release resources when playback stops */
    void releaseResources() override;

    /** Get the next audio block - called by audio device */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    //==========================================================================
    // Audio device

    /** Start audio device output */
    void startAudioDevice();

    /** Stop audio device output */
    void stopAudioDevice();

    //==========================================================================
    // Callbacks

    /** Called when playback position changes (for UI updates) */
    std::function<void(double positionSeconds)> onPositionChanged;

    /** Called when playback state changes */
    std::function<void(bool isPlaying)> onPlaybackStateChanged;

    /** Called when a file is loaded or unloaded */
    std::function<void(int cardId, bool loaded)> onFileStateChanged;

    //==========================================================================
    // Constants

    static constexpr int kMaxSlots = 8;
    static constexpr double kSkipSeconds = 5.0;
    static constexpr juce::int64 kMaxFileSizeBytes = 1024 * 1024 * 1024; // 1GB

private:
    //==========================================================================
    juce::AudioFormatManager formatManager;
    std::vector<AudioSlot> slots;

    // Playback state
    std::atomic<bool> playing { false };
    std::atomic<int> activeCardId { 0 };
    std::atomic<juce::int64> playheadPosition { 0 };
    juce::int64 totalLengthSamples = 0;

    // RMS level metering
    std::atomic<float> currentRMSdB { -100.0f };
    static constexpr float kRMSSmoothingCoeff = 0.3f;  // Smoothing factor for RMS

    // Audio settings
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // Resampling buffer for mismatched sample rates
    juce::AudioBuffer<float> resampleBuffer;

    // Audio device output
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer sourcePlayer;
    bool deviceInitialized = false;

    //==========================================================================
    // Helpers
    void updateTotalLength();
    void notifyPositionChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneAudioEngine)
};

} // namespace BlindCard
