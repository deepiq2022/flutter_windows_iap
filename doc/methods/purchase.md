# FlutterWindowsIap.purchase

## Identity

```dart
Future<WinIapPurchaseResult> purchase(String skuId)
```

| Field | Value |
|---|---|
| Return type | `Future<WinIapPurchaseResult>` |
| Category | Purchase flow |
| Channel method name | `purchase` |

## Purpose

Opens the native Microsoft Store purchase dialog for a single SKU. The dialog
is a system-managed UI — the plugin does not draw it. Returns after the user
completes or dismisses the dialog. Check `WinIapPurchaseResult.status` to
determine the outcome and whether to grant the entitlement.

## Dart implementation

`lib/flutter_windows_iap_method_channel.dart:19`

```dart
Future<WinIapPurchaseResult> purchase(String skuId) async {
  final result = await _channel.invokeMapMethod<Object?, Object?>(
    'purchase',
    {'skuId': skuId},
  );
  if (result == null) {
    return WinIapPurchaseResult(skuId: skuId, status: WinIapPurchaseStatus.failed);
  }
  return WinIapPurchaseResult.fromMap(result);
}
```

Channel args sent: `{'skuId': String}`

A null channel result (should not occur from C++ but defensive) maps to `failed`.

`WinIapPurchaseResult.fromMap` decodes the result — `lib/src/models.dart:61`.

## C++ handler

**Synchronous entry:** `HandlePurchase` — `windows/flutter_windows_iap_plugin.cpp:102`

1. Extracts `skuId` string from the args map.
2. Wraps `unique_ptr<MethodResult>` in `shared_ptr`.
3. Calls `PurchaseAsync` with `winrt::to_hstring(*sku_id)`.

**Async coroutine:** `PurchaseAsync` — `windows/flutter_windows_iap_plugin.cpp:148`

1. Calls `GetStoreContext(hwnd_)` — initialises `StoreContext` with HWND.
2. `co_await ctx.RequestPurchaseAsync(sku_id)` — opens the Store dialog.
3. Switches on `purchase_result.Status()`:
   - `Succeeded` → `"succeeded"`
   - `AlreadyPurchased` → `"alreadyPurchased"`
   - `NotPurchased` (user cancelled) → `"userCancelled"`
   - Default → `"failed"` + `ExtendedError().value` as HRESULT
4. Calls `result->Success(EncodableValue(m))`.

## WinRT API

| WinRT call | Purpose |
|---|---|
| `StoreContext::GetDefault()` | Acquires the per-process Store context |
| `IInitializeWithWindow::Initialize(hwnd)` | Parents the purchase dialog to the Flutter window |
| `StoreContext::RequestPurchaseAsync(storeId)` | Opens the native Store purchase dialog |

Reference:
[`StoreContext.RequestPurchaseAsync`](https://learn.microsoft.com/en-us/uwp/api/windows.services.store.storecontext.requestpurchaseasync)

`IInitializeWithWindow::Initialize` **must** be called before
`RequestPurchaseAsync` on packaged Win32 desktop apps. Without it, the dialog
may not appear or may appear behind the main window.

## Arguments

| Key | Type | Required | Notes |
|---|---|---|---|
| `skuId` | `String` | Yes | Microsoft Store product ID for the add-on to purchase |

## Returns

`WinIapPurchaseResult` with:

| Field | Type | Source |
|---|---|---|
| `skuId` | `String` | Echoed from the request |
| `status` | `WinIapPurchaseStatus` | Decoded from `"succeeded"` / `"alreadyPurchased"` / `"userCancelled"` / `"failed"` |
| `extendedError` | `int?` | `StorePurchaseResult.ExtendedError().value` — only present when `status == failed` |

### Status mapping

| `StorePurchaseStatus` | `WinIapPurchaseStatus` | Recommended action |
|---|---|---|
| `Succeeded` | `succeeded` | Grant the entitlement |
| `AlreadyPurchased` | `alreadyPurchased` | Grant the entitlement silently |
| `NotPurchased` | `userCancelled` | No action required |
| Any other | `failed` | Show error; check `extendedError` |

## Errors

| Condition | Code | Message | Details |
|---|---|---|---|
| Missing `skuId` arg | `BAD_ARGS` | `"missing skuId"` | — |
| `skuId` not a string | `BAD_ARGS` | `"skuId must be a string"` | — |
| WinRT `hresult_error` | `STORE_ERROR` | `e.message()` as string | HRESULT as `int` |
| Other C++ exception | `STORE_ERROR` | `"purchase failed with an unknown error"` | — |

Common HRESULT values seen from `RequestPurchaseAsync`:

| HRESULT | Meaning |
|---|---|
| `0x803F6107` | App not associated with a Store account |
| `0x80004004` | Operation cancelled (may also surface as `userCancelled`) |
| `0x803F6108` | Store unavailable |

## Thread safety

`result->Success()` is called from a WinRT thread-pool thread. See
CODE_QUALITY A-1.

## Limitations

- **Durable (non-consumable) add-ons only.** Calling `purchase` on a consumable
  SKU may return `failed` or behave unexpectedly — consumable support is deferred
  (TODO T3-1).
- **Requires MSIX packaging.** `StoreContext::GetDefault()` fails outside MSIX.
- **Requires a window handle.** If for some reason `hwnd_` is null (not expected
  in production), the `IInitializeWithWindow` call is skipped and the dialog may
  not appear.
- **Not automatable.** The purchase dialog requires user interaction. Integration
  tests cannot exercise this path end-to-end.

## TODOs

- (CODE_QUALITY A-1) Route `result->Success/Error` through Flutter platform task runner.
- (CODE_QUALITY B-1) No C++ unit test for `PurchaseAsync` return paths.
- (TODO T3-1) Consumable support will require a new method (`reportFulfillment`);
  `purchase` itself may need to return quantity information.
