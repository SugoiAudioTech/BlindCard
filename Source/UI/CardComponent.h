#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Core/Types.h"
#include "ColorUtils.h"
#include "GlowAssets.h"

namespace blindcard
{

/**
 * 選中狀態枚舉
 * 用於區分主選/副選/未選中
 */
enum class SelectionState
{
    None,      // 未選中
    Primary,   // 主選：alpha 0.8
    Secondary  // 副選：alpha 0.38-0.40
};

/**
 * 翻牌狀態枚舉
 */
enum class FlipState
{
    FaceDown,   // 背面朝上
    Flipping,   // 翻牌動畫中
    FaceUp      // 正面朝上
};

class CardComponent final : public juce::Component,
                            public juce::Timer
{
public:
    CardComponent();

    void setCard (const CardSlot& card, int currentRound, bool isSelected, GamePhase phase, RatingMode ratingMode, const QAState* qaState = nullptr);
    void clearCard();

    std::function<void(int)> onClicked;
    std::function<void(int, int)> onRated;
    std::function<void(int, float)> onGainChanged;
    std::function<void(int, const juce::String&)> onNoteChanged;
    std::function<void(int, int)> onGuessChanged;
    std::function<void(int)> onQASubmit;

    void setTrackNames (const juce::StringArray& names);

    // 選中狀態控制（主選/副選/無）
    void setSelectionState (SelectionState state);
    SelectionState getSelectionState() const { return selectionState; }

    // 翻牌動畫控制
    void startFlip();
    void setFlipped (bool flipped, bool animate = true);
    bool isFlipped() const { return flipState == FlipState::FaceUp; }
    bool isFlipping() const { return flipState == FlipState::Flipping; }

    // 光暈閃爍控制（翻牌時觸發）
    void triggerGlowFlash();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress& key) override;

    // Timer callback 用於動畫更新
    void timerCallback() override;

    // Debug 參數存取（供 Debug Panel 使用）
    struct DebugParams
    {
        float transitionDuration = 150.0f;   // 選中過渡時間 ms
        float flipDuration = 300.0f;         // 翻牌動畫時間 ms
        float flashDuration = 200.0f;        // 光暈閃爍時間 ms
        float glowAlphaPrimary = 0.8f;       // 主選光暈透明度
        float glowAlphaSecondary = 0.38f;    // 副選光暈透明度（已棄用，改用 MultiCardConfig.subAlpha）
        float glowAlphaIdle = 0.3f;          // 選中態常駐光暈透明度
        float glowScale = 1.3f;              // 光暈尺寸倍數
        bool useEaseOut = true;              // 使用 ease-out 曲線
    };

    static DebugParams& getDebugParams()
    {
        static DebugParams params;
        return params;
    }

    /**
     * 多卡場景動態調整配置
     * 根據卡牌數量自動調整間距、光暈半徑、副選透明度
     * 維持主副選約 2:1 的視覺辨識度
     */
    struct MultiCardConfig
    {
        float spacingScale;      // 間距縮放比例 (1.0 = 100%)
        float glowRadiusScale;   // 光暈半徑縮放比例 (1.0 = 100%)
        float subAlpha;          // 副選透明度
    };

    /**
     * 根據卡牌數量取得對應的動態配置
     * 4-6 張: 100%/85% 間距, 100%/85% 光暈, 0.38 副選
     * 7-8 張: 80%/75% 間距, 82%/80% 光暈, 0.40 副選
     */
    static MultiCardConfig getMultiCardConfig (int numCards)
    {
        if (numCards <= 4)
            return { 1.0f, 1.0f, 0.38f };
        else if (numCards <= 6)
            return { 0.85f, 0.85f, 0.38f };
        else if (numCards == 7)
            return { 0.80f, 0.82f, 0.40f };
        else
            return { 0.75f, 0.80f, 0.40f };
    }

    // 設定當前多卡配置（由 Editor 調用）
    void setMultiCardConfig (const MultiCardConfig& config);
    const MultiCardConfig& getCurrentMultiCardConfig() const { return currentMultiCardConfig; }

private:
    CardSlot cardData;
    bool hasCard = false;
    int round = 0;
    bool selected = false;
    GamePhase currentPhase = GamePhase::Setup;
    RatingMode currentRatingMode = RatingMode::Stars;

    // 選中狀態
    SelectionState selectionState = SelectionState::None;

    // 翻牌動畫狀態
    FlipState flipState = FlipState::FaceDown;
    AnimatedValue flipProgress { 0.0f };  // 0 = 背面, 1 = 正面

    // 光暈動畫狀態
    AnimatedValue glowAlpha { 0.0f };     // 當前光暈透明度
    bool isFlashActive = false;           // 是否在閃爍中
    float flashPhase = 0.0f;              // 閃爍動畫進度 (0-1)

    // 多卡場景配置
    MultiCardConfig currentMultiCardConfig { 1.0f, 1.0f, 0.38f };

    // 光暈顏色
    juce::Colour glowColorWarm { 0xFFFFD700 };   // 暖金：翻牌閃爍用
    juce::Colour glowColorCool { 0xFFFFF8E1 };   // 白金：選中態用

    // 動畫計時
    juce::int64 lastUpdateTime = 0;

    juce::TextButton starButtons[5];
    juce::Slider gainSlider;
    juce::Label lufsLabel;
    juce::TextEditor noteEditor;
    juce::ComboBox guessComboBox;
    juce::StringArray trackNames;

    // Q&A 模式
    juce::TextButton selectButton;
    QAState::FeedbackState qaFeedbackState = QAState::FeedbackState::None;
    bool isQATarget = false;
    bool wasQASelected = false;

    void updateStars();
    void updateGainDisplay();
    void updateNoteEditor();
    void updateGuessComboBox();
    void updateQAButton();

    // 繪製函式（三層架構）
    void paintCardBase (juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void paintGlowLayer (juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void paintGoldBorder (juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void paintCardContent (juce::Graphics& g, const juce::Rectangle<float>& bounds);

    // 計算翻牌 transform
    juce::AffineTransform getFlipTransform (const juce::Rectangle<float>& bounds) const;

    // 更新光暈目標透明度
    void updateGlowTarget();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CardComponent)
};

} // namespace blindcard
