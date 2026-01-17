#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

BlindCardEditor::BlindCardEditor (BlindCardProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // 輪數設定
    roundsLabel.setText ("Rounds:", juce::dontSendNotification);
    addAndMakeVisible (roundsLabel);

    roundsSlider.setRange (1, 10, 1);
    roundsSlider.setValue (1);
    roundsSlider.setSliderStyle (juce::Slider::IncDecButtons);
    roundsSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 40, 20);
    roundsSlider.onValueChange = [this]()
    {
        processorRef.getManager().setTotalRounds (static_cast<int> (roundsSlider.getValue()));
    };
    addAndMakeVisible (roundsSlider);

    // 按鈕
    shuffleButton.onClick = [this]() { processorRef.getManager().shuffle(); };
    addAndMakeVisible (shuffleButton);

    nextRoundButton.onClick = [this]() { processorRef.getManager().nextRound(); };
    addAndMakeVisible (nextRoundButton);

    revealButton.onClick = [this]()
    {
        processorRef.getManager().reveal();
        // 揭曉時觸發所有卡牌翻牌動畫
        for (auto* card : cardComponents)
        {
            if (card->isVisible())
                card->startFlip();
        }
    };
    addAndMakeVisible (revealButton);

    resetButton.onClick = [this]()
    {
        processorRef.getManager().reset();
        clearAllSelections();
    };
    addAndMakeVisible (resetButton);

    bypassButton.onClick = [this]()
    {
        processorRef.getManager().setBypassAll (bypassButton.getToggleState());
    };
    addAndMakeVisible (bypassButton);

    measureButton.onClick = [this]()
    {
        if (processorRef.isMeasuring())
        {
            processorRef.stopMeasurement();
            stopTimer();
            measureButton.setButtonText ("Measure (5s)");
        }
        else
        {
            processorRef.startMeasurement (5.0f);
            startTimerHz (10);
            measureButton.setButtonText ("Stop");
        }
    };
    addAndMakeVisible (measureButton);

    measureLabel.setJustificationType (juce::Justification::centredLeft);
    measureLabel.setFont (juce::Font (12.0f));
    addAndMakeVisible (measureLabel);

    modeButton.onClick = [this]()
    {
        auto& manager = processorRef.getManager();
        auto currentMode = manager.getRatingMode();
        blindcard::RatingMode nextMode;

        switch (currentMode)
        {
            case blindcard::RatingMode::Stars:
                nextMode = blindcard::RatingMode::Guess;
                break;
            case blindcard::RatingMode::Guess:
                nextMode = manager.canStartQAMode()
                    ? blindcard::RatingMode::QA
                    : blindcard::RatingMode::Stars;
                break;
            case blindcard::RatingMode::QA:
                nextMode = blindcard::RatingMode::Stars;
                break;
        }
        manager.setRatingMode (nextMode);
    };
    addAndMakeVisible (modeButton);

    // 狀態列
    statusLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusLabel);

    // 筆記區
    noteEditor.setMultiLine (true);
    noteEditor.setReturnKeyStartsNewLine (true);
    noteEditor.onTextChange = [this]()
    {
        if (selectedCardForNote >= 0)
        {
            processorRef.getManager().setNote (selectedCardForNote, noteEditor.getText());
        }
    };
    addAndMakeVisible (noteEditor);

    // Q&A 模式 UI
    questionLabel.setJustificationType (juce::Justification::centred);
    questionLabel.setFont (juce::Font (20.0f).boldened());
    questionLabel.setColour (juce::Label::textColourId, juce::Colours::yellow);
    addAndMakeVisible (questionLabel);

    progressLabel.setJustificationType (juce::Justification::centred);
    progressLabel.setFont (juce::Font (14.0f));
    addAndMakeVisible (progressLabel);

    resultLabel.setJustificationType (juce::Justification::centred);
    resultLabel.setFont (juce::Font (24.0f).boldened());
    resultLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    addAndMakeVisible (resultLabel);

    nextQuestionButton.setButtonText (juce::CharPointer_UTF8 ("\xe4\xb8\x8b\xe4\xb8\x80\xe9\xa1\x8c")); // 下一題
    nextQuestionButton.onClick = [this]()
    {
        processorRef.getManager().nextQAQuestion();
    };
    addAndMakeVisible (nextQuestionButton);

    playAgainButton.setButtonText (juce::CharPointer_UTF8 ("\xe5\x86\x8d\xe7\x8e\xa9\xe4\xb8\x80\xe6\xac\xa1")); // 再玩一次
    playAgainButton.onClick = [this]()
    {
        processorRef.getManager().reset();
        clearAllSelections();
    };
    addAndMakeVisible (playAgainButton);

    // 建立卡牌元件（最多 8 張）
    for (int i = 0; i < blindcard::GameState::MaxCards; ++i)
    {
        auto* card = cardComponents.add (new blindcard::CardComponent());
        card->onClicked = [this] (int id) { onCardClicked (id); };
        card->onRated = [this] (int id, int stars) { onCardRated (id, stars); };
        card->onGainChanged = [this] (int id, float gain) { onGainChanged (id, gain); };
        card->onNoteChanged = [this] (int id, const juce::String& note)
        {
            processorRef.getManager().setNote (id, note);
        };
        card->onGuessChanged = [this] (int id, int guessedTrackId)
        {
            processorRef.getManager().setGuess (id, guessedTrackId);
        };
        card->onQASubmit = [this] (int id) { onQASubmit (id); };
        addAndMakeVisible (card);
    }

    // Debug Panel 設定
    debugPanel.onTestFlip = [this]()
    {
        for (auto* card : cardComponents)
        {
            if (card->isVisible())
                card->startFlip();
        }
    };
    debugPanel.onTestGlow = [this]()
    {
        for (auto* card : cardComponents)
        {
            if (card->isVisible())
                card->triggerGlowFlash();
        }
    };
    debugPanel.onAddTestCards = [this]()
    {
        processorRef.getManager().addTestCards (4);
    };
    debugPanel.onAutoScreenshot = [this]()
    {
        startAutoScreenshot();
    };
    debugPanel.setVisible (false);
    addAndMakeVisible (debugPanel);

    // 監聽 Manager 變化
    processorRef.getManager().addChangeListener (this);

    // 啟用鍵盤焦點
    setWantsKeyboardFocus (true);

    // Standalone 測試模式：自動補足測試卡牌至 4 張
    // addTestCards 會檢查是否需要加入更多卡牌
    processorRef.getManager().addTestCards (4);

    setSize (800, 500);
    updateUI();
}

