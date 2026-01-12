#pragma once

#include "PluginProcessor.h"
#include "UI/CardComponent.h"

class BlindCardEditor final : public juce::AudioProcessorEditor,
                               public juce::ChangeListener
{
public:
    explicit BlindCardEditor (BlindCardProcessor&);
    ~BlindCardEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    BlindCardProcessor& processorRef;

    // 控制列
    juce::Slider roundsSlider;
    juce::Label roundsLabel;
    juce::TextButton shuffleButton { "Shuffle" };
    juce::TextButton nextRoundButton { "Next Round" };
    juce::TextButton revealButton { "Reveal" };
    juce::TextButton resetButton { "New Game" };

    // 卡牌
    juce::OwnedArray<blindcard::CardComponent> cardComponents;

    // 狀態列
    juce::Label statusLabel;

    // 筆記區
    juce::TextEditor noteEditor;
    int selectedCardForNote = -1;

    void updateUI();
    void onCardClicked (int cardId);
    void onCardRated (int cardId, int stars);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardEditor)
};
