# Claude Operating Guidelines

## Absolutely Prohibited Operations

The following operations **must never be executed without explicit user consent**:

1. **Do not delete AU Cache**
   - `~/Library/Caches/AudioUnitCache`
   - This causes all DAWs to rescan plugins, wasting significant time

2. **Do not delete User Preferences**
   - `~/Library/Preferences/`
   - This causes application settings to be lost

3. **Do not delete any system caches or configuration files**
   - Unless the user explicitly requests and agrees

## Correct Approach

If you need DAW to re-read plugins:
- Tell the user to manually rescan (Logic Pro: Preferences > Plug-in Manager > Reset & Rescan)
- **Do not automatically delete caches**

## Remember

- Always ask before executing destructive operations
- Confirm before deleting any files
- System files and caches are not ours to handle arbitrarily