BlindCardEditor::~BlindCardEditor()
{
    stopTimer();
    processorRef.getManager().removeChangeListener (this);
}

bool BlindCardEditor::keyPressed (const juce::KeyPress& key)
{
#if JUCE_DEBUG
    // D 鍵切換 Debug Panel (僅限 Debug build)
    if (key.getTextCharacter() == 'd' || key.getTextCharacter() == 'D')
    {
        debugPanelVisible = !debugPanelVisible;
        debugPanel.setVisible (debugPanelVisible);
        resized();
        return true;
    }
#endif

    return false;
}

void BlindCardEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    juce::Component::SafePointer<BlindCardEditor> safeThis (this);
    juce::MessageManager::callAsync ([safeThis]()
    {
        if (safeThis != nullptr)
            safeThis->updateUI();
    });
}

void BlindCardEditor::selectCard (int cardId, bool asPrimary)
{
    // 檢查是否已選中
    auto it = std::find (selectedIndices.begin(), selectedIndices.end(), cardId);

    if (asPrimary)
    {
        if (it != selectedIndices.end())
        {
            // 已在列表中，移到最前面（設為主選）
            selectedIndices.erase (it);
        }
        selectedIndices.insert (selectedIndices.begin(), cardId);
    }
    else
    {
        if (it == selectedIndices.end())
        {
            // 不在列表中，加入
            selectedIndices.push_back (cardId);
        }
    }

    // MVP 限制：最多 2 個選中
    while (selectedIndices.size() > MaxSelections)
    {
        selectedIndices.pop_back();
    }

    updateCardSelectionStates();
}

