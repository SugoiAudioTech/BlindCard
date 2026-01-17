#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CardComponent.h"

namespace blindcard
{

/**
 * Debug Panel 元件
 * 按 D 鍵可切換顯示
 * 用於即時調整動畫參數
 */
class DebugPanel final : public juce::Component
{
public:
    DebugPanel()
    {
        // 標題
        titleLabel.setText ("Debug Panel (Press D to toggle)", juce::dontSendNotification);
        titleLabel.setFont (juce::Font (14.0f).boldened());
        titleLabel.setColour (juce::Label::textColourId, juce::Colours::yellow);
        addAndMakeVisible (titleLabel);

        // 過渡時間
        transitionLabel.setText ("Transition (ms):", juce::dontSendNotification);
        addAndMakeVisible (transitionLabel);
        transitionSlider.setRange (50.0, 500.0, 10.0);
        transitionSlider.setValue (CardComponent::getDebugParams().transitionDuration);
        transitionSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        transitionSlider.onValueChange = [this]()
        {
            CardComponent::getDebugParams().transitionDuration = static_cast<float> (transitionSlider.getValue());
        };
        addAndMakeVisible (transitionSlider);

        // 翻牌時間
        flipLabel.setText ("Flip Duration (ms):", juce::dontSendNotification);
        addAndMakeVisible (flipLabel);
        flipSlider.setRange (100.0, 800.0, 10.0);
        flipSlider.setValue (CardComponent::getDebugParams().flipDuration);
        flipSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        flipSlider.onValueChange = [this]()
        {
            CardComponent::getDebugParams().flipDuration = static_cast<float> (flipSlider.getValue());
        };
        addAndMakeVisible (flipSlider);

        // 閃爍時間
        flashLabel.setText ("Flash Duration (ms):", juce::dontSendNotification);
        addAndMakeVisible (flashLabel);
        flashSlider.setRange (50.0, 500.0, 10.0);
        flashSlider.setValue (CardComponent::getDebugParams().flashDuration);
        flashSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        flashSlider.onValueChange = [this]()
        {
            CardComponent::getDebugParams().flashDuration = static_cast<float> (flashSlider.getValue());
        };
        addAndMakeVisible (flashSlider);

        // 主選光暈透明度
        glowPrimaryLabel.setText ("Glow Alpha Primary:", juce::dontSendNotification);
        addAndMakeVisible (glowPrimaryLabel);
        glowPrimarySlider.setRange (0.0, 1.0, 0.05);
        glowPrimarySlider.setValue (CardComponent::getDebugParams().glowAlphaPrimary);
        glowPrimarySlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        glowPrimarySlider.onValueChange = [this]()
        {
            CardComponent::getDebugParams().glowAlphaPrimary = static_cast<float> (glowPrimarySlider.getValue());
        };
        addAndMakeVisible (glowPrimarySlider);

        // 副選光暈透明度
        glowSecondaryLabel.setText ("Glow Alpha Secondary:", juce::dontSendNotification);
        addAndMakeVisible (glowSecondaryLabel);
        glowSecondarySlider.setRange (0.0, 1.0, 0.05);
        glowSecondarySlider.setValue (CardComponent::getDebugParams().glowAlphaSecondary);
        glowSecondarySlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        glowSecondarySlider.onValueChange = [this]()
        {
            CardComponent::getDebugParams().glowAlphaSecondary = static_cast<float> (glowSecondarySlider.getValue());
        };
        addAndMakeVisible (glowSecondarySlider);

        // 常駐光暈透明度
        glowIdleLabel.setText ("Glow Alpha Idle:", juce::dontSendNotification);
        addAndMakeVisible (glowIdleLabel);
        glowIdleSlider.setRange (0.0, 1.0, 0.05);
        glowIdleSlider.setValue (CardComponent::getDebugParams().glowAlphaIdle);
        glowIdleSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        glowIdleSlider.onValueChange = [this]()
        {
            CardComponent::getDebugParams().glowAlphaIdle = static_cast<float> (glowIdleSlider.getValue());
        };
        addAndMakeVisible (glowIdleSlider);

        // 光暈尺寸
        glowScaleLabel.setText ("Glow Scale:", juce::dontSendNotification);
        addAndMakeVisible (glowScaleLabel);
        glowScaleSlider.setRange (1.0, 2.0, 0.05);
        glowScaleSlider.setValue (CardComponent::getDebugParams().glowScale);
        glowScaleSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        glowScaleSlider.onValueChange = [this]()
        {
            CardComponent::getDebugParams().glowScale = static_cast<float> (glowScaleSlider.getValue());
        };
        addAndMakeVisible (glowScaleSlider);

        // Ease-out 開關
        easeOutToggle.setButtonText ("Use Ease-Out Curve");
        easeOutToggle.setToggleState (CardComponent::getDebugParams().useEaseOut, juce::dontSendNotification);
        easeOutToggle.onClick = [this]()
        {
            CardComponent::getDebugParams().useEaseOut = easeOutToggle.getToggleState();
        };
        addAndMakeVisible (easeOutToggle);

        // 測試按鈕
        testFlipButton.setButtonText ("Test Flip Animation");
        testFlipButton.onClick = [this]()
        {
            if (onTestFlip)
                onTestFlip();
        };
        addAndMakeVisible (testFlipButton);

        testGlowButton.setButtonText ("Test Glow Flash");
        testGlowButton.onClick = [this]()
        {
            if (onTestGlow)
                onTestGlow();
        };
        addAndMakeVisible (testGlowButton);

        addTestCardsButton.setButtonText ("Add Test Cards (4)");
        addTestCardsButton.onClick = [this]()
        {
            if (onAddTestCards)
                onAddTestCards();
        };
        addAndMakeVisible (addTestCardsButton);

        // 自動截圖按鈕
        autoScreenshotButton.setButtonText ("Auto Screenshot (4,6,7,8)");
        autoScreenshotButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgreen);
        autoScreenshotButton.onClick = [this]()
        {
            if (onAutoScreenshot)
                onAutoScreenshot();
        };
        addAndMakeVisible (autoScreenshotButton);

