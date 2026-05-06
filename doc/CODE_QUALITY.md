# flutter_windows_iap — Code Quality

Cross-cutting audit findings, known gaps, and the staged plan to close them.

## Status legend

| Symbol | Meaning |
|---|---|
| 🔓 | Open — not yet addressed |
| 🚧 | In progress |
| ✅ | Closed — landed (date + reference) |
| 💤 | Deferred — intentionally skipped with rationale |

---

## Findings

### A — Architecture

**A-1 🔓 Thread safety of `MethodResult` callbacks**

`result->Success()` and `result->Error()` are called from WinRT thread-pool
threads (the resumption point of `co_await` in a `winrt::fire_and_forget`).
Flutter's `MethodResult` implementation for Windows is not documented as
thread-safe. Other plugin authors do the same (it works in practice), but the
correct pattern is to post the result call back to the Flutter platform thread
via the engine's task runner.

- **Risk:** intermittent race condition on the result object under thread
  pressure; not observed in testing but theoretically possible.
- **Fix:** Capture `flutter::BinaryMessenger*` + task runner from the registrar
  in the constructor; in each coroutine, use `PostTask` on the platform runner
  before calling `result->Success/Error`.
- **Effort:** ~2 hours.

**A-2 🔓 `StoreContext` reuse across calls**

A new `StoreContext::GetDefault()` is obtained for every method call. On
MSIX-packaged apps this is a lightweight COM singleton fetch, but it could be
cached as a member for clarity and minor efficiency.

- **Risk:** negligible — `GetDefault()` is idiomatic in samples and docs.
- **Fix:** Store `StoreContext ctx_` as a member; initialize lazily in
  `RegisterWithRegistrar` or on first use.
- **Effort:** 30 min.

---

### B — Tests

**B-1 🔓 No WinRT mock — async handlers untestable at C++ level**

`QueryProductsAsync`, `PurchaseAsync`, and `RestorePurchasesAsync` call live WinRT
APIs. There is no seam to inject a fake `StoreContext`. The C++ tests only cover
synchronous argument-validation paths.

- **Risk:** real behaviour of the async handlers is only validated by manual
  integration testing with a real MSIX + Microsoft account.
- **Fix (option 1):** Extract a `IStoreContextAdapter` C++ interface and inject a
  fake in tests — significant refactor.
- **Fix (option 2):** Add a test mode flag that makes the async handlers return
  canned results — simpler but a test-only code path in production binary.
- **Effort:** 4–8 hours for option 1; 1 hour for option 2.
- **Recommendation:** option 2 as a short-term measure; option 1 for v1.0.

**B-2 🔓 No integration test coverage in CI**

Integration tests require MSIX packaging and a real Microsoft Store sandbox
account. They cannot run in standard CI (GitHub Actions, etc.).

- **Risk:** regressions in the WinRT path not caught until manual testing.
- **Fix:** Document the manual test checklist (done — see `doc/testing.md`);
  investigate Store test certificates for CI long-term.
- **Effort:** CI integration is high-effort / uncertain feasibility.

**B-3 💤 No `example/test/widget_test.dart` content**

The generated widget test file is effectively empty. For a plugin example app,
widget tests add little value compared to integration tests.

- **Decision:** Leave as-is; the file exists to satisfy Flutter scaffolding.

---

### C — pub.dev / Publishing

**C-1 🔓 pana score not verified**

The package has not been published to pub.dev. The pana score (out of 130) is
unknown. Common deductions: missing topics, missing `funding`, incomplete
example, pedantic lint issues.

- **Fix:** Run `dart pub global run pana` locally before first publish;
  address any deductions.
- **Effort:** 1–2 hours.

**C-2 🔓 `pubspec.yaml` missing `topics` and `funding`**

pub.dev uses `topics` for discovery. Adding `topics: [iap, windows, store]` and
an optional `funding:` list improves discoverability and pana score.

- **Effort:** 5 min.

**C-3 🔓 Example app not runnable without real SKUs**

`example/lib/main.dart` hardcodes `['rm_ads', 'rs_2026_pack', 'pu_custom']` which
are Deep IQ SKUs, not real product IDs. The example compiles but will show empty
results for any other developer.

- **Fix:** Replace with a comment-only placeholder, or use a well-known Microsoft
  test SKU from the Store testing guide.
- **Effort:** 15 min.

---

### D — API Design

**D-1 💤 No consumable support**

Consumable add-ons require a `reportConsumableFulfillmentAsync` call after each
purchase, and a quantity-tracking model. Deferred to a later minor version.

- **Deferred until:** v0.2.0 or community request.

**D-2 💤 No subscription support**

Subscription add-ons require license-expiry checking and renewal logic. Deferred
indefinitely — complexity is high and demand is uncertain.

**D-3 🔓 `WinIapProduct.fromMap` silently casts**

`WinIapProduct.fromMap` uses `as String` casts without null safety; if the C++
side ever sends a null or wrong type, it throws an uncaught `TypeError` on the
Dart side rather than a clean `PlatformException`.

- **Fix:** Use `as String? ?? ''` with a fallback or throw a meaningful error.
- **Effort:** 15 min.

---

## Staged plan

| Phase | Items | Est. effort | Status |
|---|---|---|---|
| 0 — Foundation | C-2, C-3, D-3 (quick wins before first publish) | 1 hr | 🔓 |
| 1 — Publish | C-1 (pana), then first `pub publish` | 2 hr | 🔓 |
| 2 — Test coverage | B-1 option 2 (test-mode flag for async handlers) | 1 hr | 🔓 |
| 3 — Thread safety | A-1 (UI-thread result posting) | 2 hr | 🔓 |
| 4 — Polish | A-2 (ctx cache), B-1 option 1 (full mock) | 4–8 hr | 🔓 |
| 5 — Consumables | D-1 | TBD | 💤 |

---

## Update protocol

- When a finding is fixed: flip 🔓 → ✅, add date and commit/PR reference.
- When a new finding is identified: add to the appropriate section with a
  unique letter-number ID; add a row to the staged plan.
- When a finding is intentionally deferred: flip to 💤 with a rationale line.
