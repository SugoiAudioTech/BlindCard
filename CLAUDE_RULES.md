# Claude 操作規範

## 絕對禁止的操作

以下操作**絕對不可以在未經用戶明確同意的情況下執行**：

1. **不要刪除 AU Cache**
   - `~/Library/Caches/AudioUnitCache`
   - 這會導致所有 DAW 需要重新掃描插件，浪費大量時間

2. **不要刪除 User Preferences**
   - `~/Library/Preferences/`
   - 這會導致應用程式設定遺失

3. **不要刪除任何系統快取或設定檔**
   - 除非用戶明確要求並同意

## 正確的做法

如果需要讓 DAW 重新讀取插件：
- 告訴用戶手動重新掃描（Logic Pro: Preferences > Plug-in Manager > Reset & Rescan）
- **不要自動刪除快取**

## 記住

- 永遠先詢問，再執行破壞性操作
- 刪除任何檔案前都要確認
- 系統檔案和快取不是我們可以隨意處理的
