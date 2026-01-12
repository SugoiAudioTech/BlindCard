#include "PluginProcessor.h"
#include "PluginEditor.h"

BlindCardProcessor::BlindCardProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

BlindCardProcessor::~BlindCardProcessor()
{
}

const juce::String BlindCardProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BlindCardProcessor::acceptsMidi() const { return false; }
bool BlindCardProcessor::producesMidi() const { return false; }
bool BlindCardProcessor::isMidiEffect() const { return false; }
double BlindCardProcessor::getTailLengthSeconds() const { return 0.0; }

int BlindCardProcessor::getNumPrograms() { return 1; }
int BlindCardProcessor::getCurrentProgram() { return 0; }
void BlindCardProcessor::setCurrentProgram (int) {}
const juce::String BlindCardProcessor::getProgramName (int) { return {}; }
void BlindCardProcessor::changeProgramName (int, const juce::String&) {}

void BlindCardProcessor::prepareToPlay (double, int) {}
void BlindCardProcessor::releaseResources() {}

bool BlindCardProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void BlindCardProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 目前先直接 pass-through，之後加入獨奏邏輯
}

bool BlindCardProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BlindCardProcessor::createEditor()
{
    return new BlindCardEditor (*this);
}

void BlindCardProcessor::getStateInformation (juce::MemoryBlock&) {}
void BlindCardProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BlindCardProcessor();
}