        setSize (300, 460);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black.withAlpha (0.85f));
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.drawRect (getLocalBounds(), 1);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (10);
        const int rowHeight = 28;
        const int labelWidth = 140;

        titleLabel.setBounds (bounds.removeFromTop (rowHeight));
        bounds.removeFromTop (5);

        auto makeRow = [&] (juce::Label& label, juce::Slider& slider)
        {
            auto row = bounds.removeFromTop (rowHeight);
            label.setBounds (row.removeFromLeft (labelWidth));
            slider.setBounds (row);
            bounds.removeFromTop (2);
        };

        makeRow (transitionLabel, transitionSlider);
        makeRow (flipLabel, flipSlider);
        makeRow (flashLabel, flashSlider);
        makeRow (glowPrimaryLabel, glowPrimarySlider);
        makeRow (glowSecondaryLabel, glowSecondarySlider);
        makeRow (glowIdleLabel, glowIdleSlider);
        makeRow (glowScaleLabel, glowScaleSlider);

        bounds.removeFromTop (5);
        easeOutToggle.setBounds (bounds.removeFromTop (rowHeight));

        bounds.removeFromTop (10);
        testFlipButton.setBounds (bounds.removeFromTop (30));
        bounds.removeFromTop (5);
        testGlowButton.setBounds (bounds.removeFromTop (30));
        bounds.removeFromTop (5);
        addTestCardsButton.setBounds (bounds.removeFromTop (30));
        bounds.removeFromTop (5);
        autoScreenshotButton.setBounds (bounds.removeFromTop (30));
    }

    // 回調函式（供 Editor 連接）
    std::function<void()> onTestFlip;
    std::function<void()> onTestGlow;
    std::function<void()> onAddTestCards;
    std::function<void()> onAutoScreenshot;

private:
    juce::Label titleLabel;

    juce::Label transitionLabel;
    juce::Slider transitionSlider;

    juce::Label flipLabel;
    juce::Slider flipSlider;

    juce::Label flashLabel;
    juce::Slider flashSlider;

    juce::Label glowPrimaryLabel;
    juce::Slider glowPrimarySlider;

    juce::Label glowSecondaryLabel;
    juce::Slider glowSecondarySlider;

    juce::Label glowIdleLabel;
    juce::Slider glowIdleSlider;

    juce::Label glowScaleLabel;
    juce::Slider glowScaleSlider;

    juce::ToggleButton easeOutToggle;

    juce::TextButton testFlipButton;
    juce::TextButton testGlowButton;
    juce::TextButton addTestCardsButton;
    juce::TextButton autoScreenshotButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DebugPanel)
};

} // namespace blindcard
