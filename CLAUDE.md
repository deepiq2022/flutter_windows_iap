# flutter_windows_iap — Project Guide for Claude

## What this project is

`flutter_windows_iap` is a Flutter plugin that wraps `Windows.Services.Store.StoreContext`
via a C++/WinRT platform channel, providing Microsoft Store in-app purchases for
Windows Flutter apps.

**Audience:** Flutter developers distributing apps through the Microsoft Store who need
non-consumable (durable) IAP on Windows. Consumables and subscriptions are deferred
(see [doc/TODO.md](doc/TODO.md)).

Published on pub.dev. Windows-only; all other platforms return `UnimplementedError`.

## Repo layout

```
lib/
├── flutter_windows_iap.dart              # Public API facade (singleton FlutterWindowsIap)
├── flutter_windows_iap_platform_interface.dart  # Abstract platform interface
├── flutter_windows_iap_method_channel.dart      # Default method-channel impl
└── src/
    └── models.dart                       # WinIapProduct, WinIapPurchaseResult, WinIapPurchaseStatus

windows/
├── flutter_windows_iap_plugin.h          # Plugin class declaration
├── flutter_windows_iap_plugin.cpp        # WinRT async implementation
├── flutter_windows_iap_plugin_c_api.cpp  # Flutter C API bridge (generated)
├── CMakeLists.txt                        # C++20, links windowsapp.lib
└── test/
    └── flutter_windows_iap_plugin_test.cpp  # C++ unit tests (arg validation/routing)

example/
├── lib/main.dart                         # Live IAP demo UI
└── integration_test/plugin_integration_test.dart  # Requires real MSIX + Store

test/
├── flutter_windows_iap_test.dart         # Platform-interface delegation tests
└── flutter_windows_iap_method_channel_test.dart  # Channel round-trip tests

doc/                                     # See section below — read BEFORE diving into code
```

## Read these docs first

`doc/` is the canonical reference. Read before reverse-engineering from code.

| What you need | Read |
|---|---|
| Product scope, requirements, what's in/out | [doc/REQUIREMENTS.md](doc/REQUIREMENTS.md) |
| Per-requirement implementation status | [doc/REQUIREMENTS_ASSESSMENT.md](doc/REQUIREMENTS_ASSESSMENT.md) |
| Code-quality findings and open gaps | [doc/CODE_QUALITY.md](doc/CODE_QUALITY.md) |
| Open work items | [doc/TODO.md](doc/TODO.md) |
| Architecture, WinRT patterns, design decisions | [doc/PROJECT_NOTES.md](doc/PROJECT_NOTES.md) |
| How to test (Dart, C++, integration, manual) | [doc/testing.md](doc/testing.md) |
| pub.dev publishing workflow | [doc/PUBLISHING.md](doc/PUBLISHING.md) |
| One file per public API method | [doc/methods/](doc/methods/) |
| Building, running, toolchain setup | [doc/claude/BUILDING.md](doc/claude/BUILDING.md) |
| Fresh-session bootstrap (read on first load) | [doc/claude/README.md](doc/claude/README.md) |

> **First time on this machine?** Read [doc/claude/README.md](doc/claude/README.md) first —
> it's the bootstrap guide for a fresh Claude session.

## Critical project rules

These rules prevent silent regressions and pub.dev quality issues.

### Thread safety

- **`result->Success()` / `result->Error()` are called from WinRT thread-pool threads**
  inside `winrt::fire_and_forget` coroutines. This is the accepted pattern for Flutter
  Windows plugins but means the `MethodResult` must not be touched after the coroutine
  captures it. See [doc/PROJECT_NOTES.md § Thread safety](doc/PROJECT_NOTES.md).
- **Never call `result` after moving it.** Each handler moves the `unique_ptr` into a
  `shared_ptr` immediately — do not use the original after that point.

### IInitializeWithWindow

- **`StoreContext::GetDefault()` alone is not sufficient for packaged Win32 apps.**
  Always call `ctx.as<IInitializeWithWindow>()->Initialize(hwnd_)` before any Store
  operation that shows UI. If `hwnd_` is null the Initialize call is skipped — this only
  happens in test contexts where no real window exists.

### Dart API surface

- **Three public methods only:** `queryProducts`, `purchase`, `restorePurchases`.
  Do not add new platform-interface methods without also adding method-channel impls,
  C++ handlers, Dart tests, and a doc/methods/ entry.
- **All models are in `lib/src/models.dart` and exported from the barrel.**
  Do not scatter model classes across files.

### pub.dev standards

- **`dart analyze` must be clean** (zero issues, including info) before every publish.
- **`flutter test` must be 100% green** before every publish.
- **CHANGELOG.md must be updated** for every version bump — pub.dev renders it.
- **README.md is the pub.dev landing page** — keep it accurate and the code samples
  compilable.

### Documentation maintenance

When you change code, update docs in the same commit:

- **Edit a C++ handler** → update `doc/methods/{method}.md` (Engine behavior + line refs);
  retire closed TODOs in `doc/TODO.md`.
- **Add a new public method** → create `doc/methods/{method}.md` from the section contract
  in `doc/methods/README.md`; update `doc/REQUIREMENTS.md` and
  `doc/REQUIREMENTS_ASSESSMENT.md`.
- **Land a code-quality fix** → flip 🔓 → ✅ in `doc/CODE_QUALITY.md` with date +
  reference.
- **Change product scope** → update `doc/REQUIREMENTS.md` and
  `doc/REQUIREMENTS_ASSESSMENT.md` together.
- **Publish a version** → follow the checklist in `doc/PUBLISHING.md`.

## Test harness

```bash
# From repo root:
dart analyze .                           # must be clean
flutter test                             # Dart unit tests (8 tests, no real Store needed)

# C++ tests (from example build output after flutter build windows in example/):
# Open the generated .sln in Visual Studio and run flutter_windows_iap_test target,
# or see doc/claude/BUILDING.md for command-line instructions.

# Integration tests (require MSIX packaging + Microsoft Store sandbox):
# See doc/testing.md — cannot run in standard CI.
```

## Recommended agents and skills

- **`claude-code-guide`** — questions about the Flutter plugin SDK, method channels,
  platform interface pattern, pub.dev publishing.
- **`general-purpose`** — WinRT API research, Windows SDK questions, CMake issues.
- **`Plan`** — designing the consumables or subscriptions feature before implementation.
- **`github-steward`** — PR management, issue triage, release tagging.

**Skills available in this project:**
- `/audit` — runs `dart analyze` + `flutter test` and summarizes results.
- `/claude-api` — if adding any Anthropic SDK integration (unlikely but possible for
  a companion tool).

## Memory

Recommended memory location for this project:
`~/.claude/projects/<encoded-path>/memory/`

Suggested memory files to maintain:
- `user_*.md` — who is working on this, their Flutter/C++ background
- `feedback_*.md` — approach corrections and validated decisions
- `project_*.md` — current version target, outstanding blockers, next milestone
- `reference_*.md` — links to Windows Store dashboard, pub.dev package page, CI

See [doc/claude/README.md](doc/claude/README.md) for the full memory bootstrap
guidance.
