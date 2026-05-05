# flutter_windows_iap — Publishing Guide

Step-by-step process for publishing a new version to pub.dev.

---

## Pre-publish checklist

Run through this list in order. Every item must be green before `pub publish`.

### 1. Code quality

```bash
dart analyze .          # must be zero issues
flutter test            # must be 8/8 (or more) passing
```

### 2. pana score

```bash
dart pub global activate pana
dart pub global run pana . --no-warning
```

Target: **≥ 110 / 130**. Common deductions to fix before publishing:
- Missing `topics` in `pubspec.yaml`
- Example app has analysis issues
- README missing sections pana expects (installation, usage, etc.)

### 3. Dry run

```bash
dart pub publish --dry-run
```

Fix any warnings before proceeding. Errors are blockers.

### 4. Verify version consistency

- `pubspec.yaml` `version:` matches the top entry in `CHANGELOG.md`
- `CHANGELOG.md` top entry has the correct date and all changes listed
- `README.md` features table and code samples reflect the current API

### 5. Manual test

Complete the Layer 4 manual test checklist in [testing.md](testing.md) on a
real MSIX-packaged build.

---

## Version bump process

### Patch release (bug fix, no API change)

1. Fix the bug.
2. Bump `version: 0.x.Y` → `0.x.Y+1` in `pubspec.yaml`.
3. Add a `## 0.x.Y+1` entry to `CHANGELOG.md`.
4. Update `docs/REQUIREMENTS_ASSESSMENT.md` if any 🟡/🔴 items were resolved.
5. Flip relevant items in `docs/CODE_QUALITY.md` and `docs/TODO.md`.

### Minor release (new feature, backward-compatible)

1. Implement the feature; update all relevant `docs/api/` files.
2. Bump `version: 0.X.y` → `0.X+1.0`.
3. Update `docs/REQUIREMENTS.md` if scope changed.
4. Update `docs/REQUIREMENTS_ASSESSMENT.md`.
5. Create `docs/api/{new_method}.md` for any new public methods.
6. Update `README.md` features table and usage examples.
7. Full pre-publish checklist above.

### Breaking change (major bump)

> The package is `0.x` so technically any minor bump can break. Once `1.0.0`
> is published, breaking changes require a major bump.

1. Make the breaking change.
2. Bump version to next appropriate semver.
3. Add a **migration guide** section to `CHANGELOG.md`.
4. Update `README.md` fully.

---

## Publishing

```bash
dart pub publish
```

You will be prompted to confirm. Pub.dev publishing is irreversible for a given
version — you can retract a version (which hides it from search) but not delete it.

After publishing:

- Verify the package page at `https://pub.dev/packages/flutter_windows_iap`
- Check that the README, CHANGELOG, and API docs rendered correctly
- Verify the pana score on the "Scores" tab

---

## CHANGELOG conventions

```markdown
## 0.2.0

* Add consumable add-on support (`WinIapConsumableResult`, `reportFulfillment`).
* Fix: `WinIapProduct.fromMap` now uses null-safe fallbacks (issue #12).

## 0.1.0

* Implement `queryProducts`, `purchase`, and `restorePurchases` via WinRT async.
* ...
```

Rules:
- One `## X.Y.Z` header per version, newest at top.
- Bullet points starting with a verb: **Add**, **Fix**, **Remove**, **Update**, **Deprecate**.
- Reference issue/PR numbers where applicable: `(issue #12)`, `(PR #15)`.
- No internal implementation details (e.g. "refactored HandleQueryProducts") — write
  from the consumer's perspective.

---

## pub.dev package metadata

Key fields in `pubspec.yaml` that affect discoverability and pana score:

```yaml
name: flutter_windows_iap
description: >-
  Flutter plugin for Microsoft Store in-app purchases on Windows.
  Wraps Windows.Services.Store.StoreContext via C++/WinRT.
version: 0.1.0
homepage: https://github.com/<org>/flutter_windows_iap
repository: https://github.com/<org>/flutter_windows_iap
issue_tracker: https://github.com/<org>/flutter_windows_iap/issues
topics:
  - iap
  - windows
  - store
  - in-app-purchase
```

> Fill in the `<org>` GitHub organization before publishing.

---

## Post-publish tasks

- [ ] Tag the commit: `git tag v0.1.0 && git push origin v0.1.0`
- [ ] Create a GitHub Release from the tag; paste the CHANGELOG entry as the
      release notes
- [ ] Close any GitHub issues resolved in this version
- [ ] Update `docs/TODO.md` — move published Tier 1 items to Closed
- [ ] Update `docs/REQUIREMENTS_ASSESSMENT.md` if publish gating items are now met
