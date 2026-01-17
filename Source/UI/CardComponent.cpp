#include "CardComponent.h"
#include <cmath>

namespace blindcard
{

CardComponent::CardComponent()
{
    for (int i = 0; i < 5; ++i)
    {
        starButtons[i].setButtonText (juce::String::charToString (0x2606)); // 空心星
        starButtons[i].onClick = [this, i]()
        {
            if (onRated && hasCard)
                onRated (cardData.id, i + 1);
        };
        addAndMakeVisible (starButtons[i]);
    }

    // 手動增益 Slider
    gainSlider.setRange (-12.0, 12.0, 0.1);
    gainSlider.setValue (0.0);
    gainSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
    gainSlider.setTextValueSuffix (" dB");
    gainSlider.onValueChange = [this]()
    {
        if (onGainChanged && hasCard)
            onGainChanged (cardData.id, static_cast<float> (gainSlider.getValue()));
    };
    addAndMakeVisible (gainSlider);

    // LUFS 顯示
    lufsLabel.setJustificationType (juce::Justification::centred);
    lufsLabel.setFont (juce::Font (11.0f));
    lufsLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (lufsLabel);

    // 筆記編輯器
    noteEditor.setMultiLine (true);
    noteEditor.setReturnKeyStartsNewLine (true);
    noteEditor.setFont (juce::Font (12.0f));
    noteEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha (0.3f));
    noteEditor.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    noteEditor.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    noteEditor.onTextChange = [this]()
    {
        if (onNoteChanged && hasCard)
            onNoteChanged (cardData.id, noteEditor.getText());
    };
    addAndMakeVisible (noteEditor);

    // 猜測下拉選單
    guessComboBox.setColour (juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha (0.3f));
    guessComboBox.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    guessComboBox.onChange = [this]()
    {
        if (onGuessChanged && hasCard)
        {
            int selectedIndex = guessComboBox.getSelectedItemIndex();
            onGuessChanged (cardData.id, selectedIndex - 1);
        }
    };
    addAndMakeVisible (guessComboBox);

    // Q&A 選擇按鈕
    selectButton.setButtonText (juce::CharPointer_UTF8 ("\xe9\x81\xb8\xe6\xad\xa4")); // 選此
    selectButton.setColour (juce::TextButton::buttonColourId, juce::Colours::orange.darker());
    selectButton.onClick = [this]()
    {
        if (onQASubmit && hasCard)
            onQASubmit (cardData.id);
    };
    addAndMakeVisible (selectButton);

    // 初始化動畫值
    glowAlpha.setEaseOut (true);
    flipProgress.setEaseOut (true);
}

void CardComponent::setCard (const CardSlot& card, int currentRound, bool isSelected, GamePhase phase, RatingMode ratingMode, const QAState* qaState)
{
    cardData = card;
    hasCard = true;
    round = currentRound;
    selected = isSelected;
    currentPhase = phase;
    currentRatingMode = ratingMode;

    // 更新 Q&A 狀態
    if (qaState != nullptr && ratingMode == RatingMode::QA)
    {
        qaFeedbackState = qaState->lastFeedback;
        isQATarget = (card.id == qaState->targetCardId);
        wasQASelected = (card.id == qaState->lastAnsweredCardId);
    }
    else
    {
        qaFeedbackState = QAState::FeedbackState::None;
        isQATarget = false;
        wasQASelected = false;
    }

    // 更新選中狀態（根據 isSelected 設定主選）
    if (isSelected && selectionState == SelectionState::None)
    {
        setSelectionState (SelectionState::Primary);
    }
    else if (!isSelected && selectionState == SelectionState::Primary)
    {
        setSelectionState (SelectionState::None);
    }

    updateStars();
    updateGainDisplay();
    updateNoteEditor();
    updateGuessComboBox();
    updateQAButton();
    repaint();
}

void CardComponent::setTrackNames (const juce::StringArray& names)
{
    trackNames = names;
    updateGuessComboBox();
}

void CardComponent::clearCard()
{
    hasCard = false;
    setSelectionState (SelectionState::None);
    repaint();
}

void CardComponent::setSelectionState (SelectionState state)
{
    if (selectionState == state)
        return;

    selectionState = state;
    updateGlowTarget();

    // 確保動畫計時器在運行
    if (!isTimerRunning())
        startTimerHz (60);
}

