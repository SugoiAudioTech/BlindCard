#include "PluginProcessor.h"
#include "PluginEditor.h"

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

    revealButton.onClick = [this]() { processorRef.getManager().reveal(); };
    addAndMakeVisible (revealButton);

    resetButton.onClick = [this]() { processorRef.getManager().reset(); };
    addAndMakeVisible (resetButton);

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

    // 建立卡牌元件（最多 8 張）
    for (int i = 0; i < blindcard::GameState::MaxCards; ++i)
    {
        auto* card = cardComponents.add (new blindcard::CardComponent());
        card->onClicked = [this] (int id) { onCardClicked (id); };
        card->onRated = [this] (int id, int stars) { onCardRated (id, stars); };
        addAndMakeVisible (card);
    }

    // 監聽 Manager 變化
    processorRef.getManager().addChangeListener (this);

    setSize (800, 500);
    updateUI();
}

BlindCardEditor::~BlindCardEditor()
{
    processorRef.getManager().removeChangeListener (this);
}

void BlindCardEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // 確保在 message thread 更新 UI
    juce::Component::SafePointer<BlindCardEditor> safeThis (this);
    juce::MessageManager::callAsync ([safeThis]()
    {
        if (safeThis != nullptr)
            safeThis->updateUI();
    });
}

void BlindCardEditor::updateUI()
{
    auto& manager = processorRef.getManager();
    auto phase = manager.getPhase();
    auto cards = manager.getCards();
    int currentRound = manager.getCurrentRound();
    int totalRounds = manager.getTotalRounds();
    int selectedId = manager.getSelectedCardId();

    // 更新控制按鈕狀態
    roundsSlider.setEnabled (phase == blindcard::GamePhase::Setup);
    roundsSlider.setValue (totalRounds, juce::dontSendNotification);

    shuffleButton.setEnabled (phase == blindcard::GamePhase::Setup && !cards.isEmpty());
    nextRoundButton.setEnabled (phase == blindcard::GamePhase::BlindTesting && currentRound < totalRounds - 1);
    revealButton.setEnabled (phase == blindcard::GamePhase::BlindTesting);
    resetButton.setEnabled (phase != blindcard::GamePhase::Setup);

    // 更新狀態列
    juce::String statusText;
    switch (phase)
    {
        case blindcard::GamePhase::Setup:
            statusText = "Setup - " + juce::String (cards.size()) + " tracks registered";
            break;
        case blindcard::GamePhase::BlindTesting:
            statusText = "Round " + juce::String (currentRound + 1) + " / " + juce::String (totalRounds);
            break;
        case blindcard::GamePhase::Revealed:
            statusText = "Results";
            break;
    }
    statusLabel.setText (statusText, juce::dontSendNotification);

    // 更新卡牌
    for (int i = 0; i < cardComponents.size(); ++i)
    {
        if (i < cards.size())
        {
            cardComponents[i]->setVisible (true);
            cardComponents[i]->setCard (&cards.getReference(i), currentRound,
                                         cards[i].id == selectedId, phase);
        }
        else
        {
            cardComponents[i]->setVisible (false);
        }
    }

    // 更新筆記區
    noteEditor.setEnabled (phase == blindcard::GamePhase::BlindTesting && selectedId >= 0);
    if (selectedId >= 0 && selectedId < cards.size())
    {
        if (selectedCardForNote != selectedId)
        {
            selectedCardForNote = selectedId;
            const auto& card = cards[selectedId];
            if (currentRound < card.rounds.size())
                noteEditor.setText (card.rounds[currentRound].note, false);
        }
    }
    else
    {
        selectedCardForNote = -1;
        noteEditor.clear();
    }

    resized();
    repaint();
}

void BlindCardEditor::onCardClicked (int cardId)
{
    auto& manager = processorRef.getManager();
    auto phase = manager.getPhase();

    if (phase == blindcard::GamePhase::BlindTesting)
    {
        // 切換選中狀態
        if (manager.getSelectedCardId() == cardId)
            manager.deselectCard();
        else
            manager.selectCard (cardId);
    }
}

void BlindCardEditor::onCardRated (int cardId, int stars)
{
    processorRef.getManager().rateCard (cardId, stars);
}

void BlindCardEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));
}

void BlindCardEditor::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    // 控制列
    auto controlBar = bounds.removeFromTop (40);
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

    bounds.removeFromTop (10);

    // 狀態列
    statusLabel.setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (10);

    // 筆記區
    auto noteArea = bounds.removeFromBottom (80);
    noteEditor.setBounds (noteArea);
    bounds.removeFromBottom (10);

    // 卡牌區域
    auto cardArea = bounds;
    auto cards = processorRef.getManager().getCards();
    int numCards = juce::jmax (1, cards.size());
    int cardWidth = (cardArea.getWidth() - (numCards - 1) * 10) / numCards;
    cardWidth = juce::jmin (cardWidth, 150);

    for (int i = 0; i < cardComponents.size(); ++i)
    {
        if (i < cards.size())
        {
            cardComponents[i]->setBounds (cardArea.removeFromLeft (cardWidth));
            cardArea.removeFromLeft (10);
        }
    }
}
