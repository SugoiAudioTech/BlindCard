## 1. CMake AAX config (build)

- [x] 1.1 Review the uncommitted `CMakeLists.txt` diff (AAX SDK path guard, `FORMATS ... AAX`, `AAX_IDENTIFIER`, `AAX_CATEGORY`) and confirm correctness
- [x] 1.2 Commit the CMake AAX configuration with a scoped commit message
- [x] 1.3 Clean universal rebuild: `cmake -B build -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'` then `cmake --build build --config Release`
- [x] 1.4 Verify `build/BlindCard_artefacts/Release/AAX/Blind Card.aaxplugin` exists and `lipo -archs` shows `arm64 x86_64`

## 2. AAX Validator verification

- [x] 2.1 Remove quarantine from the Validator tools (first-use, if needed)
- [x] 2.2 Run the full AAX Validator suite against the installed `.aaxplugin`
- [x] 2.3 Confirm core tests PASS (`describe_validation`, `load_unload`, `parameters`, `parameter_traversal.linear`, `parameter_traversal.random`)
- [x] 2.4 Record known-ignorable ABORT/FAIL results (SysInfo probes, `page_table.load`) in the change notes

## 3. PACE Central setup (user-performed, one-time)

- [x] 3.1 User: PACE Central → Products → New → `BlindCard` (Active)
- [x] 3.2 User: PACE Central → Wrap Configuration → New → `BlindCard`, link the Product via Authorization Key
- [x] 3.3 User: copy the Wrap GUID and provide it
- [x] 3.4 Record the Wrap GUID in `pace-signing-credentials.md` and the `sugoi-audio-aax-protools` skill per-plugin table

## 4. Extend packaging pipeline

- [x] 4.1 Add AAX pre-flight checks to `sign-and-package.sh` (wraptool executable, iLok connected via `ioreg`, `.aaxplugin` exists) — abort non-zero with a named-missing message
- [x] 4.2 Add an AAX signing step: `wraptool sign` with `--wcguid <AAX_WRAP_GUID>`, Developer ID Application identity, and `--extrasigningoptions "--timestamp --options runtime --entitlements ${ENTITLEMENTS}"`
- [x] 4.3 Verify the signed AAX: `wraptool verify` (Sugoi signer) and `codesign -dvvv` shows `flags=0x10000(runtime)` + `Timestamp` (not adhoc)
- [x] 4.4 Add `pkgbuild` for AAX: identifier `com.sugoi.blindcard.aax`, version `${VERSION}`, install-location `/Library/Application Support/Avid/Audio/Plug-Ins`, using symlink-preserving copy (`ditto`/`cp -R -H`)
- [x] 4.5 Add AAX build-artifact existence guard alongside the existing AU/VST3/Standalone guards
- [x] 4.6 Add AAX arch verification to the existing `verify_arch` gate
- [x] 4.7 Update `packaging/Distribution/distribution.xml`: add an AAX choice + `pkg-ref` to `BlindCard-AAX.pkg`; update the choices-outline

## 5. Build, notarize, verify installer

- [x] 5.1 Run `sign-and-package.sh` end-to-end and confirm a single signed `.pkg` is produced
- [x] 5.2 Expand the `.pkg` and confirm it contains AU, VST3, Standalone, and AAX component packages
- [x] 5.3 Notarize the installer via `notarize.sh` and staple; confirm notarization success
- [x] 5.4 `pkgutil --check-signature` on the final installer passes

## 6. Pro Tools acceptance

- [ ] 6.1 Launch Pro Tools Developer, load BlindCard AAX (appears in Insert menu)
- [ ] 6.2 Verify audio passes (non-silent, no clicks), UI renders, controls respond, bypass works across buffer sizes
- [ ] 6.3 Record the acceptance result; update the skill's "completed AAX plugins" table with BlindCard

## 7. Close-out

- [x] 7.1 Commit packaging changes (sign-and-package.sh + distribution.xml) with a scoped message
- [ ] 7.2 Archive the OpenSpec change