void BlindCardEditor::deselectCard (int cardId)
{
    auto it = std::find (selectedIndices.begin(), selectedIndices.end(), cardId);
    if (it != selectedIndices.end())
    {
        selectedIndices.erase (it);
    }

    updateCardSelectionStates();
}

void BlindCardEditor::clearAllSelections()
{
    selectedIndices.clear();
    updateCardSelectionStates();
}

bool BlindCardEditor::isCardSelected (int cardId) const
{
    return std::find (selectedIndices.begin(), selectedIndices.end(), cardId) != selectedIndices.end();
}

blindcard::SelectionState BlindCardEditor::getCardSelectionState (int cardId) const
{
    if (selectedIndices.empty())
        return blindcard::SelectionState::None;

    if (selectedIndices[0] == cardId)
        return blindcard::SelectionState::Primary;

    for (size_t i = 1; i < selectedIndices.size(); ++i)
    {
        if (selectedIndices[i] == cardId)
            return blindcard::SelectionState::Secondary;
    }

    return blindcard::SelectionState::None;
}

void BlindCardEditor::updateCardSelectionStates()
{
    for (auto* card : cardComponents)
    {
        if (card->isVisible())
        {
            // 這裡需要知道卡牌的 ID，但 CardComponent 沒有暴露
            // 改用 setCard 時更新選中狀態
        }
    }
    repaint();
}

void BlindCardEditor::updateUI()
{
    auto& manager = processorRef.getManager();
    auto phase = manager.getPhase();
    const auto& cards = manager.getCards();
    int currentRound = manager.getCurrentRound();
    int totalRounds = manager.getTotalRounds();
    int selectedId = manager.getSelectedCardId();

    // 更新控制按鈕狀態
    roundsSlider.setEnabled (phase == blindcard::GamePhase::Setup);
    roundsSlider.setValue (totalRounds, juce::dontSendNotification);

    // Shuffle 按鈕需要至少 MinCards 張卡牌才能啟用
    bool hasEnoughCards = cards.size() >= blindcard::GameState::MinCards;
    shuffleButton.setEnabled (phase == blindcard::GamePhase::Setup && hasEnoughCards);
    nextRoundButton.setEnabled (phase == blindcard::GamePhase::BlindTesting && currentRound < totalRounds - 1);
    revealButton.setEnabled (phase == blindcard::GamePhase::BlindTesting);
    resetButton.setEnabled (phase != blindcard::GamePhase::Setup);
    bypassButton.setToggleState (manager.isBypassAll(), juce::dontSendNotification);

    // 更新模式按鈕
    auto ratingMode = manager.getRatingMode();
    juce::String modeText;
    switch (ratingMode)
    {
        case blindcard::RatingMode::Stars: modeText = "Mode: Stars"; break;
        case blindcard::RatingMode::Guess: modeText = "Mode: Guess"; break;
        case blindcard::RatingMode::QA:    modeText = "Mode: Q&A";   break;
    }
    modeButton.setButtonText (modeText);
    modeButton.setEnabled (phase == blindcard::GamePhase::Setup);

    // 更新狀態列
    juce::String statusText;
    switch (phase)
    {
        case blindcard::GamePhase::Setup:
            statusText = "Setup - " + juce::String (cards.size()) + "/" + juce::String (blindcard::GameState::MaxCards) + " tracks";
            if (cards.size() < blindcard::GameState::MinCards)
                statusText += " (need at least " + juce::String (blindcard::GameState::MinCards) + ")";
            else if (cards.size() >= blindcard::GameState::MaxCards)
                statusText += " (max reached)";
            break;
        case blindcard::GamePhase::BlindTesting:
            statusText = "Round " + juce::String (currentRound + 1) + " / " + juce::String (totalRounds);
            break;
        case blindcard::GamePhase::Revealed:
            statusText = "Results";
            break;
    }
    statusLabel.setText (statusText, juce::dontSendNotification);

    // 收集軌道名稱
    juce::StringArray trackNames;
    for (const auto& card : cards)
        trackNames.add (card.realTrackName);

    // Q&A 狀態
    const blindcard::QAState* qaStatePtr = nullptr;
    if (ratingMode == blindcard::RatingMode::QA)
        qaStatePtr = &manager.getQAState();

    // 更新卡牌
    for (int i = 0; i < cardComponents.size(); ++i)
    {
        if (i < cards.size())
        {
            int cardId = cards[i].id;
            bool isSelected = (cardId == selectedId);

            cardComponents[i]->setVisible (true);
            cardComponents[i]->setTrackNames (trackNames);
            cardComponents[i]->setCard (cards.getReference(i), currentRound,
                                         isSelected, phase, ratingMode, qaStatePtr);

            // 更新選中狀態
            auto selState = getCardSelectionState (cardId);
            cardComponents[i]->setSelectionState (selState);
        }
        else
        {
            cardComponents[i]->setVisible (false);
            cardComponents[i]->clearCard();
        }
    }

    // 隱藏底部筆記區
    noteEditor.setVisible (false);

    // 更新 Q&A UI
    updateQAUI();

    resized();
    repaint();
}

