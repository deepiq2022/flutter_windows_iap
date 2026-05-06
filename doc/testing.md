# flutter_windows_iap — Testing Guide

There are four layers of testing. Each layer has different prerequisites.

---

## Layer 1 — Dart unit tests (no Store required)

**Location:** `test/`  
**Command:**

```bash
flutter test
```

**What is covered:**

| Test file | Coverage |
|---|---|
| `flutter_windows_iap_test.dart` | `FlutterWindowsIap` delegates correctly to the platform interface; `MethodChannelFlutterWindowsIap` is the default instance |
| `flutter_windows_iap_method_channel_test.dart` | All three channel methods (`queryProducts`, `purchase`, `restorePurchases`) with a mock `MethodChannel` handler; Dart model parsing from map |

**What is NOT covered:**

- C++ async handler execution
- Real WinRT API calls
- Store status codes beyond what the mock returns

**Current status:** 8/8 passing.

---

## Layer 2 — C++ unit tests (no Store required)

**Location:** `windows/test/flutter_windows_iap_plugin_test.cpp`

These tests build as part of the example project. They do **not** require a real
Store context or MSIX packaging. The plugin is instantiated with `hwnd_ = nullptr`
so `IInitializeWithWindow::Initialize` is skipped.

**What is covered:**

| Test | Coverage |
|---|---|
| `UnknownMethodReturnsNotImplemented` | Routing — unknown method name hits `NotImplemented()` |
| `QueryProductsMissingArgsReturnsError` | `HandleQueryProducts` with null args map |
| `QueryProductsMissingSkuIdsReturnsError` | `HandleQueryProducts` with map but no `"skuIds"` key |
| `PurchaseMissingArgsReturnsError` | `HandlePurchase` with null args map |
| `PurchaseMissingSkuIdReturnsError` | `HandlePurchase` with map but no `"skuId"` key |

**What is NOT covered:**

- `QueryProductsAsync`, `PurchaseAsync`, `RestorePurchasesAsync` return values —
  these reach WinRT which is unavailable in the test binary. See CODE_QUALITY B-1
  for the planned fix.

**How to run:**

```bash
cd example
flutter build windows          # generates the CMake project
# Then open build/windows/x64/flutter_windows_iap.sln in Visual Studio
# and run the flutter_windows_iap_test target, or:
cd build/windows/x64
ctest --test-dir . -C Debug --output-on-failure
```

See [doc/claude/BUILDING.md](claude/BUILDING.md) for the full build setup.

---

## Layer 3 — Integration tests (MSIX + Store sandbox required)

**Location:** `example/integration_test/plugin_integration_test.dart`

These tests run against the real C++ plugin inside a Flutter app.

**Prerequisites:**

1. Windows MSIX packaging (Developer Certificate or Store certificate)
2. A Microsoft account signed in to the Windows Store
3. If testing `purchase`: SKUs registered in
   [Partner Center](https://partner.microsoft.com) and associated with the app
4. The app must be running in a Store sandbox context — see the
   [Store testing guide](https://learn.microsoft.com/en-us/windows/apps/publish/store-test-scenarios)

**What is covered:**

| Test | What it checks |
|---|---|
| `queryProducts returns a list` | Plugin reaches WinRT, returns a `List<WinIapProduct>` (may be empty for fake SKUs) |
| `restorePurchases returns a list` | Plugin reaches WinRT `GetAppLicenseAsync`, returns `List<String>` |

**What is NOT covered in integration tests:**

- `purchase` — requires a real user interaction and a live SKU; not automatable.

**How to run:**

```bash
cd example
flutter run --device-id windows integration_test/plugin_integration_test.dart
```

> Note: `flutter test integration_test/` does not work for Windows plugin
> integration tests — use `flutter run` with the integration test file.

---

## Layer 4 — Manual testing checklist

Use this before every publish. Requires MSIX packaging and real Store SKUs.

### Setup

- [ ] App is signed and packaged as MSIX
- [ ] SKUs are configured in Partner Center as Durable add-ons
- [ ] A Microsoft account is signed in on the test machine
- [ ] App is listed in Store submission (or using a sandbox)

### `queryProducts`

- [ ] Calling with valid SKU IDs returns products with non-empty `title`,
      `formattedPrice`, and `currencyCode`
- [ ] Calling with an unknown SKU ID returns an empty list (not an error)
- [ ] Calling with an empty list returns an empty list

### `purchase`

- [ ] Calling opens the native Store purchase dialog
- [ ] Completing the purchase returns `WinIapPurchaseStatus.succeeded`
- [ ] Dismissing the dialog without buying returns `WinIapPurchaseStatus.userCancelled`
- [ ] Attempting to purchase an already-owned SKU returns `WinIapPurchaseStatus.alreadyPurchased`
- [ ] The purchase dialog is correctly parented to the Flutter window (not behind it)

### `restorePurchases`

- [ ] After a successful purchase, `restorePurchases` includes the SKU ID
- [ ] On a fresh account with no purchases, returns an empty list
- [ ] Owned SKUs appear across reinstalls on the same account

### Error handling

- [ ] Running the app unpackaged (if possible) surfaces a `PlatformException`
      rather than crashing
- [ ] Network offline: `queryProducts` surfaces a `PlatformException` with
      a non-null `message`

---

## What each layer can and cannot do

| Capability | Layer 1 (Dart) | Layer 2 (C++) | Layer 3 (Integration) | Layer 4 (Manual) |
|---|---|---|---|---|
| No Store/MSIX needed | ✅ | ✅ | ❌ | ❌ |
| Runs in CI | ✅ | ✅ (with setup) | ❌ | ❌ |
| Tests WinRT async paths | ❌ | ❌ | ✅ | ✅ |
| Tests purchase UI | ❌ | ❌ | ❌ | ✅ |
| Tests real SKU data | ❌ | ❌ | 🟡 (empty SKUs) | ✅ |
