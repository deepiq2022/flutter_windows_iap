# FlutterWindowsIap.queryProducts

## Identity

```dart
Future<List<WinIapProduct>> queryProducts(List<String> skuIds)
```

| Field | Value |
|---|---|
| Return type | `Future<List<WinIapProduct>>` |
| Category | Product metadata |
| Channel method name | `queryProducts` |

## Purpose

Fetches localised product metadata (title, description, price) for one or more
Microsoft Store add-on SKU IDs. Call this to populate a paywall or store screen
before the user initiates a purchase.

## Dart implementation

`lib/flutter_windows_iap_method_channel.dart:10`

```dart
Future<List<WinIapProduct>> queryProducts(List<String> skuIds) async {
  final result = await _channel.invokeListMethod<Map<Object?, Object?>>(
    'queryProducts',
    {'skuIds': skuIds},
  );
  return (result ?? []).map(WinIapProduct.fromMap).toList();
}
```

Channel args sent: `{'skuIds': List<String>}`

`WinIapProduct.fromMap` decodes each result map — `lib/src/models.dart:21`.

## C++ handler

**Synchronous entry:** `HandleQueryProducts` — `windows/flutter_windows_iap_plugin.cpp:87`

1. Extracts `skuIds` from the args map as `EncodableList`.
2. Builds `std::vector<winrt::hstring>` from the list.
3. Wraps `unique_ptr<MethodResult>` in `shared_ptr`.
4. Calls `QueryProductsAsync`.

**Async coroutine:** `QueryProductsAsync` — `windows/flutter_windows_iap_plugin.cpp:118`

1. Calls `GetStoreContext(hwnd_)` — initialises `StoreContext` with HWND.
2. Builds a `IVector<hstring>` for SKU IDs and a `IVector<hstring>` for product kinds (`"Durable"`).
3. `co_await ctx.GetStoreProductsAsync(product_kinds, ids)`
4. Iterates `query_result.Products()`, builds an `EncodableList` of `EncodableMap`.
5. Calls `result->Success(EncodableValue(products))`.

## WinRT API

| WinRT call | Purpose |
|---|---|
| `StoreContext::GetDefault()` | Acquires the per-process Store context |
| `IInitializeWithWindow::Initialize(hwnd)` | Parents any Store UI to the Flutter window |
| `StoreContext::GetStoreProductsAsync(kinds, ids)` | Fetches product details for the given SKU IDs |

Reference:
[`StoreContext.GetStoreProductsAsync`](https://learn.microsoft.com/en-us/uwp/api/windows.services.store.storecontext.getstoreproductsasync)

## Arguments

| Key | Type | Required | Notes |
|---|---|---|---|
| `skuIds` | `List<String>` | Yes | One or more Store product IDs. Unknown IDs are silently omitted by the Store. |

## Returns

`List<WinIapProduct>` — may be empty if no SKUs are recognised.

Each `WinIapProduct` map from C++:

| Key | C++ source | Dart field |
|---|---|---|
| `skuId` | `p.StoreId()` | `WinIapProduct.skuId` |
| `title` | `p.Title()` | `WinIapProduct.title` |
| `description` | `p.Description()` | `WinIapProduct.description` |
| `formattedPrice` | `p.Price().FormattedPrice()` | `WinIapProduct.formattedPrice` |
| `currencyCode` | `p.Price().CurrencyCode()` | `WinIapProduct.currencyCode` |

## Errors

| Condition | Code | Message | Details |
|---|---|---|---|
| Missing `skuIds` arg | `BAD_ARGS` | `"missing skuIds"` | — |
| `skuIds` is not a list | `BAD_ARGS` | `"skuIds must be a list"` | — |
| WinRT `hresult_error` | `STORE_ERROR` | `e.message()` as string | HRESULT as `int` |
| Other C++ exception | `STORE_ERROR` | `"queryProducts failed with an unknown error"` | — |

## Thread safety

`result->Success()` is called from a WinRT thread-pool thread (the `co_await`
resumption point). This is the accepted Flutter Windows plugin pattern but is
technically unsafe — see CODE_QUALITY A-1 for the planned fix (routing through
the Flutter platform task runner).

## Limitations

- Queries `"Durable"` product kind only. Consumable and subscription SKU IDs
  passed to this method will be silently omitted by the Store.
- Requires MSIX packaging. Returns `PlatformException` if the Store context is
  unavailable.

## TODOs

- (CODE_QUALITY B-1) No C++ unit test for `QueryProductsAsync` return values.
- (TODO T1-3) Verify behaviour with real SKUs after pana gating.
