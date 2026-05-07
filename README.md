# flutter_windows_iap

[![pub package](https://img.shields.io/pub/v/flutter_windows_iap.svg)](https://pub.dev/packages/flutter_windows_iap)
[![pub points](https://img.shields.io/pub/points/flutter_windows_iap)](https://pub.dev/packages/flutter_windows_iap/score)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Flutter plugin for Microsoft Store in-app purchases on Windows. Wraps
[`Windows.Services.Store.StoreContext`](https://learn.microsoft.com/en-us/uwp/api/windows.services.store.storecontext)
via a C++/WinRT platform channel — no third-party dependencies, no server-side
validation required for non-consumables.

| Platform | Supported |
|---|---|
| Windows | ✅ |
| Android / iOS / macOS / Linux / Web | ❌ |

---

## Features

| Method | Description |
|---|---|
| `queryProducts(skuIds)` | Fetch product metadata (title, description, price) for one or more Store SKU IDs |
| `purchase(skuId)` | Open the native Microsoft Store purchase dialog for a single SKU |
| `restorePurchases()` | Return all owned non-consumable SKU IDs for the current Microsoft account |

---

## Requirements

- **Windows 10 version 1607** (build 14393) or later — minimum for `Windows.Services.Store`
- App must be **packaged as MSIX** and distributed through the Microsoft Store

---

## Installation

```yaml
dependencies:
  flutter_windows_iap: ^0.1.0
```

This plugin only activates on Windows; it is safe to include in a multi-platform
`pubspec.yaml` as long as your code guards IAP calls with a platform check.

---

## Usage

### 1. Guard platform calls

```dart
import 'dart:io' show Platform;
import 'package:flutter_windows_iap/flutter_windows_iap.dart';

final iap = Platform.isWindows ? FlutterWindowsIap.instance : null;
```

### 2. Query product details

Call this on the paywall screen to display localised titles and prices. Unknown
SKU IDs are silently omitted by the Store.

```dart
final products = await iap!.queryProducts(['rm_ads', 'pu_custom']);
for (final p in products) {
  print('${p.title}  ${p.formattedPrice}  (${p.currencyCode})');
}
```

### 3. Purchase

```dart
final result = await iap!.purchase('rm_ads');

switch (result.status) {
  case WinIapPurchaseStatus.succeeded:
    // SKU is now owned — grant the entitlement.
    break;
  case WinIapPurchaseStatus.alreadyPurchased:
    // User already owns this — grant entitlement silently.
    break;
  case WinIapPurchaseStatus.userCancelled:
    // User dismissed the Store dialog — no action needed.
    break;
  case WinIapPurchaseStatus.failed:
    final code = result.extendedError?.toRadixString(16) ?? '?';
    print('Purchase failed: HRESULT 0x$code');
    break;
}
```

### 4. Restore on launch

Always call this at app startup to reinstate entitlements for users who
reinstall or switch devices.

```dart
final owned = await iap!.restorePurchases();
// owned is a List<String> of SKU IDs the current account has purchased.
if (owned.contains('rm_ads')) {
  removeAds();
}
```

---

## Models

### `WinIapProduct`

| Field | Type | Description |
|---|---|---|
| `skuId` | `String` | Microsoft Store product ID |
| `title` | `String` | Localised product title |
| `description` | `String` | Localised product description |
| `formattedPrice` | `String` | Localised price string, e.g. `"$0.99"` |
| `currencyCode` | `String` | ISO 4217 code, e.g. `"USD"` |

### `WinIapPurchaseResult`

| Field | Type | Description |
|---|---|---|
| `skuId` | `String` | The SKU that was purchased |
| `status` | `WinIapPurchaseStatus` | `succeeded`, `alreadyPurchased`, `userCancelled`, or `failed` |
| `extendedError` | `int?` | HRESULT from `RequestPurchaseAsync` when status is `failed` |

### `WinIapPurchaseStatus`

| Value | Meaning |
|---|---|
| `succeeded` | Purchase completed; SKU is now owned |
| `alreadyPurchased` | Non-consumable was already owned by this account |
| `userCancelled` | User closed the Store dialog without purchasing |
| `failed` | Store returned an error; check `extendedError` for the HRESULT |

---

## Error handling

All three methods throw a `PlatformException` if the Store layer itself fails
(e.g. the Store service is unavailable, or `IInitializeWithWindow` fails).

```dart
try {
  final result = await iap!.purchase('rm_ads');
  // ...
} on PlatformException catch (e) {
  // e.code    == 'STORE_ERROR'
  // e.message == WinRT error message
  // e.details == HRESULT as int (when available)
}
```

---

## Microsoft Store setup

1. In [Partner Center](https://partner.microsoft.com), open your app submission.
2. Under **In-app products**, create a **Durable** add-on for each SKU.
3. Note the **Store ID** (e.g. `9NBLGGH4TNMP`) — this is the `skuId` you pass
   to the plugin.
4. During development, use the
   [Store testing guide](https://learn.microsoft.com/en-us/windows/apps/publish/store-test-scenarios)
   to test purchases without real charges.

---

## Limitations

- **Non-consumables (Durable add-ons) only.** Consumable and subscription add-ons are
  not yet supported; calling `purchase` on one will return `failed`.
- **Requires MSIX packaging.** `Windows.Services.Store` is unavailable in unpackaged
  Win32 apps.
- **No offline license verification.** License state is always fetched live from the
  Store; there is no local cache.

---

## License

MIT — see [LICENSE](LICENSE).
