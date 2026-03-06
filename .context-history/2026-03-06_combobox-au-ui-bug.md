# 上下文摘要 - 2026-03-06 ComboBox 導致 AU 插件 UI 無法重開

## 討論重點
- BlindCard 在 Logic Pro 中，切換語言後關閉 UI 再重開，UI 完全無法顯示
- 經過 5 輪修復嘗試才找到根因
- 同樣的 bug 也出現在 SugoiVocalFX 專案（ComboBox 選擇空間類型）

## 決策記錄
- **選擇方案**: 把語言選擇器從 `juce::ComboBox` 改為 `juce::TextButton` 組
- **原因**: ComboBox popup 創建 native NSWindow，干擾 Logic Pro 的 AUHostingServiceXPC 窗口管理
- **放棄方案**: setOpaque(true)、parentHierarchyChanged()、zero-size protection、modal cleanup — 全部無效

## 技術細節

### 根因
JUCE `ComboBox` 的下拉選單使用 native `NSWindow`。在 AU 插件環境中（AUHostingServiceXPC process），這個 native window 干擾了 Logic Pro 的窗口管理。表現為 message thread hang 或 process 被 watchdog kill。

### 關鍵發現
1. Logic Pro 不會 destroy/recreate AU editor — 只是隱藏/顯示 hosting window
2. `createEditor()` 只被呼叫一次
3. Bug 不會產生 crash report（process 被 SIGKILL）
4. File-based logging（`/tmp/`）是 AU sandbox 唯一可靠的診斷方式

### 修改檔案
- `Source/UI/Components/SettingsPanel.h` — ComboBox → OwnedArray<TextButton>
- `Source/UI/Components/SettingsPanel.cpp` — setupLanguageSelector() 完全重寫
- `Source/UI/BlindCardEditor.h` — 加 visibilityChanged(), parentHierarchyChanged()
- `Source/UI/BlindCardEditor.cpp` — 加 setOpaque(true), file logging, visibility tracking
- `Source/PluginProcessor.cpp` — 加 file logging in createEditor()

### 無效的修復嘗試（記錄供參考）
1. LOCALIZE() buttons + hideOverlay in destructor
2. Version bump 1.0.1 → 1.0.2
3. parentHierarchyChanged() + zero-size protection in resized()/paint()
4. setOpaque(true) + modal exitModalState(0) + parentHierarchyChanged 強化
5. **最終有效**: 把 ComboBox 換成 TextButton 組

## 待辦事項
- [ ] 清理 BlindCardEditor.cpp 中的診斷 logging（file-based logging）
- [ ] 檢查其他 Sugoi 插件是否有類似 ComboBox 問題
- [ ] 考慮 BlindCard Standalone 版本的 preset ComboBox 是否也需要替換（Standalone 不經過 AU，可能不受影響）
