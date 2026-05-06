# FlutterWindowsIap.restorePurchases

## Identity

```dart
Future<List<String>> restorePurchases()
```

| Field | Value |
|---|---|
| Return type | `Future<List<String>>` |
| Category | License restore |
| Channel method name | `restorePurchases` |

## Purpose

Queries the Microsoft Store for all non-consumable add-ons that the currently
signed-in Microsoft account has ever purchased. Call this at app startup and when
the user taps a "Restore purchases" button to reinstate entitlements after a
reinstall or device change.

## Dart implementation

`lib/flutter_windows_iap_method_channel.dart:34`

```dart
Future<List<String>> restorePurchases() async {
  final result = await _channel.invokeListMethod<String>('restorePurchases');
  return result ?? [];
}
```

Channel args sent: *(none — no argument map)*

Returns an empty list if the channel result is null.

## C++ handler

**Synchronous entry:** `HandleRestorePurchases` — `windows/flutter_windows_iap_plugin.cpp:116`

1. Receives no args (the Dart side sends no map).
2. Wraps `unique_ptr<MethodResult>` in `shared_ptr`.
3. Calls `RestorePurchasesAsync`.

**Async coroutine:** `RestorePurchasesAsync` — `windows/flutter_windows_iap_plugin.cpp:173`

1. Calls `GetStoreContext(hwnd_)`.
2. `co_await ctx.GetAppLicenseAsync()` — fetches the app's license, which
   includes all add-on licenses.
3. Iterates `license.AddOnLicenses()`:
   - Skips inactive licenses (`addon.IsActive()` == false).
   - Appends `addon.SkuStoreId()` to the result list.
4. Calls `result->Success(EncodableValue(owned))`.

## WinRT API

| WinRT call | Purpose |
|---|---|
| `StoreContext::GetDefault()` | Acquires the per-process Store context |
| `IInitializeWithWindow::Initialize(hwnd)` | Required for consistent context init; no UI shown by this method |
| `StoreContext::GetAppLicenseAsync()` | Fetches the app license including all add-on licenses |
| `StoreAppLicense::AddOnLicenses()` | Map of `StoreId → StoreLicense` for all add-ons |
| `StoreLicense::IsActive()` | True if the license is valid and not expired |
| `StoreLicense::SkuStoreId()` | The Store product ID string for the add-on |

Reference:
[`StoreContext.GetAppLicenseAsync`](https://learn.microsoft.com/en-us/uwp/api/windows.services.store.storecontext.getapplicenseasync)

## Arguments

*(none)*

## Returns

`List<String>` — SKU IDs of all active owned non-consumable add-ons. May be empty.

Each string is a Microsoft Store product ID (e.g. `"9NBLGGH4TNMP"`), matching
the `skuId` values passed to `queryProducts` and `purchase`.

## Errors

| Condition | Code | Message | Details |
|---|---|---|---|
| WinRT `hresult_error` | `STORE_ERROR` | `e.message()` as string | HRESULT as `int` |
| Other C++ exception | `STORE_ERROR` | `"restorePurchases failed with an unknown error"` | — |

## Thread safety

`result->Success()` is called from a WinRT thread-pool thread. See
CODE_QUALITY A-1.

## Limitations

- **Active licenses only.** A license that has lapsed (e.g. a subscription that
  expired, or a refunded purchase) will not appear in the result. For non-consumable
  durables, licenses do not expire, so this limitation is only relevant to future
  subscription support.
- **Requires MSIX packaging.** `StoreContext::GetDefault()` fails outside MSIX.
- **No offline cache.** Every call fetches live from the Store. If the Store is
  unreachable, a `PlatformException` is thrown. Callers that need offline
  robustness should cache the result locally.
- **Consumable fulfillment state is not included.** The WinRT `GetAppLicenseAsync`
  API reflects ownership, not fulfillment count. This method is only correct for
  non-consumable (Durable) add-ons.

## TODOs

- (CODE_QUALITY A-1) Route `result->Success/Error` through Flutter platform task runner.
- (CODE_QUALITY B-1) No C++ unit test for `RestorePurchasesAsync` return paths.
- (TODO T2-1) Consider a `lastRestored` timestamp so callers can avoid redundant
  calls within a session.
