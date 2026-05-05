# flutter_windows_iap — API Reference

One document per public method on `FlutterWindowsIap`.

## Index

| File | Method | Category |
|---|---|---|
| [query_products.md](query_products.md) | `queryProducts(List<String> skuIds)` | Product metadata |
| [purchase.md](purchase.md) | `purchase(String skuId)` | Purchase flow |
| [restore_purchases.md](restore_purchases.md) | `restorePurchases()` | License restore |

---

## Section-structure contract

Every method doc follows these sections in order. Keep all sections present even
if the answer is "N/A" — it makes the file machine-parseable and consistent.

1. `# FlutterWindowsIap.{methodName}` — heading matches the Dart method name exactly
2. `## Identity` — Dart signature, return type, category tag
3. `## Purpose` — 1–2 sentences plain English; what the caller uses this for
4. `## Dart implementation` — which file/line handles the channel send; what map keys are sent
5. `## C++ handler` — synchronous entry point (`Handle*`) + async coroutine (`*Async`); file:line refs
6. `## WinRT API` — which `Windows.Services.Store` class and method is called
7. `## Arguments` — table of channel arg keys, types, required/optional
8. `## Returns` — what the Dart method resolves to; how the C++ result is decoded
9. `## Errors` — all error codes, conditions, and Dart `PlatformException` shape
10. `## Thread safety` — which thread calls `result->Success/Error`; known risks
11. `## Limitations` — known gaps, deferred features
12. `## TODOs` — open items specific to this method

---

## Universal conventions

- **File refs** use `file:line` notation, e.g. `lib/src/models.dart:21`.
- **TODO cross-refs** point to `docs/TODO.md` by ID, e.g. `(TODO T2-1)`.
- **Code-quality cross-refs** point to `docs/CODE_QUALITY.md` by ID, e.g. `(CODE_QUALITY A-1)`.
- When the C++ line numbers change, update the line refs in the method doc in the
  same commit.

---

## Adding a new method

When a new public method is added to `FlutterWindowsIap`:

1. Create `docs/api/{snake_case_name}.md` using the section contract above.
2. Add a row to the index table in this README.
3. Add the method to `docs/REQUIREMENTS.md` under a new `R-XX` group.
4. Add assessment rows to `docs/REQUIREMENTS_ASSESSMENT.md`.
5. Update `README.md` features table and Usage section.
6. Update `CHANGELOG.md`.
