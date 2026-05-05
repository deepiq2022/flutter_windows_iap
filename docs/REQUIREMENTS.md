# flutter_windows_iap — Requirements

## Vision

Provide Flutter developers a minimal, idiomatic plugin for Microsoft Store
in-app purchases on Windows: three methods, zero server setup, native Store UI,
with clear error surfaces and faithful mapping of Store status codes to Dart enums.

---

## Personas

| Persona | Description |
|---|---|
| **App developer** (primary) | A Flutter developer shipping a Windows app through the Microsoft Store who needs to gate features or remove ads behind a one-time purchase. |
| **Plugin maintainer** (secondary) | The developer (or AI tool) keeping `flutter_windows_iap` up-to-date with Flutter engine changes, WinRT API changes, and pub.dev standards. |

---

## Scope

### In scope

- Query product metadata (title, description, localised price) for durable add-ons
- Initiate a purchase via the native Windows Store dialog
- Restore all previously purchased non-consumable SKUs for the signed-in account
- Surface WinRT errors as `PlatformException` with HRESULT details
- Windows 10 1607 (build 14393) and later

### Out of scope

- Consumable add-ons (managed or unmanaged)
- Subscription add-ons
- In-app offer eligibility queries
- Receipt/license server validation
- Offline license caching
- Non-Windows platforms (Android, iOS, macOS, Linux, Web)
- Microsoft Account sign-in UI

---

## Functional requirements

### R-QP — Query products

| ID | Requirement |
|---|---|
| R-QP-1 | `queryProducts(List<String> skuIds)` accepts one or more Store product IDs |
| R-QP-2 | Returns a `List<WinIapProduct>` containing only recognised SKUs (unknown IDs are silently omitted by the Store) |
| R-QP-3 | Each `WinIapProduct` exposes `skuId`, `title`, `description`, `formattedPrice`, `currencyCode` |
| R-QP-4 | `formattedPrice` is the Store's localised price string (e.g. `"$0.99"`) |
| R-QP-5 | `currencyCode` is an ISO 4217 code |
| R-QP-6 | Queries only `"Durable"` product kind |

### R-PU — Purchase

| ID | Requirement |
|---|---|
| R-PU-1 | `purchase(String skuId)` opens the native Microsoft Store purchase dialog |
| R-PU-2 | Returns `WinIapPurchaseResult` with `status` one of: `succeeded`, `alreadyPurchased`, `userCancelled`, `failed` |
| R-PU-3 | `succeeded` — SKU is now owned; caller should grant the entitlement |
| R-PU-4 | `alreadyPurchased` — non-consumable already owned; caller should grant silently |
| R-PU-5 | `userCancelled` — user dismissed the dialog; no action required |
| R-PU-6 | `failed` — Store returned an error; `extendedError` contains the HRESULT |
| R-PU-7 | The purchase dialog is parented to the Flutter host window via `IInitializeWithWindow` |

### R-RP — Restore purchases

| ID | Requirement |
|---|---|
| R-RP-1 | `restorePurchases()` queries all owned non-consumable SKUs for the current Microsoft account |
| R-RP-2 | Returns `List<String>` of owned SKU IDs (may be empty) |
| R-RP-3 | Only active licenses are included |

### R-ERR — Error handling

| ID | Requirement |
|---|---|
| R-ERR-1 | WinRT `hresult_error` is surfaced as `PlatformException` with `code: 'STORE_ERROR'` |
| R-ERR-2 | The WinRT error message is included in `PlatformException.message` |
| R-ERR-3 | The HRESULT integer value is included in `PlatformException.details` when available |
| R-ERR-4 | A catch-all for non-WinRT exceptions prevents `fire_and_forget` from propagating to process termination |
| R-ERR-5 | Missing or malformed method channel arguments return `PlatformException` with `code: 'BAD_ARGS'` |

### R-API — Dart API surface

| ID | Requirement |
|---|---|
| R-API-1 | Public API is exposed through a singleton `FlutterWindowsIap.instance` |
| R-API-2 | The platform interface follows the Flutter `plugin_platform_interface` pattern |
| R-API-3 | All public model types are exported from the top-level barrel |
| R-API-4 | The method channel name is `'flutter_windows_iap'` |

---

## Non-functional requirements

### R-NF-PLATFORM

| ID | Requirement |
|---|---|
| R-NF-PLATFORM-1 | Minimum Windows version: 10 1607 (build 14393) |
| R-NF-PLATFORM-2 | Requires MSIX packaging (documented in README) |
| R-NF-PLATFORM-3 | C++ plugin uses C++20 (`/std:c++20`) and links `windowsapp.lib` |

### R-NF-QUALITY

| ID | Requirement |
|---|---|
| R-NF-QUALITY-1 | `dart analyze .` reports zero issues (including info) |
| R-NF-QUALITY-2 | `flutter test` passes 100% |
| R-NF-QUALITY-3 | pub.dev pana score ≥ 110/130 |
| R-NF-QUALITY-4 | All three public methods have Dart unit tests using a mock channel |
| R-NF-QUALITY-5 | C++ handlers have unit tests covering routing and argument validation |

### R-NF-PUBLISH

| ID | Requirement |
|---|---|
| R-NF-PUBLISH-1 | CHANGELOG.md updated for every version bump |
| R-NF-PUBLISH-2 | README.md code samples are compilable and accurate |
| R-NF-PUBLISH-3 | pubspec.yaml `version` matches the latest CHANGELOG entry |
| R-NF-PUBLISH-4 | `dart pub publish --dry-run` passes before any real publish |

---

## Glossary

| Term | Definition |
|---|---|
| **SKU ID** | The Microsoft Store product ID (also called Store ID) for an add-on, e.g. `9NBLGGH4TNMP` |
| **Durable** | Non-consumable add-on type in the Windows Store — owned indefinitely once purchased |
| **HRESULT** | A 32-bit Windows error code. Negative values indicate failure. |
| **MSIX** | Windows application package format required for Store distribution |
| **`StoreContext`** | The WinRT class that is the entry point for all `Windows.Services.Store` operations |
| **`IInitializeWithWindow`** | COM interface used to associate a WinRT dialog with a Win32 HWND |
| **`fire_and_forget`** | C++/WinRT coroutine return type that runs async work without a caller-observable handle |

---

## Update protocol

When requirements change:
1. Edit this file to add/modify/retire requirements.
2. Update `docs/REQUIREMENTS_ASSESSMENT.md` to reflect the new status.
3. If scope changes (in/out), update the README.md limitations section.
4. If a new method is added, create `docs/api/{method}.md`.
