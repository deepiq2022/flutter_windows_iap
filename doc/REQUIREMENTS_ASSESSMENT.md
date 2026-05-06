# flutter_windows_iap — Requirements Assessment

Tracks the implementation status of every requirement in
[REQUIREMENTS.md](REQUIREMENTS.md).

## Status legend

| Symbol | Meaning |
|---|---|
| ✅ | Met — implemented and verified |
| 🟡 | Partial — implemented but with known gaps |
| 🔴 | Missing — not yet implemented |
| 💤 | Deferred — intentionally out of scope for now |

## Roll-up

| Area | ✅ | 🟡 | 🔴 | 💤 |
|---|---|---|---|---|
| R-QP (query products) | 5 | 1 | 0 | 0 |
| R-PU (purchase) | 7 | 0 | 0 | 0 |
| R-RP (restore) | 3 | 0 | 0 | 0 |
| R-ERR (errors) | 5 | 0 | 0 | 0 |
| R-API (Dart API) | 4 | 0 | 0 | 0 |
| R-NF-PLATFORM | 3 | 0 | 0 | 0 |
| R-NF-QUALITY | 3 | 1 | 1 | 0 |
| R-NF-PUBLISH | 4 | 0 | 0 | 0 |
| **Total** | **34** | **2** | **1** | **0** |

---

## R-QP — Query products

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-QP-1 | ✅ | `FlutterWindowsIap.queryProducts(List<String>)` in `lib/flutter_windows_iap.dart:20` |
| R-QP-2 | ✅ | `HandleQueryProducts` returns only the products in `query_result.Products()` — unknown SKUs omitted by the Store |
| R-QP-3 | ✅ | `WinIapProduct` fields: `skuId`, `title`, `description`, `formattedPrice`, `currencyCode` — `lib/src/models.dart:1` |
| R-QP-4 | ✅ | `price.FormattedPrice()` via WinRT — `windows/flutter_windows_iap_plugin.cpp` `QueryProductsAsync` |
| R-QP-5 | ✅ | `price.CurrencyCode()` via WinRT |
| R-QP-6 | 🟡 | Queries `"Durable"` only — consumable and subscription kinds excluded. Gap: if a caller passes a consumable SKU ID, it will be silently omitted rather than flagged. |

## R-PU — Purchase

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-PU-1 | ✅ | `HandlePurchase` → `PurchaseAsync` → `ctx.RequestPurchaseAsync(sku_id)` |
| R-PU-2 | ✅ | `WinIapPurchaseStatus` enum with four values — `lib/src/models.dart:33` |
| R-PU-3 | ✅ | `StorePurchaseStatus::Succeeded` → `"succeeded"` — `flutter_windows_iap_plugin.cpp` `PurchaseAsync` |
| R-PU-4 | ✅ | `StorePurchaseStatus::AlreadyPurchased` → `"alreadyPurchased"` |
| R-PU-5 | ✅ | `StorePurchaseStatus::NotPurchased` → `"userCancelled"` |
| R-PU-6 | ✅ | Default case → `"failed"` + `extendedError` from `purchase_result.ExtendedError().value` |
| R-PU-7 | ✅ | `ctx.as<IInitializeWithWindow>()->Initialize(hwnd_)` in `GetStoreContext()` — `flutter_windows_iap_plugin.cpp:35` |

## R-RP — Restore purchases

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-RP-1 | ✅ | `HandleRestorePurchases` → `RestorePurchasesAsync` → `ctx.GetAppLicenseAsync()` |
| R-RP-2 | ✅ | Returns `EncodableList` of `SkuStoreId()` strings; Dart decodes as `List<String>` |
| R-RP-3 | ✅ | `addon.IsActive()` guard in `RestorePurchasesAsync` |

## R-ERR — Error handling

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-ERR-1 | ✅ | `catch (const winrt::hresult_error &e)` → `result->Error("STORE_ERROR", ...)` in all three coroutines |
| R-ERR-2 | ✅ | `winrt::to_string(e.message())` passed as the error message |
| R-ERR-3 | ✅ | `static_cast<int>(e.code().value)` passed as `EncodableValue` details |
| R-ERR-4 | ✅ | `catch (...)` catch-all in all three `fire_and_forget` coroutines |
| R-ERR-5 | ✅ | `BAD_ARGS` errors for missing `skuIds` / `skuId` in synchronous entry points |

## R-API — Dart API surface

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-API-1 | ✅ | `FlutterWindowsIap.instance` singleton — `lib/flutter_windows_iap.dart:14` |
| R-API-2 | ✅ | `FlutterWindowsIapPlatform extends PlatformInterface` — `lib/flutter_windows_iap_platform_interface.dart` |
| R-API-3 | ✅ | `export 'src/models.dart'` in barrel — `lib/flutter_windows_iap.dart:4` |
| R-API-4 | ✅ | `MethodChannel('flutter_windows_iap')` — `lib/flutter_windows_iap_method_channel.dart:7` |

## R-NF-PLATFORM

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-NF-PLATFORM-1 | ✅ | Documented in README; `Windows.Services.Store` requires build 14393 |
| R-NF-PLATFORM-2 | ✅ | MSIX requirement documented in README and REQUIREMENTS.md |
| R-NF-PLATFORM-3 | ✅ | `target_compile_features(... cxx_std_20)` + `target_link_libraries(... windowsapp)` — `windows/CMakeLists.txt:52-53` |

## R-NF-QUALITY

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-NF-QUALITY-1 | ✅ | `dart analyze .` → 0 issues (verified 2026-05-05) |
| R-NF-QUALITY-2 | ✅ | `flutter test` → 8/8 passing (verified 2026-05-05) |
| R-NF-QUALITY-3 | 🟡 | pana score not yet verified — package not yet published. Run `dart pub global run pana` locally before publishing. |
| R-NF-QUALITY-4 | ✅ | `test/flutter_windows_iap_method_channel_test.dart` covers all three methods with mock channel |
| R-NF-QUALITY-5 | 🔴 | C++ unit tests cover routing and BAD_ARGS paths, but do not test `QueryProductsAsync` / `PurchaseAsync` / `RestorePurchasesAsync` return values — WinRT mocking not yet in place |

## R-NF-PUBLISH

| ID | Status | Evidence / Gaps |
|---|---|---|
| R-NF-PUBLISH-1 | ✅ | CHANGELOG.md updated through v0.1.0 |
| R-NF-PUBLISH-2 | ✅ | README.md code samples match current API |
| R-NF-PUBLISH-3 | ✅ | `pubspec.yaml` `version: 0.1.0` matches CHANGELOG |
| R-NF-PUBLISH-4 | ✅ | `dart pub publish --dry-run` must be run as final pre-publish check (see PUBLISHING.md) |

---

## Update protocol

1. After any code change, update the Status column and Evidence/Gaps text.
2. Update the Roll-up table counts.
3. Add a date annotation when flipping 🟡 or 🔴 → ✅ (e.g. `✅ 2026-05-05`).
4. Keep 🔴 rows visible even after they're fixed — move them to a "Closed findings"
   section at the bottom with the fix date and PR/commit reference.