void CardComponent::startFlip()
{
    if (flipState == FlipState::Flipping)
        return;

    flipState = FlipState::Flipping;
    auto& params = getDebugParams();

    // 切換翻牌狀態：若已是正面則翻回背面，反之亦然
    float currentValue = flipProgress.getValue();
    float targetValue = (currentValue > 0.5f) ? 0.0f : 1.0f;

    flipProgress.setTarget (targetValue, params.flipDuration);

    // 翻到正面時觸發光暈閃爍
    if (targetValue > 0.5f)
        triggerGlowFlash();

    if (!isTimerRunning())
        startTimerHz (60);
}

void CardComponent::setFlipped (bool flipped, bool animate)
{
    float target = flipped ? 1.0f : 0.0f;

    if (animate)
    {
        auto& params = getDebugParams();
        flipProgress.setTarget (target, params.flipDuration);
        flipState = FlipState::Flipping;

        if (flipped)
            triggerGlowFlash();

        if (!isTimerRunning())
            startTimerHz (60);
    }
    else
    {
        flipProgress.setImmediate (target);
        flipState = flipped ? FlipState::FaceUp : FlipState::FaceDown;
    }
}

void CardComponent::triggerGlowFlash()
{
    isFlashActive = true;
    flashPhase = 0.0f;

    // 閃爍：0.3 → 1.0 → 0.3
    auto& params = getDebugParams();
    glowAlpha.setTarget (1.0f, params.flashDuration / 2.0f);

    if (!isTimerRunning())
        startTimerHz (60);
}

void CardComponent::updateGlowTarget()
{
    auto& params = getDebugParams();
    float targetAlpha = 0.0f;

    switch (selectionState)
    {
        case SelectionState::Primary:
            // 主選 Alpha 鎖定不動
            targetAlpha = params.glowAlphaPrimary;
            break;
        case SelectionState::Secondary:
            // 副選 Alpha 使用多卡場景配置（光暈變小時調高以維持辨識度）
            targetAlpha = currentMultiCardConfig.subAlpha;
            break;
        case SelectionState::None:
            targetAlpha = 0.0f;
            break;
    }

    // 如果選中，保持常駐光暈
    if (selected && selectionState == SelectionState::None)
        targetAlpha = params.glowAlphaIdle;

    glowAlpha.setTarget (targetAlpha, params.transitionDuration);
}

void CardComponent::timerCallback()
{
    auto currentTime = juce::Time::getMillisecondCounterHiRes();
    float deltaTime = lastUpdateTime > 0 ? static_cast<float> (currentTime - lastUpdateTime) : 16.67f;
    lastUpdateTime = static_cast<juce::int64> (currentTime);

    bool needsRepaint = false;
    auto& params = getDebugParams();

    // 更新翻牌動畫
    if (flipProgress.update (deltaTime))
    {
        needsRepaint = true;
    }

    // 檢查翻牌動畫是否完成（獨立於 update 返回值）
    if (flipState == FlipState::Flipping)
    {
        float progress = flipProgress.getValue();
        float target = flipProgress.getTarget();

        // 動畫完成：值已到達目標附近
        if (std::abs (progress - target) < 0.01f)
        {
            flipState = target > 0.5f ? FlipState::FaceUp : FlipState::FaceDown;
        }
    }

    // 更新光暈閃爍
    if (isFlashActive)
    {
        flashPhase += deltaTime / params.flashDuration;

        if (flashPhase >= 1.0f)
        {
            // 閃爍完成，回到目標值
            isFlashActive = false;
            flashPhase = 0.0f;
            updateGlowTarget();
        }
        else if (flashPhase >= 0.5f && glowAlpha.getTarget() > 0.9f)
        {
            // 閃爍中點，開始下降
            updateGlowTarget();
        }

        needsRepaint = true;
    }

    // 更新光暈透明度
    if (glowAlpha.update (deltaTime))
    {
        needsRepaint = true;
    }

    if (needsRepaint)
    {
        repaint();
    }
    else
    {
        // 沒有動畫需要更新，停止計時器
        stopTimer();
        lastUpdateTime = 0;
    }
}

