# flutter_windows_iap

Flutter plugin for Microsoft Store in-app purchases on Windows. Wraps
[`Windows.Services.Store.StoreContext`](https://learn.microsoft.com/en-us/uwp/api/windows.services.store.storecontext)
via a C++/WinRT platform channel.

> **Status:** scaffold / stub. The Dart API and C++ method routing are complete;
> the WinRT async calls are marked `// TODO` and return empty results until
> the full implementation lands.

## Features

| Method | Description |
|---|---|
| `queryProducts(skuIds)` | Fetch product metadata (title, price, currency) for one or more Store SKU IDs |
| `purchase(skuId)` | Open the native Microsoft Store purchase dialog for a single SKU |
| `restorePurchases()` | Return all owned non-consumable SKU IDs for the current Microsoft account |

## Requirements

- Windows 10 version 1607 (build 14393) or later (minimum for `Windows.Services.Store`)
- App must be packaged as MSIX and distributed through the Microsoft Store

## Installation

```yaml
dependencies:
  flutter_windows_iap: ^0.1.0
```

## Usage

```dart
import 'package:flutter_windows_iap/flutter_windows_iap.dart';

final iap = FlutterWindowsIap.instance;

// Fetch product details
final products = await iap.queryProducts(['rm_ads', 'rs_2026_pack', 'pu_custom']);
for (final p in products) {
  print('${p.title}  ${p.formattedPrice}');
}

// Purchase
final result = await iap.purchase('rm_ads');
switch (result.status) {
  case WinIapPurchaseStatus.succeeded:
    // grant entitlement
  case WinIapPurchaseStatus.alreadyPurchased:
    // already owned — grant entitlement
  case WinIapPurchaseStatus.userCancelled:
    // user dismissed the dialog
  case WinIapPurchaseStatus.failed:
    print('Purchase failed: HRESULT 0x${result.extendedError?.toRadixString(16)}');
}

// Restore on app launch
final owned = await iap.restorePurchases();
```

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

## Platform support

| Platform | Supported |
|---|---|
| Windows | ✅ |
| Android | ❌ |
| iOS | ❌ |
| macOS | ❌ |
| Linux | ❌ |
| Web | ❌ |

## License

MIT — see [LICENSE](LICENSE).