void BlindCardEditor::onCardClicked (int cardId)
{
    auto& manager = processorRef.getManager();
    int currentSelectedId = manager.getSelectedCardId();

    if (currentSelectedId == cardId)
    {
        // 點擊已選中的卡牌：取消選中
        manager.deselectCard();
        deselectCard (cardId);
    }
    else
    {
        // 點擊新卡牌：設為主選
        if (currentSelectedId >= 0)
        {
            // 將舊主選降為副選
            selectCard (currentSelectedId, false);
        }
        manager.selectCard (cardId);
        selectCard (cardId, true);
    }
}

void BlindCardEditor::onCardRated (int cardId, int stars)
{
    processorRef.getManager().rateCard (cardId, stars);
}

void BlindCardEditor::onGainChanged (int cardId, float gainDb)
{
    processorRef.getManager().setManualGain (cardId, gainDb);
}

void BlindCardEditor::onQASubmit (int cardId)
{
    processorRef.getManager().submitQAAnswer (cardId);
}

void BlindCardEditor::updateQAUI()
{
    auto& manager = processorRef.getManager();
    auto ratingMode = manager.getRatingMode();
    auto phase = manager.getPhase();

    bool isQAMode = (ratingMode == blindcard::RatingMode::QA);
    bool isBlindTesting = (phase == blindcard::GamePhase::BlindTesting);
    bool isRevealed = (phase == blindcard::GamePhase::Revealed);

    const auto& qaState = manager.getQAState();
    int maxQuestions = manager.getQAMaxQuestions();

    // 問題顯示
    if (isQAMode && isBlindTesting && !qaState.isComplete (maxQuestions))
    {
        juce::String trackName = manager.getCurrentQuestionTrackName();
        juce::String questionText = juce::String (juce::CharPointer_UTF8 ("\xe5\x93\xaa\xe8\xbb\x8c\xe6\x98\xaf ")) + trackName + "?";
        questionLabel.setText (questionText, juce::dontSendNotification);
        questionLabel.setVisible (true);

        progressLabel.setText (
            "Question " + juce::String (qaState.currentQuestion + 1) +
            "/" + juce::String (maxQuestions),
            juce::dontSendNotification);
        progressLabel.setVisible (true);
    }
    else
    {
        questionLabel.setVisible (false);
        progressLabel.setVisible (false);
    }

    // Feedback 後的 "下一題" 按鈕
    bool showNext = isQAMode && isBlindTesting
                    && qaState.lastFeedback != blindcard::QAState::FeedbackState::None
                    && !qaState.isComplete (maxQuestions);
    nextQuestionButton.setVisible (showNext);

    // 結果顯示
    if (isQAMode && isRevealed)
    {
        int correct = qaState.getCorrectCount();
        int total = static_cast<int> (qaState.answers.size());
        int percent = total > 0 ? (correct * 100 / total) : 0;

        juce::String resultText = juce::String (juce::CharPointer_UTF8 ("\xe6\xad\xa3\xe7\xa2\xba\xe7\x8e\x87: ")) +
            juce::String (correct) + "/" +
            juce::String (total) + " (" + juce::String (percent) + "%)";
        resultLabel.setText (resultText, juce::dontSendNotification);
        resultLabel.setVisible (true);
        playAgainButton.setVisible (true);
    }
    else
    {
        resultLabel.setVisible (false);
        playAgainButton.setVisible (false);
    }
}

