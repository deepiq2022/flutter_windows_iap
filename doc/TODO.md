# flutter_windows_iap — TODO

Open work items, ranked by priority within each tier.

## Status legend

| Symbol | Meaning |
|---|---|
| 🔓 | Open |
| 🚧 | In progress |
| ✅ | Closed (date + reference) |
| 💤 | Deferred |

---

## Tier 1 — Pre-publish blockers (~1 hr total)

These must be done before the first `pub publish`.

| ID | Item | Notes |
|---|---|---|
| T1-1 🔓 | Add `topics: [iap, windows, store, in-app-purchase]` to `pubspec.yaml` | Improves pana score and pub.dev discoverability |
| T1-2 🔓 | Replace hardcoded Deep IQ SKUs in `example/lib/main.dart` | Swap `['rm_ads', 'rs_2026_pack', 'pu_custom']` with a comment placeholder or official Microsoft test SKU IDs |
| T1-3 🔓 | Run `dart pub global run pana` and fix any deductions | Target ≥ 110/130; see CODE_QUALITY.md C-1 |
| T1-4 🔓 | Run `dart pub publish --dry-run` and resolve any warnings | Final gate before real publish |

---

## Tier 2 — Post-publish improvements (~4–6 hr total)

Do in the first few versions after the initial publish.

| ID | Item | Notes |
|---|---|---|
| T2-1 🔓 | Fix `WinIapProduct.fromMap` unsafe casts (D-3 in CODE_QUALITY) | Use null-safe fallbacks; `lib/src/models.dart:21` |
| T2-2 🔓 | Add test-mode stubs for async C++ handlers (B-1 option 2) | Allows C++ unit tests to cover the coroutine return paths without a real Store |
| T2-3 🔓 | Route `result->Success/Error` through Flutter's platform task runner (A-1) | Eliminates theoretical thread-safety risk; see CODE_QUALITY.md A-1 |
| T2-4 🔓 | Cache `StoreContext` as a plugin member (A-2) | Minor: avoids repeated `GetDefault()` calls |
| T2-5 🔓 | Add `funding:` to `pubspec.yaml` if desired | Optional pub.dev nicety |

---

## Tier 3 — Future features (deferred)

| ID | Item | Notes |
|---|---|---|
| T3-1 💤 | Consumable add-on support | Requires `ReportConsumableFulfillmentAsync`, quantity tracking, new Dart model `WinIapConsumableResult`; target v0.2.0 |
| T3-2 💤 | Subscription add-on support | License-expiry, renewal, grace period handling; high complexity; defer until community demand |
| T3-3 💤 | `IStoreContextAdapter` interface for full C++ unit test isolation (B-1 option 1) | High-effort architectural refactor; worthwhile for v1.0 |
| T3-4 💤 | CI integration for integration tests | Requires Store test certificates in a CI runner; feasibility uncertain |
| T3-5 💤 | Offer eligibility queries (`GetStoreProductForCurrentAppAsync`) | Useful but niche; defer |

---

## Known issues

| ID | Issue | Workaround |
|---|---|---|
| KI-1 | `StoreContext::GetDefault()` throws `E_ILLEGAL_METHOD_CALL` when called outside an MSIX package | Expected — document in README. No workaround; MSIX is required. |
| KI-2 | `IInitializeWithWindow::Initialize` is skipped when `hwnd_` is null (test context) | Only affects C++ unit tests where no real window exists; async handlers will not be exercised in that context anyway |

---

## Closed items

*(none yet — will be populated as items are resolved)*

---

## Maintenance protocol

- Pick the top-open Tier 1 item first; never leave Tier 1 open at publish time.
- When closing an item: flip 🔓 → ✅, add date + commit/PR ref, move to
  "Closed items" section.
- When discovering a new item: decide tier (is it blocking publish? Is it a
  feature?), add to the appropriate table, and cross-reference CODE_QUALITY.md
  if it maps to an existing finding.
