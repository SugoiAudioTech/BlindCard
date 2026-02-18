/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "CardComponent.h"
#include "Localization/LocalizationManager.h"
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

    // Manual gain slider
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

    // LUFS display
    lufsLabel.setJustificationType (juce::Justification::centred);
    lufsLabel.setFont (juce::Font (11.0f));
    lufsLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (lufsLabel);

    // Note editor
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
    // Don't let TextEditor intercept spacebar, pass to DAW for play/pause
    noteEditor.addKeyListener (this);
    addAndMakeVisible (noteEditor);

    // Guess dropdown menu
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

    // Q&A selection button
    selectButton.setButtonText (LOCALIZE(CardSelect)); // Select this
    selectButton.setColour (juce::TextButton::buttonColourId, juce::Colours::orange.darker());
    selectButton.onClick = [this]()
    {
        if (onQASubmit && hasCard)
            onQASubmit (cardData.id);
    };
    addAndMakeVisible (selectButton);

    // Initialize animation values
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

    // Update Q&A state
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

    // Update selection state (set as primary based on isSelected)
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

    // Ensure animation timer is running
    if (!isTimerRunning())
        startTimerHz (60);
}

void CardComponent::startFlip()
{
    if (flipState == FlipState::Flipping)
        return;

    flipState = FlipState::Flipping;
    auto& params = getDebugParams();

    // Toggle flip state: if face up, flip to back, otherwise flip to front
    float currentValue = flipProgress.getValue();
    float targetValue = (currentValue > 0.5f) ? 0.0f : 1.0f;

    flipProgress.setTarget (targetValue, params.flipDuration);

    // Trigger glow flash when flipping to front
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

    // Flash: 0.3 → 1.0 → 0.3
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
            // Primary alpha is fixed
            targetAlpha = params.glowAlphaPrimary;
            break;
        case SelectionState::Secondary:
            // Secondary alpha uses multi-card config (increase when glow shrinks to maintain visibility)
            targetAlpha = currentMultiCardConfig.subAlpha;
            break;
        case SelectionState::None:
            targetAlpha = 0.0f;
            break;
    }

    // If selected, maintain idle glow
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

    // Update flip animation
    if (flipProgress.update (deltaTime))
    {
        needsRepaint = true;
    }

    // Check if flip animation is complete (independent of update return value)
    if (flipState == FlipState::Flipping)
    {
        float progress = flipProgress.getValue();
        float target = flipProgress.getTarget();

        // Animation complete: value has reached near target
        if (std::abs (progress - target) < 0.01f)
        {
            flipState = target > 0.5f ? FlipState::FaceUp : FlipState::FaceDown;
        }
    }

    // Update glow flash
    if (isFlashActive)
    {
        flashPhase += deltaTime / params.flashDuration;

        if (flashPhase >= 1.0f)
        {
            // Flash complete, return to target value
            isFlashActive = false;
            flashPhase = 0.0f;
            updateGlowTarget();
        }
        else if (flashPhase >= 0.5f && glowAlpha.getTarget() > 0.9f)
        {
            // Flash midpoint, start descending
            updateGlowTarget();
        }

        needsRepaint = true;
    }

    // Update glow alpha
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
        // No animation needs updating, stop timer
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

    // scaleX from 1 → 0 → 1 (flip in middle)
    float scaleX;
    if (progress < 0.5f)
    {
        // First half: 1 → 0
        scaleX = 1.0f - progress * 2.0f;
    }
    else
    {
        // Second half: 0 → 1
        scaleX = (progress - 0.5f) * 2.0f;
    }

    // Ensure minimum scale is not zero
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

    // Apply multi-card scene glow radius scale
    scale *= currentMultiCardConfig.glowRadiusScale;

    // Calculate glow area (expand to 1.3x card size, then multiply by multi-card scale)
    float expandX = bounds.getWidth() * (scale - 1.0f) / 2.0f;
    float expandY = bounds.getHeight() * (scale - 1.0f) / 2.0f;
    auto glowBounds = bounds.expanded (expandX, expandY);

    // Choose glow image (warm gold for flash, cool white-gold for selection)
    // Use Compact version based on card width
    bool useCompact = bounds.getWidth() < 70.0f;
    bool use2x = bounds.getWidth() > 100.0f;  // Use @2x for larger sizes

    const juce::Image& glowImage = isFlashActive
        ? (useCompact ? assets.getGlowWarmCompact (use2x) : assets.getGlowWarm (use2x))
        : (useCompact ? assets.getGlowCoolCompact (use2x) : assets.getGlowCool (use2x));

    if (glowImage.isValid())
    {
        // Draw using PNG texture
        g.setOpacity (alpha);
        g.drawImage (glowImage,
                     glowBounds,
                     juce::RectanglePlacement::centred);
        g.setOpacity (1.0f);
    }
    else
    {
        // Fallback: programmatic gradient (when assets not loaded)
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
    // Only draw gold border when selected
    if (selectionState == SelectionState::None && !selected)
    {
        // Draw normal border when not selected
        g.setColour (juce::Colours::grey);
        g.drawRoundedRectangle (bounds.reduced (1), 10.0f, 2.0f);
        return;
    }

    // Gold border: adjust alpha based on selection state
    float borderAlpha = 1.0f;
    if (selectionState == SelectionState::Secondary)
        borderAlpha = 0.5f;

    // Cool white-gold border
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

    // Switch display content mid-flip animation (Revealed phase only)
    bool showFront = flipProgress.getValue() > 0.5f;

    switch (currentPhase)
    {
        case GamePhase::Setup:
            // Setup: show real track name
            displayText = cardData.realTrackName;
            break;

        case GamePhase::BlindTesting:
            // Blind testing: always show code (A, B, C...), hide real name
            displayText = juce::String::charToString ('A' + cardData.displayPosition);
            break;

        case GamePhase::Revealed:
            // Revealed: flip animation shows real name
            if (showFront)
                displayText = cardData.realTrackName;
            else
                displayText = juce::String::charToString ('A' + cardData.displayPosition);
            break;
    }

    g.drawFittedText (displayText,
                      bounds.toNearestInt().reduced (10, 10).withHeight (30),
                      juce::Justification::centred, 1);

    // Show average rating when revealed
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

    // Layer 1: Glow layer (below card)
    paintGlowLayer (g, bounds);

    // Apply flip transform
    if (flipState == FlipState::Flipping)
    {
        auto transform = getFlipTransform (bounds);
        g.addTransform (transform);

        // Shadow offset (based on flip progress)
        float progress = flipProgress.getValue();
        float shadowOffset = std::sin (progress * juce::MathConstants<float>::pi) * 5.0f;

        // Draw shadow
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (bounds.translated (shadowOffset, shadowOffset), 10.0f);
    }

    // Layer 2: Card base
    paintCardBase (g, bounds);

    // Layer 3: Gold border decoration
    paintGoldBorder (g, bounds);

    // Card content
    paintCardContent (g, bounds);
}