void BlindCardEditor::timerCallback()
{
    if (!processorRef.isMeasuring())
    {
        stopTimer();
        measureButton.setButtonText ("Measure (5s)");
        measureLabel.setText ("", juce::dontSendNotification);
        return;
    }

    float progress = processorRef.getMeasurementProgress();
    measureLabel.setText (juce::String (static_cast<int> (progress * 100)) + "%", juce::dontSendNotification);
}

void BlindCardEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));
}

void BlindCardEditor::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    // Debug Panel（右側浮動）
    if (debugPanelVisible)
    {
        auto panelBounds = bounds.removeFromRight (310);
        debugPanel.setBounds (panelBounds);
        bounds.removeFromRight (10);
    }

    // 控制列 (上排)
    auto controlBar = bounds.removeFromTop (30);
    roundsLabel.setBounds (controlBar.removeFromLeft (60));
    roundsSlider.setBounds (controlBar.removeFromLeft (100));
    controlBar.removeFromLeft (20);
    shuffleButton.setBounds (controlBar.removeFromLeft (80));
    controlBar.removeFromLeft (10);
    nextRoundButton.setBounds (controlBar.removeFromLeft (100));
    controlBar.removeFromLeft (10);
    revealButton.setBounds (controlBar.removeFromLeft (80));
    controlBar.removeFromLeft (10);
    resetButton.setBounds (controlBar.removeFromLeft (100));

    bounds.removeFromTop (5);

    // 控制列 (下排)
    auto controlBar2 = bounds.removeFromTop (30);
    bypassButton.setBounds (controlBar2.removeFromLeft (100));
    controlBar2.removeFromLeft (20);
    measureButton.setBounds (controlBar2.removeFromLeft (100));
    controlBar2.removeFromLeft (10);
    measureLabel.setBounds (controlBar2.removeFromLeft (50));
    controlBar2.removeFromLeft (20);
    modeButton.setBounds (controlBar2.removeFromLeft (110));

    bounds.removeFromTop (10);

    // Q&A 問題區域
    auto& manager = processorRef.getManager();
    auto ratingMode = manager.getRatingMode();
    auto phase = manager.getPhase();

    if (ratingMode == blindcard::RatingMode::QA && phase == blindcard::GamePhase::BlindTesting)
    {
        questionLabel.setBounds (bounds.removeFromTop (28));
        progressLabel.setBounds (bounds.removeFromTop (20));
        bounds.removeFromTop (5);
    }

    // 狀態列
    statusLabel.setBounds (bounds.removeFromTop (25));
    bounds.removeFromTop (5);

    // Q&A 結果區域
    if (ratingMode == blindcard::RatingMode::QA && phase == blindcard::GamePhase::Revealed)
    {
        resultLabel.setBounds (bounds.removeFromTop (35));
        bounds.removeFromTop (5);
        auto playAgainArea = bounds.removeFromTop (35);
        playAgainButton.setBounds (playAgainArea.reduced (200, 0));
        bounds.removeFromTop (10);
    }

    // "下一題" 按鈕
    juce::Rectangle<int> nextButtonArea;
    if (nextQuestionButton.isVisible())
    {
        nextButtonArea = bounds.removeFromBottom (35);
        bounds.removeFromBottom (5);
    }

    // 卡牌區域 - 固定 4+4 雙排佈局，統一靠左對齊
    auto cardArea = bounds;
    const auto& cards = manager.getCards();
    int numCards = static_cast<int> (cards.size());

    // 取得多卡場景配置
    auto multiCardConfig = blindcard::CardComponent::getMultiCardConfig (numCards);

    // 固定佈局參數
    int hSpacing = 10;  // 水平間距
    int vSpacing = 16;  // 垂直間距（行間）- 會議決議從 8px 改為 16px
    int cardsPerRow = 4;

    int rowHeight = (cardArea.getHeight() - vSpacing) / 2;
    auto topRow = cardArea.removeFromTop (rowHeight);
    cardArea.removeFromTop (vSpacing);
    auto bottomRow = cardArea;

    int cardWidth = (topRow.getWidth() - (cardsPerRow - 1) * hSpacing) / cardsPerRow;
    cardWidth = juce::jmin (cardWidth, 150);

    // 統一靠左對齊（會議決議：移除置中邏輯）
    for (int i = 0; i < cardComponents.size(); ++i)
    {
        if (i < numCards)
        {
            cardComponents[i]->setMultiCardConfig (multiCardConfig);
            cardComponents[i]->setVisible (true);

            if (i < cardsPerRow)
            {
                // 上排 (0-3)
                int x = i * (cardWidth + hSpacing);
                cardComponents[i]->setBounds (topRow.getX() + x, topRow.getY(), cardWidth, rowHeight);
            }
            else
            {
                // 下排 (4-7)
                int x = (i - cardsPerRow) * (cardWidth + hSpacing);
                cardComponents[i]->setBounds (bottomRow.getX() + x, bottomRow.getY(), cardWidth, rowHeight);
            }
        }
        else
        {
            cardComponents[i]->setVisible (false);
        }
    }

    // 設定 "下一題" 按鈕位置
    if (nextQuestionButton.isVisible())
    {
        nextQuestionButton.setBounds (nextButtonArea.reduced (200, 0));
    }
}

