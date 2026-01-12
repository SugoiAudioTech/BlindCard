#include "PluginProcessor.h"
#include "PluginEditor.h"

BlindCardEditor::BlindCardEditor (BlindCardProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (800, 500);
}

BlindCardEditor::~BlindCardEditor()
{
}

void BlindCardEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("Blind Card", getLocalBounds(), juce::Justification::centred, 1);
}

void BlindCardEditor::resized()
{
}
