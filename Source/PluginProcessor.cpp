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

    // 計算淡入淡出的增益步進量（每個 sample 的變化）
    // 例如 10ms @ 44100Hz = 441 samples，所以 step = 1.0 / 441
    float fadeSamples = static_cast<float> (sampleRate * kFadeTimeMs / 1000.0);
    muteGainStep = 1.0f / fadeSamples;

    // 只在第一次 prepareToPlay 時註冊
    if (cardId < 0)
    {
        // 使用緩存的軌道名稱（如果 DAW 已經傳來）
        // 如果沒有，Manager 會自動編號 (Track 1, Track 2, ...)
        cardId = manager->registerInstance (this, cachedTrackName);

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

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // LUFS 測量（校準用）- 必須在靜音檢查之前，測量原始輸入信號
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

    // 獨奏邏輯：設定目標增益（0=靜音, 1=正常播放）
    targetMuteGain = shouldMute.load() ? 0.0f : 1.0f;

    // 即時 RMS 計算（給 UI 顯示用）- 只計算未靜音的軌道
    {
        float blockSumSquared = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
                blockSumSquared += data[i] * data[i];
        }
        float blockRMS = std::sqrt (blockSumSquared / static_cast<float> (numSamples * numChannels));

        // 指數平滑（避免跳動太快）
        rmsSmoothed = rmsSmoothed + kRMSSmoothingCoeff * (blockRMS - rmsSmoothed);

        // 轉換為 dB
        float rmsDb = (rmsSmoothed > 1e-10f)
            ? 20.0f * std::log10 (rmsSmoothed)
            : -100.0f;
        currentRMSdB = rmsDb;
    }

    // 應用增益：結合 Level-Match 補償增益和靜音淡入淡出
    float gainLinear = currentGainLinear.load();

    // 逐 sample 應用平滑的靜音增益（防止切換時爆音）
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer (ch);

        // 每個 channel 重置 muteGain 到 block 開頭的值（第一個 channel 時不重置）
        float localMuteGain = muteGain;

        for (int i = 0; i < numSamples; ++i)
        {
            // 平滑過渡到目標增益
            if (localMuteGain < targetMuteGain)
            {
                localMuteGain += muteGainStep;
                if (localMuteGain > targetMuteGain)
                    localMuteGain = targetMuteGain;
            }
            else if (localMuteGain > targetMuteGain)
            {
                localMuteGain -= muteGainStep;
                if (localMuteGain < targetMuteGain)
                    localMuteGain = targetMuteGain;
            }

            // 應用總增益（Level-Match * 靜音淡入淡出）
            data[i] *= gainLinear * localMuteGain;
        }

        // 只在第一個 channel 時更新成員變數
        if (ch == 0)
            muteGain = localMuteGain;
    }

    // 如果完全靜音，重置 RMS
    if (muteGain < 0.0001f)
    {
        currentRMSdB = -100.0f;
        rmsSmoothed = 0.0f;
    }
}

bool BlindCardProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BlindCardProcessor::createEditor()
{
    return new BlindCard::BlindCardEditor(*this);
}

void BlindCardProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // 儲存 Q&A 問題數設定
    juce::ValueTree state ("BlindCardState");
    state.setProperty ("qaQuestionCount", manager->getQAQuestionCount(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml != nullptr)
        copyXmlToBinary (*xml, destData);
}

void BlindCardProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // 載入 Q&A 問題數設定
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr)
    {
        juce::ValueTree state = juce::ValueTree::fromXml (*xml);
        if (state.isValid() && state.hasType ("BlindCardState"))
        {
            int qaCount = state.getProperty ("qaQuestionCount", 5);
            manager->setQAQuestionCount (qaCount);
        }
    }
}

void BlindCardProcessor::updateTrackProperties (const TrackProperties& properties)
{
    // 當 DAW 傳來軌道名稱時
    if (properties.name.has_value())
    {
        // 緩存名稱（以防 prepareToPlay 尚未被呼叫）
        cachedTrackName = *properties.name;

        // 如果已註冊，更新 Manager 中的卡牌名稱
        if (cardId >= 0)
        {
            manager->setTrackName (cardId, cachedTrackName);
        }
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
