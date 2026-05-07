# flutter_windows_iap — Build Guide

How to build, test, and run the plugin on a development machine.

---

## Prerequisites

| Tool | Version | Notes |
|---|---|---|
| Flutter | ≥ 3.22 | `flutter doctor` should show Windows desktop enabled |
| Visual Studio 2022 | with "Desktop development with C++" workload | Required for C++20 + WinRT headers |
| Windows SDK | 10.0.19041.0 or later | Installed as part of the VS workload |
| Dart SDK | bundled with Flutter | |

### Verify setup

```bash
flutter doctor -v
```

Ensure "Windows (desktop)" shows ✓ with no errors. The key item is Visual Studio
2022 with the correct workload.

---

## Quick start

```bash
# 1. Install Dart dependencies
flutter pub get

# 2. Static analysis (must be clean)
dart analyze .

# 3. Dart unit tests
flutter test

# 4. Run the example app
cd example
flutter run -d windows
```

---

## Dart unit tests (no C++ build needed)

```bash
# From repo root:
flutter test

# Specific file:
flutter test test/flutter_windows_iap_method_channel_test.dart
```

Tests use a mock `MethodChannel` — no real Windows Store or C++ plugin needed.

---

## C++ unit tests

The C++ tests build as part of the example project's CMake configuration.

```bash
# Step 1: Build the example project (generates the .sln / CMake output)
cd example
flutter build windows --debug

# Step 2: Run ctest from the build output directory
cd build/windows/x64
ctest --test-dir . -C Debug --output-on-failure
```

Alternatively, open `example/build/windows/x64/ALL_BUILD.vcxproj` (or the
generated `.sln`) in Visual Studio 2022 and run the `flutter_windows_iap_test`
target directly.

**Note:** The C++ tests use `HWND = nullptr`. They do not require MSIX packaging
or a real Store context.

---

## Running the example app

The example app demonstrates the IAP API. For a meaningful test you need:
- MSIX packaging (see below) or an unpackaged run (which will get Store errors)
- Real SKU IDs in `example/lib/main.dart` (currently placeholder values — see TODO T1-2)

```bash
cd example
flutter run -d windows
```

The example app will launch. The "Query products" button will call `queryProducts`
and show results (or an empty list / error if SKUs are not configured).

---

## MSIX packaging (for real Store testing)

### Developer certificate (local testing)

1. In Visual Studio, right-click the example project → Publish → Create App Packages.
2. Choose "Sideloading" → create a self-signed test certificate.
3. Install the certificate to "Local Machine > Trusted People".
4. Build the MSIX and install it.

### Store submission package

```bash
cd example
flutter build windows --release
```

Then use the MSIX Packaging Tool or Visual Studio to package the release output.
See the
[Microsoft docs](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/deploy-packaged-apps)
for full packaging instructions.

---

## Integration tests

Requires MSIX packaging. See [doc/testing.md](../testing.md) Layer 3 for details.

```bash
cd example
flutter run --device-id windows integration_test/plugin_integration_test.dart
```

---

## Build matrix at a glance

| Task | Command | Requires MSIX |
|---|---|---|
| Static analysis | `dart analyze .` | No |
| Dart unit tests | `flutter test` | No |
| C++ unit tests | `ctest` (after `flutter build windows`) | No |
| Run example | `cd example && flutter run -d windows` | No (but Store features won't work) |
| Integration tests | `flutter run ...integration_test...` | Yes |
| Manual full test | (follow testing.md Layer 4) | Yes |
| Publish dry run | `dart pub publish --dry-run` | No |

---

## Known build issues

| Issue | Fix |
|---|---|
| `error C2338: winrt::hresult_error requires C++17 or later` | Ensure `target_compile_features(... cxx_std_20)` is in `windows/CMakeLists.txt`. Should already be there. |
| `LNK2019: unresolved external symbol` for WinRT types | Ensure `target_link_libraries(... windowsapp)` is in `windows/CMakeLists.txt`. |
| Visual Studio can't find `ShObjIdl_core.h` | Included in Windows SDK 10.0.19041.0+. Update SDK via Visual Studio Installer. |
| `flutter doctor` shows missing Windows toolchain | Run Visual Studio Installer → Modify → add "Desktop development with C++" |
| pana reports issues | Run `dart pub global run pana .` locally and read each finding. |

---

## Pre-publish build verification

```bash
# From repo root:
dart analyze .                     # 0 issues
flutter test                       # all passing
dart pub publish --dry-run         # no errors

# From example/:
flutter build windows --release    # release build succeeds
```

All four must pass before publishing. See [doc/PUBLISHING.md](../PUBLISHING.md)
for the full publish workflow.
