#pragma once

#include "PluginProcessor.h"
#include "UI/CardComponent.h"
#include "UI/DebugPanel.h"
#include <vector>

class BlindCardEditor final : public juce::AudioProcessorEditor,
                               public juce::ChangeListener,
                               public juce::Timer
{
public:
    explicit BlindCardEditor (BlindCardProcessor&);
    ~BlindCardEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void timerCallback() override;

private:
    BlindCardProcessor& processorRef;

    // 控制列
    juce::Slider roundsSlider;
    juce::Label roundsLabel;
    juce::TextButton shuffleButton { "Shuffle" };
    juce::TextButton nextRoundButton { "Next Round" };
    juce::TextButton revealButton { "Reveal" };
    juce::TextButton resetButton { "New Game" };
    juce::ToggleButton bypassButton { "Bypass All" };
    juce::TextButton measureButton { "Measure (5s)" };
    juce::Label measureLabel;
    juce::TextButton modeButton { "Mode: Stars" };

    // 卡牌
    juce::OwnedArray<blindcard::CardComponent> cardComponents;

    // 選中狀態管理（用陣列存，預留多選擴充）
    // MVP: 最多 1 主選 + 1 副選
    std::vector<int> selectedIndices;  // 存放選中的 card id
    static constexpr int MaxSelections = 2;  // MVP 限制

    // 狀態列
    juce::Label statusLabel;

    // 筆記區
    juce::TextEditor noteEditor;
    int selectedCardForNote = -1;

    // Q&A 模式 UI
    juce::Label questionLabel;
    juce::Label progressLabel;
    juce::Label resultLabel;
    juce::TextButton nextQuestionButton;
    juce::TextButton playAgainButton;

    // Debug Panel
    blindcard::DebugPanel debugPanel;
    bool debugPanelVisible = false;

    // 自動截圖功能
    void startAutoScreenshot();
    void performScreenshotStep();
    void saveScreenshot (const juce::String& filename);
    int screenshotStep = -1;  // -1 = 未啟動, 0-3 = 進行中
    juce::File screenshotDir;
    static constexpr int screenshotCardCounts[] = { 4, 6, 7, 8 };

    void updateUI();
    void updateQAUI();
    void updateCardSelectionStates();  // 更新所有卡牌的選中狀態
    void onCardClicked (int cardId);
    void onCardRated (int cardId, int stars);
    void onGainChanged (int cardId, float gainDb);
    void onQASubmit (int cardId);

    // 選中狀態管理
    void selectCard (int cardId, bool asPrimary);
    void deselectCard (int cardId);
    void clearAllSelections();
    bool isCardSelected (int cardId) const;
    blindcard::SelectionState getCardSelectionState (int cardId) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardEditor)
};