void BlindCardEditor::startAutoScreenshot()
{
    // 設定截圖目錄
    screenshotDir = juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                        .getChildFile ("BlindCard_Screenshots");
    screenshotDir.createDirectory();

    // 隱藏 Debug Panel 避免遮擋
    debugPanelVisible = false;
    debugPanel.setVisible (false);

    // 開始第一步
    screenshotStep = 0;
    performScreenshotStep();
}

void BlindCardEditor::performScreenshotStep()
{
    if (screenshotStep < 0 || screenshotStep >= 4)
    {
        // 完成或未啟動
        screenshotStep = -1;
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::InfoIcon,
            "Auto Screenshot",
            "Screenshots saved to Desktop/BlindCard_Screenshots/");
        return;
    }

    auto& manager = processorRef.getManager();
    int targetCards = screenshotCardCounts[screenshotStep];

    // 重置並添加指定數量的測試卡牌
    manager.reset();
    clearAllSelections();
    manager.addTestCards (targetCards);

    // 設定主選（第一張）和副選（第二張）
    if (targetCards >= 1)
    {
        selectCard (0, true);   // 主選
    }
    if (targetCards >= 2)
    {
        selectCard (1, false);  // 副選
    }

    updateUI();
    resized();

    // 延遲截圖，等待動畫完成
    juce::Timer::callAfterDelay (500, [this, targetCards]()
    {
        juce::String filename = juce::String::formatted ("cards_%d_primary_secondary.png", targetCards);
        saveScreenshot (filename);

        // 進入下一步
        screenshotStep++;
        performScreenshotStep();
    });
}

void BlindCardEditor::saveScreenshot (const juce::String& filename)
{
    // 使用 JUCE 的 createComponentSnapshot 截取整個編輯器
    auto snapshot = createComponentSnapshot (getLocalBounds());

    if (snapshot.isValid())
    {
        juce::File outputFile = screenshotDir.getChildFile (filename);
        juce::FileOutputStream stream (outputFile);

        if (stream.openedOk())
        {
            juce::PNGImageFormat pngFormat;
            pngFormat.writeImageToStream (snapshot, stream);
            DBG ("Screenshot saved: " << outputFile.getFullPathName());
        }
    }
}
