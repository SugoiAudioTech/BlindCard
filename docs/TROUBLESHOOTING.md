# BlindCard Troubleshooting Guide

**Version 1.0.4 | Sugoi Audio**

This guide covers common issues and solutions for BlindCard on Windows and macOS.

---

## Table of Contents

1. [Installation & Loading](#1-installation--loading)
2. [Display & UI](#2-display--ui)
3. [Audio & Playback](#3-audio--playback)
4. [Level Matching](#4-level-matching)
5. [Standalone Mode](#5-standalone-mode)
6. [DAW-Specific Issues](#6-daw-specific-issues)
7. [Performance](#7-performance)

---

## 1. Installation & Loading

### Plugin does not appear in DAW after installation

**Windows:**
- Confirm the `.vst3` bundle is in `C:\Program Files\Common Files\VST3\`
- Restart the DAW and re-scan the VST3 folder
- Some DAWs (e.g. FL Studio) require explicitly adding the VST3 search path in plugin settings

**macOS:**
- AU: Located in `~/Library/Audio/Plug-Ins/Components/`
- VST3: Located in `~/Library/Audio/Plug-Ins/VST3/`
- In Logic Pro, open **Audio Units Manager** and click **Reset & Rescan**
- If macOS blocks the plugin: go to **System Settings > Privacy & Security** and click **Allow Anyway**

### Plugin fails AU validation in Logic Pro

- Ensure you are using the latest version of BlindCard
- Try removing the cached AU component and re-scanning:
  ```
  rm ~/Library/Caches/AudioUnitCache/com.apple.audiounits.cache
  ```
- Restart Logic Pro

---

## 2. Display & UI

### Plugin UI looks blurry in Ableton Live (Windows)

This is a known limitation of Ableton Live's DPI handling on Windows. Live does not support per-monitor DPI awareness, so Windows applies bitmap scaling to the entire window, causing the plugin to appear blurry.

**Workaround:**
1. Close Ableton Live
2. Right-click the Ableton Live `.exe` file (e.g. `Ableton Live 12 Suite.exe`)
3. Select **Properties > Compatibility > Change high DPI settings**
4. Check **Override high DPI scaling behavior**
5. Set **Scaling performed by:** to **Application**
6. Click OK and restart Live

> **Note:** This issue does not affect Reaper, FL Studio, Cubase, Studio One, or other DAWs that correctly handle DPI scaling.

### Plugin window is too small / too large

- BlindCard supports resizing. Drag the bottom-right corner to resize the window.
- Minimum size: 900 x 750 pixels
- If the window appears off-screen, try deleting the DAW's plugin window position cache and reloading the project.

### Text or labels appear garbled or as rectangles

- This may indicate a font rendering issue. BlindCard bundles its own fonts (Noto Sans TC, Bebas Neue, Cinzel) so system fonts should not be required.
- Try updating your GPU drivers.
- On Windows, ensure DirectWrite is not disabled.

---

## 3. Audio & Playback

### No audio when a card is selected

- **Plugin mode:** BlindCard routes the audio from your DAW track. Ensure the track has audio playing and routing is correct.
- **Standalone mode:** Ensure you have loaded audio files into the card slots and pressed Play.
- Check that the card is not muted by the blind test logic — during BlindTesting phase, only the selected card plays audio.

### Clicks or pops when switching cards

- BlindCard uses a configurable crossfade (1-100 ms) to prevent clicks. Try increasing the **Crossfade** value (e.g. 20-50 ms).
- If clicks persist, this may be caused by audio buffer underruns. Try increasing your DAW's audio buffer size.

### Audio cuts out or drops occasionally

- Increase the audio buffer size in your DAW or audio interface settings.
- Close other CPU-intensive applications.
- On Windows, ensure your audio driver is set to ASIO for lowest latency and best stability.

---

## 4. Level Matching

### Level Match / Auto Gain is not calibrating

- Level Match requires audio to be playing during calibration. Ensure all tracks are playing audio when you enable Auto Gain.
- In **Plugin mode**, calibration measures the live input signal — play the project during calibration.
- In **Standalone mode**, calibration scans the loaded files offline (no playback required).

### Measured LUFS values seem inaccurate

- BlindCard uses EBU R128 integrated loudness (LUFS) with gating. Very quiet tracks or tracks with long silence will produce lower LUFS readings — this is correct behavior.
- For best results, ensure all tracks contain similar content duration.

---

## 5. Standalone Mode

### Audio file fails to load

- Supported formats: **WAV, MP3, AIFF**
- Maximum file size: 1 GB
- Ensure the file is not corrupted or DRM-protected.
- Try converting the file to WAV using another application and loading again.

### Playback is out of sync between cards

- BlindCard uses sample-accurate synchronized playback. All cards share the same playhead position.
- If files have different sample rates, BlindCard resamples on-the-fly. This should be transparent, but extremely high sample rate ratios (e.g. 22 kHz vs 192 kHz) may use more CPU.

### No audio output in Standalone mode

- Check that the correct audio output device is selected (Settings panel > Audio Device).
- On Windows, WASAPI Shared mode is used by default. If you experience issues, try selecting a different output device.

---

## 6. DAW-Specific Issues

### Reaper

- BlindCard works well in Reaper with no known issues.
- For Level Matching in plugin mode, ensure each BlindCard instance is on a separate track with audio routed to it.

### FL Studio

- If the plugin does not appear, go to **Options > Manage Plugins > Plugin search paths** and add `C:\Program Files\Common Files\VST3\`.
- Click **Find plugins** to scan.

### Ableton Live

- **Blurry UI on Windows:** See [Plugin UI looks blurry in Ableton Live](#plugin-ui-looks-blurry-in-ableton-live-windows) above.
- Live does not support VST3 per-note-expression passthrough, but this does not affect BlindCard.

### Logic Pro

- If the plugin shows as **incompatible**, try resetting the AU cache (see [Plugin fails AU validation](#plugin-fails-au-validation-in-logic-pro)).
- Logic Pro may intercept certain keyboard shortcuts. Use number keys (1-8) or bracket keys ([ ]) for card switching.

### Cubase / Studio One / Bitwig

- No known issues. BlindCard should work as expected.

---

## 7. Performance

### High CPU usage

- BlindCard is lightweight and should use minimal CPU. If you notice high CPU usage:
  - Ensure you are using the **Release** build, not Debug.
  - In Standalone mode with many long files loaded, the initial LUFS scan may briefly use CPU. This runs in the background and completes quickly.
  - Close and reopen the plugin if CPU usage remains unexpectedly high.

### Plugin causes DAW to hang on close

- This is rare. If it happens, ensure you are using the latest version of BlindCard.
- Report the issue with your DAW name and version at [github.com/SugoiAudioTech/BlindCard/issues](https://github.com/SugoiAudioTech/BlindCard/issues).

---

## Still need help?

- **Website:** [sugoiaudio.com](https://sugoiaudio.com)
- **GitHub Issues:** [github.com/SugoiAudioTech/BlindCard/issues](https://github.com/SugoiAudioTech/BlindCard/issues)
- **User Manual:** See `docs/USER_MANUAL.md` for detailed feature documentation.

---

*BlindCard is free and open-source software, licensed under AGPL-3.0.*
