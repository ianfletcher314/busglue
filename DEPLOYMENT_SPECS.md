# BusGlue AU Plugin Deployment Specifications

This document defines the deployment specifications and tests for deploying the BusGlue Audio Unit plugin to Logic Pro on macOS.

## Plugin Identification

| Property | Value |
|----------|-------|
| Plugin Name | Bus Glue |
| Bundle ID | com.ianfletcheraudio.busglue |
| AU Type | aufx (Audio Effect) |
| Subtype Code | BusG |
| Manufacturer Code | Flet |
| Version | 1.0.0 |

---

## Pre-Deployment Checks

### 1. Build Success Check

| Field | Value |
|-------|-------|
| **Check Name** | AU Build Succeeds |
| **Command** | `xcodebuild -project /Users/ianfletcher/busglue/Builds/MacOSX/BusGlue.xcodeproj -scheme "BusGlue - AU" -configuration Release build` |
| **Expected Result** | Exit code 0, output contains "BUILD SUCCEEDED" |
| **Failure Action** | Review build errors in Xcode, check for missing headers/frameworks, verify JUCE module paths |

### 2. Build Warnings Check

| Field | Value |
|-------|-------|
| **Check Name** | No Critical Build Warnings |
| **Command** | `xcodebuild -project /Users/ianfletcher/busglue/Builds/MacOSX/BusGlue.xcodeproj -scheme "BusGlue - AU" -configuration Release build 2>&1 \| grep -c "warning:"` |
| **Expected Result** | 0 (no warnings) or acceptable known warnings |
| **Failure Action** | Review warnings, address deprecated API usage, fix potential issues |

### 3. Component Bundle Exists

| Field | Value |
|-------|-------|
| **Check Name** | AU Component Bundle Created |
| **Command** | `ls -la /Users/ianfletcher/busglue/Builds/MacOSX/build/Release/BusGlue.component` |
| **Expected Result** | Directory exists with Contents/ subdirectory |
| **Failure Action** | Verify AU is enabled in pluginFormats in .jucer file, regenerate Xcode project with Projucer |

### 4. Binary Architecture Check

| Field | Value |
|-------|-------|
| **Check Name** | Correct Architecture |
| **Command** | `file /Users/ianfletcher/busglue/Builds/MacOSX/build/Release/BusGlue.component/Contents/MacOS/BusGlue` |
| **Expected Result** | Contains "Mach-O 64-bit bundle arm64" (for Apple Silicon) or "universal binary" |
| **Failure Action** | Check Xcode build settings for ARCHS, ensure building for correct architecture |

---

## File System Checks

### 5. Components Directory Exists

| Field | Value |
|-------|-------|
| **Check Name** | AU Components Directory Accessible |
| **Command** | `ls -la ~/Library/Audio/Plug-Ins/Components/` |
| **Expected Result** | Directory exists and is writable |
| **Failure Action** | Create directory: `mkdir -p ~/Library/Audio/Plug-Ins/Components/` |

### 6. Sufficient Disk Space

| Field | Value |
|-------|-------|
| **Check Name** | Adequate Disk Space |
| **Command** | `df -h ~/Library \| tail -1 \| awk '{print $4}'` |
| **Expected Result** | At least 100MB free |
| **Failure Action** | Free up disk space before deployment |

### 7. No File Lock on Target

| Field | Value |
|-------|-------|
| **Check Name** | Target Not Locked by DAW |
| **Command** | `lsof ~/Library/Audio/Plug-Ins/Components/BusGlue.component/Contents/MacOS/BusGlue 2>/dev/null` |
| **Expected Result** | No output (file not in use) |
| **Failure Action** | Close Logic Pro and any other DAWs using the plugin |

---

## Code Signing Specifications

### 8. Code Signature Valid

| Field | Value |
|-------|-------|
| **Check Name** | AU Has Valid Signature |
| **Command** | `codesign -dvv /Users/ianfletcher/busglue/Builds/MacOSX/build/Release/BusGlue.component 2>&1` |
| **Expected Result** | Shows valid signature (adhoc or Developer ID) |
| **Failure Action** | Sign with: `codesign --force --deep --sign - /path/to/BusGlue.component` |

### 9. Code Signature Verification

| Field | Value |
|-------|-------|
| **Check Name** | Signature Passes Verification |
| **Command** | `codesign --verify --verbose /Users/ianfletcher/busglue/Builds/MacOSX/build/Release/BusGlue.component` |
| **Expected Result** | "valid on disk" message, exit code 0 |
| **Failure Action** | Re-sign the component, check for unsigned nested frameworks |

---

## AU Validation Specifications

### 10. AU Validation - Basic

| Field | Value |
|-------|-------|
| **Check Name** | auval Basic Validation |
| **Command** | `auval -v aufx BusG Flet` |
| **Expected Result** | All tests pass, no FAIL results |
| **Failure Action** | Review auval output for specific failures, common issues: incorrect channel configs, missing required properties |

### 11. AU Validation - Strict Mode

| Field | Value |
|-------|-------|
| **Check Name** | auval Strict Validation |
| **Command** | `auval -v aufx BusG Flet -strict` |
| **Expected Result** | All tests pass with no warnings |
| **Failure Action** | Address all warnings as they may become failures in future macOS versions |

### 12. AU Listed in System

| Field | Value |
|-------|-------|
| **Check Name** | AU Appears in Component List |
| **Command** | `auval -a \| grep -i "BusG.*Flet"` |
| **Expected Result** | Shows "aufx BusG Flet - Fletcher: Bus Glue" |
| **Failure Action** | Clear AU cache, rescan, verify Info.plist has correct AU metadata |

---

## Deployment Steps

### 13. Remove Old Version

