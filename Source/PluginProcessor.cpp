#include "PluginProcessor.h"
#include "UI/BlindCardEditor.h"
#include <cmath>

BlindCardProcessor::BlindCardProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // 延遲註冊到 prepareToPlay，避免 AU validation 問題
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
    bool bypassAll = manager->isBypassAll();

    // 更新增益
    float gainDb = manager->getGainForCard (cardId);
    currentGainLinear = std::pow (10.0f, gainDb / 20.0f);

    // Bypass All 模式：全部發聲
    if (bypassAll)
    {
        shouldMute = false;
        return;
    }

    // 嚴格獨奏模式：永遠只有一軌播放
    if (selectedId >= 0)
    {
        // 有選中卡牌時，只有被選中的發聲
        shouldMute = (cardId != selectedId);
    }
    else
    {
        // 沒有選中卡牌時，全部靜音（禁止多軌同時播放）
        shouldMute = true;
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

void BlindCardProcessor::prepareToPlay (double newSampleRate, int)
{
    sampleRate = newSampleRate;

    // 只在第一次 prepareToPlay 時註冊
    if (cardId < 0)
    {
        // 使用空字串，讓 Manager 自動編號 (Track 1, Track 2, ...)
        cardId = manager->registerInstance (this, {});

        if (cardId >= 0)
            manager->addChangeListener (this);
    }
}
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
        return;
    }

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // LUFS 測量
    if (measuring.load())
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                sumSquared += static_cast<double> (data[i] * data[i]);
            }
        }
        sampleCount += numSamples * numChannels;

        // 檢查是否測量完成
        if (sampleCount >= targetSampleCount)
        {
            measuring = false;
            // 計算 LUFS (簡化版 - 基於 RMS)
            double meanSquared = sumSquared / static_cast<double> (sampleCount);
            float lufs = static_cast<float> (10.0 * std::log10 (meanSquared + 1e-10) - 0.691);
            manager->setMeasuredLUFS (cardId, lufs);
        }
    }

    // 應用增益補償
    float gainLinear = currentGainLinear.load();
    if (std::abs (gainLinear - 1.0f) > 0.0001f)
    {
        buffer.applyGain (gainLinear);
    }
}

bool BlindCardProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BlindCardProcessor::createEditor()
{
    return new BlindCard::BlindCardEditor(*this);
}

void BlindCardProcessor::getStateInformation (juce::MemoryBlock&) {}
void BlindCardProcessor::setStateInformation (const void*, int) {}

void BlindCardProcessor::updateTrackProperties (const TrackProperties& properties)
{
    // 當 DAW 傳來軌道名稱時，更新 Manager 中的卡牌名稱
    if (cardId >= 0 && properties.name.has_value())
    {
        manager->setTrackName (cardId, *properties.name);
    }
}

void BlindCardProcessor::startMeasurement (float durationSeconds)
{
    sumSquared = 0.0;
    sampleCount = 0;
    targetSampleCount = static_cast<int64_t> (sampleRate * durationSeconds * 2.0); // stereo
    measuring = true;
}

void BlindCardProcessor::stopMeasurement()
{
    measuring = false;
}

float BlindCardProcessor::getMeasurementProgress() const
{
    if (!measuring.load() || targetSampleCount == 0)
        return 0.0f;
    return static_cast<float> (sampleCount) / static_cast<float> (targetSampleCount);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BlindCardProcessor();
}
