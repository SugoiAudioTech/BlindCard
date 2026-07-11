## Context

BlindCard is a JUCE 8.0.12 plugin (COMPANY `Sugoi` / code `Blcd`), currently shipping AU/VST3/Standalone via `packaging/sign-and-package.sh` → `productbuild` → `notarize.sh`. AAX is already drafted in `CMakeLists.txt` (uncommitted) and a universal dev `.aaxplugin` exists but is only ad-hoc signed and absent from the pipeline. BlindCard has **no nested dylibs** (no ONNX/ORT), so it is closer to the SugoiDimension AAX case than the Kiyome one — no deep-sign-nested-dylib complexity. Tooling is confirmed present: AAX SDK 2.9.0, `wraptool` (Eden 5), iLok connected. PACE Central has no BlindCard Product/Wrap Config yet.

Reference implementation: AirCheck's `sign-and-package.sh` (the canonical AAX packaging template) and the `sugoi-audio-aax-protools` skill.

## Goals / Non-Goals

**Goals:**
- Commit a clean, correct CMake AAX config and produce a validated universal `.aaxplugin`.
- Extend the existing packaging pipeline to sign (PACE + hardened runtime), package, and notarize AAX as a 4th format in one installer.
- Keep AU/VST3/Standalone behavior byte-for-byte unchanged.

**Non-Goals:**
- No DSP/UI/parameter changes.
- No Page Table XML (S6 hardware surface mapping).
- No Windows AAX build (macOS only this round).
- No per-arch AAX installers (BlindCard already ships per-arch installers for AU/VST3, but AAX ships universal in the default installer to match the smaller scope; per-arch AAX is a possible follow-up).

## Decisions

- **Extend the existing `sign-and-package.sh` rather than fork an AAX-only script.** Rationale: one pipeline, one version source, one distribution.xml. Alternative (separate script) rejected — drifts from AU/VST3 versioning and duplicates pre-flight logic.
- **AAX signing slots in as a new step between Standalone signing and `pkgbuild`.** It runs `wraptool sign` on the built `.aaxplugin` (from the same `ARTIFACTS_DIR/AAX/`), then `pkgbuild --install-location /Library/Application Support/Avid/Audio/Plug-Ins`. Rationale: mirrors AirCheck's `[3/7] wraptool sign AAX` slot.
- **`--extrasigningoptions` is mandatory.** wraptool's internal codesign omits hardened runtime/timestamp/entitlements by default → notarization fails. Pass `--timestamp --options runtime --entitlements packaging/BlindCard.entitlements`. This is the single most common AAX-notarization failure and is a hard requirement in the spec.
- **AAX signing is gated behind pre-flight checks** (wraptool executable, iLok present via `ioreg`, `.aaxplugin` exists). If prerequisites are missing the script aborts before touching any bundle — never produces a half-signed installer.
- **Wrap GUID is injected via a script variable** (e.g. `AAX_WRAP_GUID`), sourced after the user creates the PACE Central Product + Wrap Config. Until it exists, the AAX step is the one true blocker; the script fails the pre-flight with a clear "create PACE Central Wrap Config" message.
- **Copies use `ditto`/`cp -R -H`** to preserve the PACE `dsig` symlink — a documented failure mode where a plain `cp -R` corrupts the signature and Pro Tools rejects it.
- **Validator ABORTs are accepted as tool noise.** The local Validator's Ruby `SysInfo.rb` aborts several info probes; only the core test PASSes gate release (matches skill guidance).

## Risks / Trade-offs

- **[PACE Central Wrap Config is a manual, user-only step]** → The one hard external dependency. Mitigation: script fails-fast with an actionable message; design documents the exact 6-step PACE Central flow from the skill; GUID recorded in memory once obtained.
- **[wraptool cert not yet on iLok / sync lag]** → `CouldNotFindSignerCredentials`. Mitigation: re-Synchronize iLok 2-3×; escalation path (support@paceap.com) documented in skill.
- **[Signing after build then mutating bundle corrupts dsig]** → Pro Tools shows "not valid 64 bit AAX". Mitigation: symlink-preserving copies only; never edit bundle post-sign.
- **[Universal AAX size]** → Since BlindCard has no nested dylibs the size delta is modest; acceptable for a universal-in-default-installer approach.
- **[Pro Tools Developer can't save sessions]** → Acceptance test is load/audio/UI only; full session round-trip verified later with a PACE-signed build in real Pro Tools.

## Migration Plan

1. Commit CMake AAX config; clean universal rebuild.
2. Run AAX Validator (core PASS gate).
3. **User** creates PACE Central Product + Wrap Config → Wrap GUID → recorded in memory + skill table.
4. Extend `sign-and-package.sh` (+ pre-flight) and `distribution.xml`; dry-run packaging.
5. Notarize; verify expanded `.pkg` has 4 formats.
6. Pro Tools Developer load/audio/UI acceptance.
Rollback: AAX is additive; reverting the packaging/CMake commits restores the AU/VST3/Standalone-only installer with no impact on the other formats.

## Open Questions

- Confirm the AAX Wrap GUID value once the user creates the PACE Central config (blocks the signing step only).
- Whether to also produce per-arch AAX installers to match the existing per-arch AU/VST3 installers — deferred to a follow-up unless the user wants parity now.