| Field | Value |
|-------|-------|
| **Check Name** | Clean Old Installation |
| **Command** | `rm -rf ~/Library/Audio/Plug-Ins/Components/BusGlue.component` |
| **Expected Result** | Old component removed (exit code 0) |
| **Failure Action** | Close any DAWs using the plugin, use sudo if permissions issue |

### 14. Copy New Component

| Field | Value |
|-------|-------|
| **Check Name** | Deploy New Component |
| **Command** | `cp -R /Users/ianfletcher/busglue/Builds/MacOSX/build/Release/BusGlue.component ~/Library/Audio/Plug-Ins/Components/` |
| **Expected Result** | Exit code 0, component exists at destination |
| **Failure Action** | Check source exists, verify write permissions on destination |

### 15. Clear AU Cache

| Field | Value |
|-------|-------|
| **Check Name** | Clear AudioUnit Cache |
| **Command** | `killall -9 AudioComponentRegistrar 2>/dev/null; rm -rf ~/Library/Caches/AudioUnitCache/` |
| **Expected Result** | Cache cleared (commands may fail if not running/existing - OK) |
| **Failure Action** | Reboot system if cache persists |

---

## Post-Deployment Verification

### 16. Deployed Component Exists

| Field | Value |
|-------|-------|
| **Check Name** | Verify Deployment |
| **Command** | `ls -la ~/Library/Audio/Plug-Ins/Components/BusGlue.component/Contents/MacOS/BusGlue` |
| **Expected Result** | File exists with correct size (approximately same as source) |
| **Failure Action** | Re-copy component, verify no copy errors |

### 17. Deployed Component Signed

| Field | Value |
|-------|-------|
| **Check Name** | Deployed Signature Valid |
| **Command** | `codesign --verify --verbose ~/Library/Audio/Plug-Ins/Components/BusGlue.component` |
| **Expected Result** | "valid on disk" |
| **Failure Action** | Signature may be stripped during copy - re-sign at destination |

### 18. Post-Deploy AU Validation

| Field | Value |
|-------|-------|
| **Check Name** | Deployed AU Passes Validation |
| **Command** | `auval -v aufx BusG Flet` |
| **Expected Result** | All tests pass |
| **Failure Action** | Check deployment didn't corrupt files, compare with source |

---

## Logic Pro Integration Tests

### 19. Logic Pro Plugin Scan

| Field | Value |
|-------|-------|
| **Check Name** | Logic Pro Recognizes Plugin |
| **Verification Method** | Launch Logic Pro, open Preferences > Plug-in Manager, search for "Bus Glue" |
| **Expected Result** | Plugin appears in list, shows "Compatible" or checkmark |
| **Failure Action** | Click "Reset & Rescan Selection", check AU validation log in Console.app |

### 20. Logic Pro Plugin Instantiation

| Field | Value |
|-------|-------|
| **Check Name** | Plugin Loads in Logic Pro |
| **Verification Method** | Create new project, add Audio Effect on channel strip, select Audio Units > Fletcher > Bus Glue |
| **Expected Result** | Plugin window opens, UI displays correctly |
| **Failure Action** | Check Console.app for crashes, verify all resources are bundled |

### 21. Logic Pro Audio Processing

| Field | Value |
|-------|-------|
| **Check Name** | Plugin Processes Audio |
| **Verification Method** | Route audio through plugin, verify meters move and audio passes |
| **Expected Result** | Audio passes through, meters respond, no clicks/pops/distortion |
| **Failure Action** | Check sample rate compatibility, verify DSP code handles buffer sizes |

### 22. Logic Pro Parameter Automation

| Field | Value |
|-------|-------|
| **Check Name** | Automation Works |
| **Verification Method** | Automate Threshold parameter in Logic Pro, play back |
| **Expected Result** | Parameter moves with automation, smooth transitions |
| **Failure Action** | Verify parameter IDs are consistent between sessions |

### 23. Logic Pro Session Save/Load

| Field | Value |
|-------|-------|
| **Check Name** | State Persistence |
| **Verification Method** | Set plugin parameters, save Logic project, close and reopen |
| **Expected Result** | All parameter values restored correctly |
| **Failure Action** | Check getStateInformation/setStateInformation implementation |

### 24. Logic Pro CPU Usage

| Field | Value |
|-------|-------|
| **Check Name** | Acceptable CPU Usage |
| **Verification Method** | Monitor Logic Pro CPU meter with plugin active at 128 buffer size |
| **Expected Result** | CPU usage reasonable (<5% per instance on modern Mac) |
| **Failure Action** | Profile DSP code, optimize hot paths |

---

## Automated Test Script

Run all automated checks with:

```bash
/Users/ianfletcher/busglue/scripts/deploy-to-logic.sh
```

---

## Troubleshooting Guide

### Plugin Not Appearing in Logic Pro

1. Verify AU validation passes: `auval -v aufx BusG Flet`
2. Clear AU cache: `rm -rf ~/Library/Caches/AudioUnitCache/`
3. Kill cache daemon: `killall -9 AudioComponentRegistrar`
4. Check Logic Pro Plugin Manager for failed/blocked plugins
5. Look for validation errors in Console.app (filter: "AudioUnit")

### Plugin Crashes on Load

1. Check Console.app for crash logs
2. Verify all JUCE modules linked correctly
3. Test standalone version first: `open /path/to/BusGlue.app`
4. Run with sanitizers in Debug build

### Audio Glitches

1. Increase buffer size in Logic Pro
2. Check for denormals in DSP code
3. Verify no allocations in processBlock()
4. Profile with Instruments

### Code Signing Issues

1. For development: `codesign --force --deep --sign - BusGlue.component`
2. For distribution: Use Developer ID Application certificate
3. Notarization required for distribution outside Mac App Store

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-01-31 | Initial deployment specifications |
