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

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    // 公開給 Editor 使用
    blindcard::BlindCardManager& getManager() { return *manager; }
    int getCardId() const { return cardId; }
    bool isRegistered() const { return cardId >= 0; }

private:
    blindcard::SharedBlindCardManager manager;
    int cardId = -1;
    std::atomic<bool> shouldMute { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardProcessor)
};
