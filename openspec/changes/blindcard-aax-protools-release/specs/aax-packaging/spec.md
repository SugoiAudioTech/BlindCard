## ADDED Requirements

### Requirement: Packaging pre-flight verifies AAX signing prerequisites
The packaging script SHALL verify that PACE signing prerequisites are present before attempting to sign the AAX bundle, failing fast with a clear message if any are missing.

#### Scenario: Missing prerequisite aborts packaging
- **WHEN** `sign-and-package.sh` runs and any of `wraptool` (executable), a connected iLok, or the built `.aaxplugin` is absent
- **THEN** the script exits non-zero before signing, naming which prerequisite is missing

### Requirement: AAX bundle is PACE-signed with a notarization-ready signature
The AAX bundle SHALL be signed with PACE Eden `wraptool` using the BlindCard Wrap GUID, and the underlying Apple codesign SHALL include hardened runtime, a secure timestamp, and the entitlements, so the bundle can be notarized.

#### Scenario: wraptool signs with hardened runtime
- **WHEN** the AAX signing step runs
- **THEN** `wraptool sign` is invoked with the BlindCard `--wcguid`, the Developer ID Application identity, and `--extrasigningoptions "--timestamp --options runtime --entitlements <BlindCard.entitlements>"`

#### Scenario: Signature verifies as PACE + hardened runtime
- **WHEN** the signed `.aaxplugin` is inspected
- **THEN** `wraptool verify` reports the Sugoi Audio signer and BlindCard product
- **AND** `codesign -dvvv` reports `flags=0x10000(runtime)` and a `Timestamp` (not `adhoc`)

### Requirement: AAX is packaged and installed to the Avid plug-ins path
The installer SHALL include an AAX component package that installs the bundle to the Avid plug-ins directory, distinct from the AU/VST3/Standalone install locations.

#### Scenario: AAX pkg targets the Avid path
- **WHEN** the AAX component package is built
- **THEN** `pkgbuild` uses identifier `com.sugoi.blindcard.aax`, the project version, and install-location `/Library/Application Support/Avid/Audio/Plug-Ins`
- **AND** `distribution.xml` declares an AAX choice referencing `com.sugoi.blindcard.aax` and a `pkg-ref` to `BlindCard-AAX.pkg`

### Requirement: The distributed installer is signed and notarized with AAX included
The final distribution package SHALL be productsigned and Apple-notarized as a single installer that contains the AAX package alongside AU, VST3, and Standalone.

#### Scenario: Notarized installer contains four formats
- **WHEN** the release installer is built and notarized
- **THEN** the signed `.pkg` passes `pkgutil --check-signature` and Apple notarization
- **AND** expanding the `.pkg` shows AU, VST3, Standalone, and AAX component packages

### Requirement: AAX signature integrity is preserved after signing
Once PACE-signed, the AAX bundle contents SHALL NOT be mutated by non-symlink-preserving copies, so the PACE `dsig` data remains intact and Pro Tools accepts the bundle.

#### Scenario: Copies preserve PACE dsig
- **WHEN** the signed `.aaxplugin` is moved or copied during packaging
- **THEN** symlink-preserving copies (`ditto` or `cp -R -H`) are used, and the resulting bundle still passes `wraptool verify`