void CardComponent::updateStars()
{
    bool showStars = hasCard && (currentPhase == GamePhase::BlindTesting) && (currentRatingMode == RatingMode::Stars);

    for (int i = 0; i < 5; ++i)
    {
        starButtons[i].setVisible (showStars);
    }

    if (!showStars || !hasCard)
        return;

    int rating = 0;
    if (round < cardData.rounds.size())
        rating = cardData.rounds[round].rating;

    for (int i = 0; i < 5; ++i)
    {
        bool filled = (i < rating);
        starButtons[i].setButtonText (juce::String::charToString (filled ? 0x2605 : 0x2606));
        starButtons[i].setEnabled (true);

        if (filled)
        {
            starButtons[i].setColour (juce::TextButton::textColourOnId, juce::Colours::gold);
            starButtons[i].setColour (juce::TextButton::textColourOffId, juce::Colours::gold);
        }
        else
        {
            starButtons[i].setColour (juce::TextButton::textColourOnId, juce::Colours::white);
            starButtons[i].setColour (juce::TextButton::textColourOffId, juce::Colours::grey);
        }
    }
}

void CardComponent::updateGainDisplay()
{
    if (!hasCard)
    {
        gainSlider.setVisible (false);
        lufsLabel.setVisible (false);
        return;
    }

    bool showGain = (currentPhase != GamePhase::BlindTesting);
    gainSlider.setVisible (showGain);
    lufsLabel.setVisible (showGain);

    gainSlider.setValue (cardData.manualGainDb, juce::dontSendNotification);

    juce::String lufsText;
    if (cardData.hasLUFSMeasurement())
    {
        lufsText = juce::String (cardData.measuredLUFS, 1) + " LUFS";
        if (std::abs (cardData.autoGainDb) > 0.1f)
        {
            lufsText += " (";
            lufsText += (cardData.autoGainDb > 0 ? "+" : "");
            lufsText += juce::String (cardData.autoGainDb, 1) + " dB)";
        }
    }
    else
    {
        lufsText = "Not measured";
    }
    lufsLabel.setText (lufsText, juce::dontSendNotification);
}

void CardComponent::updateNoteEditor()
{
    bool showNote = hasCard && (currentPhase == GamePhase::BlindTesting);
    noteEditor.setVisible (showNote);

    if (showNote && round < cardData.rounds.size())
    {
        if (noteEditor.getText() != cardData.rounds[round].note)
            noteEditor.setText (cardData.rounds[round].note, false);
    }
}

void CardComponent::updateGuessComboBox()
{
    bool showGuess = hasCard && (currentPhase == GamePhase::BlindTesting) && (currentRatingMode == RatingMode::Guess);
    guessComboBox.setVisible (showGuess);

    if (!showGuess)
        return;

    guessComboBox.clear (juce::dontSendNotification);
    guessComboBox.addItem ("Guess...", 1);
    for (int i = 0; i < trackNames.size(); ++i)
    {
        guessComboBox.addItem (trackNames[i], i + 2);
    }

    if (round < cardData.rounds.size())
    {
        int guessedId = cardData.rounds[round].guessedTrackId;
        if (guessedId >= 0 && guessedId < trackNames.size())
            guessComboBox.setSelectedItemIndex (guessedId + 1, juce::dontSendNotification);
        else
            guessComboBox.setSelectedItemIndex (0, juce::dontSendNotification);
    }
}

void CardComponent::updateQAButton()
{
    bool showQAButton = hasCard
        && (currentPhase == GamePhase::BlindTesting)
        && (currentRatingMode == RatingMode::QA)
        && (qaFeedbackState == QAState::FeedbackState::None);

    selectButton.setVisible (showQAButton);
}

juce::AffineTransform CardComponent::getFlipTransform (const juce::Rectangle<float>& bounds) const
{
    float progress = flipProgress.getValue();

    // scaleX 從 1 → 0 → 1（中間翻轉）
    float scaleX;
    if (progress < 0.5f)
    {
        // 前半段：1 → 0
        scaleX = 1.0f - progress * 2.0f;
    }
    else
    {
        // 後半段：0 → 1
        scaleX = (progress - 0.5f) * 2.0f;
    }

    // 確保最小縮放不為零
    scaleX = std::max (0.01f, scaleX);

    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();

    return juce::AffineTransform::scale (scaleX, 1.0f, centerX, centerY);
}