void CardComponent::resized()
{
    auto bounds = getLocalBounds().reduced (5);

    // Star rating at bottom
    auto starArea = bounds.removeFromBottom (25);
    int starWidth = starArea.getWidth() / 5;
    for (int i = 0; i < 5; ++i)
    {
        starButtons[i].setBounds (starArea.removeFromLeft (starWidth));
    }

    // Q&A selection button (same area as stars)
    auto qaButtonArea = getLocalBounds().reduced (5).removeFromBottom (28);
    selectButton.setBounds (qaButtonArea);

    // Guess dropdown (shown during blind testing, above stars)
    bounds.removeFromBottom (3);
    auto guessArea = bounds.removeFromBottom (24);
    guessComboBox.setBounds (guessArea);

    // Gain slider (shown when not blind testing)
    bounds.removeFromBottom (3);
    auto gainArea = bounds.removeFromBottom (22);
    gainSlider.setBounds (gainArea);

    // LUFS display (shown when not blind testing)
    bounds.removeFromBottom (2);
    auto lufsArea = bounds.removeFromBottom (16);
    lufsLabel.setBounds (lufsArea);

    // Note editor (shown during blind testing)
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

bool CardComponent::keyPressed (const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    // Intercept spacebar, don't let TextEditor handle it, allow DAW to play/pause
    if (key.getKeyCode() == juce::KeyPress::spaceKey)
    {
        return true;  // Return true means handled, prevent TextEditor from inserting space
    }

    return false;  // Let TextEditor handle other keys normally
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

        // If in secondary selection state, update glow alpha target
        if (selectionState == SelectionState::Secondary)
        {
            updateGlowTarget();
        }

        repaint();
    }
}

} // namespace blindcard
