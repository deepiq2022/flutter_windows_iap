# flutter_windows_iap — Claude Session Bootstrap

This directory contains portable guidance for AI tools (Claude and equivalents)
working on this project. Everything in `docs/claude/` mirrors durable project
conventions so any session on any machine starts with full context — no memory
files required.

---

## Why this directory exists

Per-user memory files are machine-local. If you're working on this project from
a new machine, in a worktree, or as a different Claude instance, you won't have
access to previous session memories. This directory fills that gap: it is the
authoritative source of truth for how to work on this project, and it lives
in the repo itself.

---

## Files in this directory

| File | When to read |
|---|---|
| `README.md` (this file) | First thing on every new session |
| `BUILDING.md` | Before building, testing, or running the example app |

---

## First-session checklist

On your first session with this project on a new machine:

1. **Read `CLAUDE.md`** (repo root) — project overview, critical rules, recommended agents.
2. **Read this file** — you're doing it now.
3. **Read `docs/PROJECT_NOTES.md`** — architecture, WinRT patterns, design decisions.
4. **Read `docs/TODO.md`** — what's open so you don't duplicate work.
5. **Read `docs/claude/BUILDING.md`** — before touching any build commands.
6. **Set up memory** — see the memory section below.

You do NOT need to read every docs file upfront. Read the API docs in `docs/api/`
only when working on a specific method. Read `docs/REQUIREMENTS.md` only when
evaluating scope.

---

## Project at a glance (for fast re-orientation)

| Fact | Value |
|---|---|
| Package name | `flutter_windows_iap` |
| Pub.dev status | Not yet published (as of 2026-05-05) |
| Current version | 0.1.0 |
| Public methods | `queryProducts`, `purchase`, `restorePurchases` |
| C++ standard | C++20, links `windowsapp.lib` |
| Dart analyze | Clean (0 issues) |
| Dart tests | 8/8 passing |
| Blocking pre-publish items | T1-1 through T1-4 (see `docs/TODO.md`) |

---

## Critical rules (abbreviated)

Full rules are in `CLAUDE.md`. Key points to hold in working memory:

- **All three WinRT handlers are `fire_and_forget` coroutines.** They must catch
  all exceptions — if an exception escapes a `fire_and_forget`, the process terminates.
- **`IInitializeWithWindow::Initialize(hwnd_)` must be called** before any Store
  operation. It's in `GetStoreContext()` — don't bypass it.
- **`dart analyze .` must be clean** and **`flutter test` must pass** before every commit.
- **CHANGELOG.md must be updated** in the same commit as any version bump.
- **Docs must be updated** in the same commit as code changes (see CLAUDE.md §
  Documentation maintenance).

---

## Memory setup

Recommended memory structure for this project. Create these files at:
`~/.claude/projects/<encoded-project-path>/memory/`

The encoded path is the project directory with `/` → `-` and spaces → `-`, prefixed
with `C-` on Windows (e.g.
`C--Users-jttay-StudioProjects-flutter_windows_iap`).

### Suggested memory files

**`user_profile.md`**
```markdown
---
name: User profile
description: Developer working on flutter_windows_iap
type: user
---
[Who they are, their Flutter/C++/WinRT experience level, preferred workflow]
```

**`project_current.md`**
```markdown
---
name: Current project state
description: Active version, blockers, next milestone for flutter_windows_iap
type: project
---
Version target: 0.1.0 (pre-publish)
Next milestone: Complete T1-1 through T1-4 and publish to pub.dev
Blockers: [any current blockers]

Why: First public release; package is feature-complete for durable IAP.
How to apply: Prioritise Tier 1 items from docs/TODO.md.
```

**`feedback_approach.md`**
```markdown
---
name: Approach feedback
description: Corrections and validated decisions from past sessions
type: feedback
---
[Saved feedback from the user, e.g. preferred commit style, how to handle C++ changes]
```

**`reference_links.md`**
```markdown
---
name: Reference links
description: External resources for flutter_windows_iap maintenance
type: reference
---
- Partner Center: https://partner.microsoft.com
- pub.dev package page: https://pub.dev/packages/flutter_windows_iap (once published)
- Windows.Services.Store docs: https://learn.microsoft.com/en-us/uwp/api/windows.services.store
- Store test scenarios: https://learn.microsoft.com/en-us/windows/apps/publish/store-test-scenarios
- Flutter Windows plugin docs: https://docs.flutter.dev/platform-integration/windows/c-interop
```

---

## Sync protocol

When something in this directory becomes stale:

1. Update the affected file in `docs/claude/`.
2. If the change is also relevant to `CLAUDE.md` or another top-level doc, update
   both in the same commit.
3. Do NOT update per-machine memory files to reflect repo-level facts — the repo
   docs are authoritative. Per-machine memory is only for facts that vary by
   user/machine (who is working, their preferences, machine-specific paths).

---

## Recommended agents for this project

When working on specific tasks, consider spawning these agents rather than doing
everything inline:

| Task type | Agent |
|---|---|
| WinRT API questions, CMake issues | `general-purpose` (web search) |
| pub.dev publishing, CHANGELOG, PR management | `github-steward` |
| Planning a new feature (consumables, subscriptions) | `Plan` |
| Flutter plugin pattern questions | `claude-code-guide` |
| Code review before publish | `general-purpose` with explicit review prompt |

---

## Recommended skills

| Skill | When to use |
|---|---|
| `/audit` | Before any publish or PR — runs `dart analyze` + `flutter test` |
| `/review` | Before publishing a version — reviews the current branch changes |