void CardComponent::paintCardBase (juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    juce::Colour bgColour = selected
        ? juce::Colours::steelblue
        : juce::Colours::darkslategrey;

    if (hasCard && cardData.isRemoved)
        bgColour = juce::Colours::dimgrey;

    // Q&A 模式 feedback 顏色
    if (currentRatingMode == RatingMode::QA && qaFeedbackState != QAState::FeedbackState::None)
    {
        if (wasQASelected)
        {
            bgColour = (qaFeedbackState == QAState::FeedbackState::Correct)
                ? juce::Colours::darkgreen
                : juce::Colours::darkred;
        }
        else if (isQATarget && qaFeedbackState == QAState::FeedbackState::Wrong)
        {
            bgColour = juce::Colours::darkgreen.withAlpha (0.7f);
        }
    }

    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, 10.0f);
}

void CardComponent::paintGlowLayer (juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    float alpha = glowAlpha.getValue();
    if (alpha < 0.01f)
        return;

    auto& assets = GlowAssets::getInstance();
    auto& params = getDebugParams();
    float scale = params.glowScale;

    // 套用多卡場景光暈半徑縮放
    scale *= currentMultiCardConfig.glowRadiusScale;

    // 計算光暈區域（擴展到卡片的 1.3 倍，再乘以多卡縮放）
    float expandX = bounds.getWidth() * (scale - 1.0f) / 2.0f;
    float expandY = bounds.getHeight() * (scale - 1.0f) / 2.0f;
    auto glowBounds = bounds.expanded (expandX, expandY);

    // 選擇光暈圖片（閃爍用暖金，選中態用白金）
    // 根據卡片寬度決定是否使用 Compact 版本
    bool useCompact = bounds.getWidth() < 70.0f;
    bool use2x = bounds.getWidth() > 100.0f;  // 大尺寸使用 @2x

    const juce::Image& glowImage = isFlashActive
        ? (useCompact ? assets.getGlowWarmCompact (use2x) : assets.getGlowWarm (use2x))
        : (useCompact ? assets.getGlowCoolCompact (use2x) : assets.getGlowCool (use2x));

    if (glowImage.isValid())
    {
        // 使用 PNG 貼圖繪製
        g.setOpacity (alpha);
        g.drawImage (glowImage,
                     glowBounds,
                     juce::RectanglePlacement::centred);
        g.setOpacity (1.0f);
    }
    else
    {
        // 備用：程式繪製漸層（素材未載入時）
        juce::Colour glowColor = isFlashActive ? glowColorWarm : glowColorCool;

        juce::ColourGradient gradient (
            glowColor.withAlpha (alpha * 0.6f),
            bounds.getCentreX(), bounds.getCentreY(),
            glowColor.withAlpha (0.0f),
            glowBounds.getX(), glowBounds.getCentreY(),
            true);

        g.setGradientFill (gradient);
        g.fillRoundedRectangle (glowBounds, 15.0f);
    }
}

void CardComponent::paintGoldBorder (juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    // 只有選中狀態才繪製金邊
    if (selectionState == SelectionState::None && !selected)
    {
        // 未選中時繪製普通邊框
        g.setColour (juce::Colours::grey);
        g.drawRoundedRectangle (bounds.reduced (1), 10.0f, 2.0f);
        return;
    }

    // 金邊：根據選中狀態調整透明度
    float borderAlpha = 1.0f;
    if (selectionState == SelectionState::Secondary)
        borderAlpha = 0.5f;

    // 白金色邊框
    g.setColour (glowColorCool.withAlpha (borderAlpha));
    g.drawRoundedRectangle (bounds.reduced (1), 10.0f, 2.5f);
}

