# BlindCard 開發進度 - 2026-01-30

## 修復的 Bug

### Bug 1: Round 次數調整時不顯示計數器

**問題**：調整 Rounds 滑桿時，右側的回合計數器（如 "1/3"）沒有顯示。

**原因**：`ControlPanel::drawInfoRow()` 中的佈局邏輯問題。連續使用 `removeFromLeft()` 和 `removeFromRight()` 導致計數器區域沒有剩餘空間。

**修復**：調整 `drawInfoRow()` 的佈局順序，先用 `removeFromRight(50.0f)` 保留計數器空間。

**檔案**：`Source/UI/Components/ControlPanel.cpp:327-378`

---

### Bug 2: 盲測階段無法啟用 Level Match

**問題**：用戶在 Setup 階段忘記開啟 Level Match，進入 BlindTesting 階段後就無法再開啟。

**原因**：`BlindCardManager::startCalibration()` 只允許在 `GamePhase::Setup` 階段執行校準。

**修復**：修改條件，允許在 `Setup` 和 `BlindTesting` 兩個階段都能啟動校準。

**檔案**：`Source/Core/BlindCardManager.cpp:785-788`

```cpp
// 修改前
if (state.phase != GamePhase::Setup)
    return;

// 修改後
if (state.phase != GamePhase::Setup && state.phase != GamePhase::BlindTesting)
    return;
```

---

### Bug 3: Logic Pro 中無法使用方向鍵選擇卡牌

**問題**：在 Logic Pro 中，方向鍵被 DAW 攔截用於 transport 控制，導致無法用方向鍵導航卡牌。

**原因**：Logic Pro（以及其他一些 DAW）會優先處理方向鍵事件，插件收不到這些按鍵。

**修復**：新增多組替代快捷鍵，確保所有 DAW 都能正常操作。

**檔案**：`Source/UI/BlindCardEditor.cpp:1107-1229`

---

## 新增功能

### 替代鍵盤快捷鍵

新增以下快捷鍵，解決 Logic Pro 相容性問題：

| 快捷鍵 | 功能 | 備註 |
|--------|------|------|
| **1-8** | 直接選擇對應位置的卡牌 | 按顯示順序，非原始 track 順序 |
| **Tab** | 選擇下一張卡牌 | 循環到第一張 |
| **Shift+Tab** | 選擇上一張卡牌 | 循環到最後一張 |
| **]** | 選擇下一張卡牌 | Pro Tools 風格 |
| **[** | 選擇上一張卡牌 | Pro Tools 風格 |
| **← ↑ → ↓** | 方向鍵導航 | 保留原有功能（部分 DAW 可能無效）|

---

### Quick Start Guide 快捷鍵說明

在 Quick Start Guide（點擊右上角 `?` 開啟）中新增「Keyboard Shortcuts」區塊：

- 青色左邊框卡片樣式，與黃色「Tip」區塊呼應
- 列出所有快捷鍵及其功能
- 特別標註方向鍵在 Logic Pro 可能無效

**檔案**：
- `Source/UI/Components/QuickStartGuide.h:107` - 新增 `drawKeyboardShortcuts()` 宣告
- `Source/UI/Components/QuickStartGuide.cpp:365-410` - 實作快捷鍵區塊繪製

---

## 編譯驗證

```bash
# 編譯 AU 插件
cd /Users/Mac/Projects/BlindCard/build
cmake --build . --target BlindCard_AU -j8

# 刷新 AU 快取
killall -9 AudioComponentRegistrar
```

編譯成功，無錯誤。警告均為既有的 deprecation warnings，與本次修改無關。

---

## 測試建議

1. **Round Counter**：調整 Rounds 滑桿，確認右側顯示 "1/3"、"1/5" 等計數
2. **Level Match**：在 BlindTesting 階段點擊 Level Match 按鈕，確認可以開始校準
3. **快捷鍵**：
   - 按 1-8 測試直接選擇
   - 按 Tab/Shift+Tab 測試循環切換
   - 按 [ ] 測試 Pro Tools 風格導航
   - 在 Logic Pro 中測試以上快捷鍵
4. **Quick Start Guide**：點擊右上角 `?`，滾動到底部確認快捷鍵區塊顯示正確
