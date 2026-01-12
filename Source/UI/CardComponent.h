#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Core/Types.h"

namespace blindcard
{

class CardComponent final : public juce::Component
{
public:
    CardComponent();

    void setCard (const CardSlot* card, int currentRound, bool isSelected, GamePhase phase);

    std::function<void(int)> onClicked;
    std::function<void(int, int)> onRated;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    const CardSlot* cardData = nullptr;
    int round = 0;
    bool selected = false;
    GamePhase currentPhase = GamePhase::Setup;

    juce::TextButton starButtons[5];

    void updateStars();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CardComponent)
};

} // namespace blindcard