void CardComponent::paintCardContent (juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    if (!hasCard)
        return;

    g.setColour (juce::Colours::white);
    g.setFont (18.0f);

    juce::String displayText;

    // 翻牌動畫中途切換顯示內容（僅 Revealed 階段使用）
    bool showFront = flipProgress.getValue() > 0.5f;

    switch (currentPhase)
    {
        case GamePhase::Setup:
            // Setup: 顯示真實軌道名稱
            displayText = cardData.realTrackName;
            break;

        case GamePhase::BlindTesting:
            // 盲測階段：永遠顯示代號（A, B, C...），隱藏真實名稱
            displayText = juce::String::charToString ('A' + cardData.displayPosition);
            break;

        case GamePhase::Revealed:
            // 揭曉階段：翻牌動畫顯示真名
            if (showFront)
                displayText = cardData.realTrackName;
            else
                displayText = juce::String::charToString ('A' + cardData.displayPosition);
            break;
    }

    g.drawFittedText (displayText,
                      bounds.toNearestInt().reduced (10, 10).withHeight (30),
                      juce::Justification::centred, 1);

    // 揭曉時顯示平均分
    if (currentPhase == GamePhase::Revealed)
    {
        float avg = cardData.getAverageRating();
        if (avg > 0)
        {
            g.setFont (14.0f);
            g.drawFittedText (juce::String (avg, 1) + " stars",
                              bounds.toNearestInt().reduced (10).withTrimmedTop (40).withHeight (20),
                              juce::Justification::centred, 1);
        }

        if (currentRatingMode == RatingMode::Guess)
        {
            auto [correct, total] = cardData.getGuessAccuracy();

            juce::String guessText;
            if (total > 0)
            {
                guessText = juce::String (correct) + "/" + juce::String (total) + " correct";

                if (correct == total)
                    g.setColour (juce::Colours::lightgreen);
                else if (correct == 0)
                    g.setColour (juce::Colours::lightcoral);
                else
                    g.setColour (juce::Colours::yellow);
            }
            else
            {
                guessText = "No guesses";
                g.setColour (juce::Colours::grey);
            }

            g.setFont (12.0f);
            g.drawFittedText (guessText,
                              bounds.toNearestInt().reduced (10).withTrimmedTop (65).withHeight (20),
                              juce::Justification::centred, 1);
        }
    }
}

void CardComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 第一層：光暈層（在卡牌下方）
    paintGlowLayer (g, bounds);

    // 應用翻牌變換
    if (flipState == FlipState::Flipping)
    {
        auto transform = getFlipTransform (bounds);
        g.addTransform (transform);

        // 陰影偏移（根據翻牌進度）
        float progress = flipProgress.getValue();
        float shadowOffset = std::sin (progress * juce::MathConstants<float>::pi) * 5.0f;

        // 繪製陰影
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (bounds.translated (shadowOffset, shadowOffset), 10.0f);
    }

    // 第二層：卡牌基底
    paintCardBase (g, bounds);

    // 第三層：金邊裝飾
    paintGoldBorder (g, bounds);

    // 卡牌內容
    paintCardContent (g, bounds);
}

void CardComponent::resized()
{
    auto bounds = getLocalBounds().reduced (5);

    // 星星評分在底部
    auto starArea = bounds.removeFromBottom (25);
    int starWidth = starArea.getWidth() / 5;
    for (int i = 0; i < 5; ++i)
    {
        starButtons[i].setBounds (starArea.removeFromLeft (starWidth));
    }

    // Q&A 選擇按鈕 (與星星同一區域)
    auto qaButtonArea = getLocalBounds().reduced (5).removeFromBottom (28);
    selectButton.setBounds (qaButtonArea);

    // 猜測下拉選單 (盲測時顯示，在星星上方)
    bounds.removeFromBottom (3);
    auto guessArea = bounds.removeFromBottom (24);
    guessComboBox.setBounds (guessArea);

    // 增益 Slider (非盲測時顯示)
    bounds.removeFromBottom (3);
    auto gainArea = bounds.removeFromBottom (22);
    gainSlider.setBounds (gainArea);

    // LUFS 顯示 (非盲測時顯示)
    bounds.removeFromBottom (2);
    auto lufsArea = bounds.removeFromBottom (16);
    lufsLabel.setBounds (lufsArea);

    // 筆記編輯器 (盲測時顯示)
    bounds.removeFromTop (35);
    noteEditor.setBounds (bounds);
}

void CardComponent::mouseDown (const juce::MouseEvent&)
{
    if (onClicked && hasCard)
        onClicked (cardData.id);
}

bool CardComponent::keyPressed (const juce::KeyPress& key)
{
    if (key.getKeyCode() == juce::KeyPress::spaceKey)
    {
        return false;
    }

    return Component::keyPressed (key);
}

void CardComponent::setMultiCardConfig (const MultiCardConfig& config)
{
    constexpr float epsilon = 0.001f;

    bool configChanged =
        std::abs (currentMultiCardConfig.spacingScale - config.spacingScale) > epsilon ||
        std::abs (currentMultiCardConfig.glowRadiusScale - config.glowRadiusScale) > epsilon ||
        std::abs (currentMultiCardConfig.subAlpha - config.subAlpha) > epsilon;

    if (configChanged)
    {
        currentMultiCardConfig = config;

        // 如果是副選狀態，更新光暈透明度目標
        if (selectionState == SelectionState::Secondary)
        {
            updateGlowTarget();
        }

        repaint();
    }
}

} // namespace blindcard
