#include "PluginProcessor.h"
#include "PluginEditor.h"

BlindCardProcessor::BlindCardProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // 嘗試從 DAW 取得軌道名稱
    juce::String trackName = getName();

    cardId = manager->registerInstance (this, trackName);

    if (cardId >= 0)
        manager->addChangeListener (this);
}

BlindCardProcessor::~BlindCardProcessor()
{
    if (cardId >= 0)
    {
        manager->removeChangeListener (this);
        manager->unregisterInstance (this);
    }
}

void BlindCardProcessor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // 更新靜音狀態
    auto phase = manager->getPhase();
    int selectedId = manager->getSelectedCardId();

    if (phase == blindcard::GamePhase::BlindTesting && selectedId >= 0)
    {
        // 有選中卡牌時，只有被選中的發聲
        shouldMute = (cardId != selectedId);
    }
    else
    {
        // Setup/Revealed 階段或沒有選中卡牌時，全部發聲
        shouldMute = false;
    }
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

    // 獨奏邏輯：如果應該靜音，清空 buffer
    if (shouldMute.load())
    {
        buffer.clear();
    }
    // 否則 pass-through（不做任何處理）
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
