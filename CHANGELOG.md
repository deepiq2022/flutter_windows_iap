## 0.1.0

* Implement `queryProducts`, `purchase`, and `restorePurchases` via WinRT
  `Windows.Services.Store.StoreContext` C++/WinRT async coroutines.
* Use `IInitializeWithWindow` to parent the purchase dialog to the Flutter
  host window (required for packaged Win32 desktop apps).
* Update example app with a live IAP demo UI (query, buy, restore).
* Replace stale C++ unit tests with routing and argument-validation coverage.

## 0.0.1

* Initial scaffold: Dart API surface, method-channel routing, and C++/WinRT
  stub handlers with implementation sketches.
