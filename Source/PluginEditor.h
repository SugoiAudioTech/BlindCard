#pragma once

#include "PluginProcessor.h"

class BlindCardEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BlindCardEditor (BlindCardProcessor&);
    ~BlindCardEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BlindCardProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardEditor)
};
