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

    // TrackProperties - 從 DAW 取得軌道名稱
    void updateTrackProperties (const TrackProperties& properties) override;

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    // 公開給 Editor 使用
    blindcard::BlindCardManager& getManager() { return *manager; }
    int getCardId() const { return cardId; }
    bool isRegistered() const { return cardId >= 0; }

    // 當卡牌重新編號時由 Manager 呼叫
    void updateCardId (int newId) { cardId = newId; }

    // Level Matching
    void startMeasurement (float durationSeconds = 5.0f);
    void stopMeasurement();
    bool isMeasuring() const { return measuring.load(); }
    float getMeasurementProgress() const;

    // 即時 RMS（給 UI 顯示用）
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

    // 即時 RMS 計算
    std::atomic<float> currentRMSdB { -100.0f };
    float rmsSmoothed = 0.0f;
    static constexpr float kRMSSmoothingCoeff = 0.1f;  // 平滑係數

    // 音軌切換淡入淡出（防止爆音）
    float muteGain = 0.0f;          // 當前靜音增益 (0=靜音, 1=正常)
    float targetMuteGain = 0.0f;    // 目標靜音增益
    float muteGainStep = 0.0f;      // 每個 sample 的增益變化量
    static constexpr float kFadeTimeMs = 10.0f;  // 10ms 淡入淡出時間

    // 緩存 DAW 傳來的軌道名稱（可能在 prepareToPlay 之前收到）
    juce::String cachedTrackName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardProcessor)
};
