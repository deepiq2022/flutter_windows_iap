# flutter_windows_iap — Project Notes

Cross-cutting architecture notes, design decisions, and WinRT patterns. Read
this before making structural changes to the C++ plugin or Dart API.

---

## Architecture overview

```
Flutter Dart
  FlutterWindowsIap.instance          ← public singleton facade
    └─ FlutterWindowsIapPlatform      ← platform interface (plugin_platform_interface)
         └─ MethodChannelFlutterWindowsIap   ← method-channel impl (default)
              │  MethodChannel('flutter_windows_iap')
              ▼
Windows C++
  FlutterWindowsIapPlugin::HandleMethodCall   ← routes by method name
    ├─ HandleQueryProducts  → QueryProductsAsync  (fire_and_forget)
    ├─ HandlePurchase       → PurchaseAsync        (fire_and_forget)
    └─ HandleRestorePurchases → RestorePurchasesAsync (fire_and_forget)
              │  winrt::Windows::Services::Store::StoreContext
              ▼
  Windows.Services.Store (WinRT / COM)
```

The Dart layer is pure method-channel plumbing. All Store logic lives in C++.

---

## WinRT async pattern

Flutter's `HandleMethodCall` is synchronous. WinRT Store calls are all `co_await`
asynchronous. The bridge pattern used:

1. **Synchronous entry point** validates arguments, wraps the `unique_ptr<MethodResult>`
   in a `shared_ptr`, then calls the coroutine:

   ```cpp
   void HandleQueryProducts(const EncodableMap &args,
       std::unique_ptr<MethodResult<EncodableValue>> result) {
     // ... arg extraction ...
     SharedResult shared(std::move(result));   // unique_ptr → shared_ptr
     QueryProductsAsync(std::move(shared), std::move(sku_ids));
   }
   ```

2. **`winrt::fire_and_forget` coroutine** `co_await`s the WinRT call, then calls
   `result->Success()` or `result->Error()`. The coroutine captures the
   `shared_ptr<MethodResult>` by value:

   ```cpp
   winrt::fire_and_forget QueryProductsAsync(SharedResult result, ...) {
     try {
       auto ctx = GetStoreContext(hwnd_);
       auto qr = co_await ctx.GetStoreProductsAsync(kinds, ids);
       // ... build EncodableList ...
       result->Success(EncodableValue(products));
     } catch (const winrt::hresult_error &e) {
       result->Error("STORE_ERROR", winrt::to_string(e.message()),
                     EncodableValue(static_cast<int>(e.code().value)));
     } catch (...) {
       result->Error("STORE_ERROR", "unknown error");
     }
   }
   ```

3. The `fire_and_forget` **must catch all exceptions**. If an exception propagates
   past the coroutine boundary in `fire_and_forget`, the process terminates.

**Known issue:** `result->Success/Error` is called from a WinRT thread-pool thread,
not the Flutter platform thread. This is technically unsafe (see CODE_QUALITY A-1)
but works in practice because Flutter Windows' binary messenger is thread-safe for
sending replies. The proper fix is to post back through the task runner.

---

## `IInitializeWithWindow` — window handle requirement

`Windows.Services.Store` dialogs (purchase UI, sign-in) need a parent window.
For packaged Win32 desktop apps (as opposed to UWP), the Store context must be
told which window to use via the `IInitializeWithWindow` COM interface:

```cpp
StoreContext GetStoreContext(HWND hwnd) {
  auto ctx = StoreContext::GetDefault();
  if (hwnd) {
    winrt::check_hresult(ctx.as<IInitializeWithWindow>()->Initialize(hwnd));
  }
  return ctx;
}
```

The `HWND` is captured at plugin registration time:

```cpp
HWND hwnd = registrar->GetView()->GetNativeWindow();
auto plugin = std::make_unique<FlutterWindowsIapPlugin>(hwnd);
```

**Without this call**, `RequestPurchaseAsync` may fail silently or the purchase
dialog may not appear at all on some Windows builds.

---

## Method channel argument encoding

| Dart call | Channel args | C++ extraction |
|---|---|---|
| `queryProducts(['a', 'b'])` | `{'skuIds': ['a', 'b']}` | `EncodableList` from `EncodableMap["skuIds"]` |
| `purchase('rm_ads')` | `{'skuId': 'rm_ads'}` | `std::string` from `EncodableMap["skuId"]` |
| `restorePurchases()` | *(no args)* | `args` pointer is null; no extraction needed |

`HandleRestorePurchases` deliberately does not check for args because the Dart
side sends no map. The `HandleMethodCall` dispatcher passes `nullptr` safely.

---

## Error mapping

| WinRT condition | C++ path | Dart result |
|---|---|---|
| `winrt::hresult_error` | `catch (const winrt::hresult_error &e)` | `PlatformException(code: 'STORE_ERROR', message: e.message(), details: e.code().value)` |
| Other C++ exception | `catch (...)` | `PlatformException(code: 'STORE_ERROR', message: '...failed with an unknown error')` |
| Missing `skuIds` arg | synchronous guard in `HandleQueryProducts` | `PlatformException(code: 'BAD_ARGS', message: 'missing skuIds')` |
| Missing `skuId` arg | synchronous guard in `HandlePurchase` | `PlatformException(code: 'BAD_ARGS', message: 'missing skuId')` |
| Unknown method name | `result->NotImplemented()` in dispatcher | `MissingPluginException` on Dart side |
| `purchase` on non-Durable SKU | Store returns `StorePurchaseStatus` error | `WinIapPurchaseStatus.failed` + HRESULT in `extendedError` |

---

## Store context lifecycle

`StoreContext::GetDefault()` is a COM singleton per process. There is no
explicit "close" or "dispose" required. Each of the three handlers currently
creates a new context reference on each call — this is idiomatic in Microsoft
samples and does not cause resource leaks.

---

## Design decisions log

| Decision | Rationale | Date |
|---|---|---|
| Singleton `FlutterWindowsIap.instance` rather than instantiable class | Plugin state (HWND) is process-global; singleton avoids confusion about multiple instances | 2026-05-05 |
| `shared_ptr<MethodResult>` bridge for coroutines | `unique_ptr` cannot be captured by value in a lambda/coroutine; `shared_ptr` is the idiomatic bridge | 2026-05-05 |
| Query only `"Durable"` product kind | Consumable support adds significant complexity (fulfillment reporting, quantity); deferred to v0.2 | 2026-05-05 |
| `IInitializeWithWindow` in `GetStoreContext()` helper | Centralises the init call so it is never forgotten when adding new Store methods | 2026-05-05 |
| Separate `HandleX` sync + `XAsync` coroutine split | Keeps argument validation in the synchronous path where it is easy to test; coroutines only run on valid input | 2026-05-05 |

---

## pubspec.yaml dependency notes

- `plugin_platform_interface: ^2.0.0` — required for the `PlatformInterface`
  token-verification pattern.
- No other Dart runtime dependencies. All Windows Store logic is in C++.
- Dev dependencies: `flutter_test`, `flutter_lints`.

---

## Versioning policy

`0.x.y` — unstable API. Minor bumps (`0.x`) may break the public interface.
Patch bumps (`0.x.y`) are bug fixes only.

`1.0.0` — stable API. From that point, semver applies strictly: breaking changes
require a major bump.

Consumable support (T3-1) will be a minor bump (`0.2.0`), not a breaking change,
since it only adds new methods.
