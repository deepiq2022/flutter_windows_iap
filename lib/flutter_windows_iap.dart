import 'flutter_windows_iap_platform_interface.dart';
import 'src/models.dart';

export 'src/models.dart';

/// Windows Microsoft Store in-app purchase plugin.
///
/// Wraps [Windows.Services.Store.StoreContext] via a C++/WinRT platform
/// channel. All purchase flows go through the Microsoft Store; no
/// server-side validation is required for non-consumable purchases.
class FlutterWindowsIap {
  FlutterWindowsIap._();

  static final FlutterWindowsIap instance = FlutterWindowsIap._();

  /// Fetches product details for the given [skuIds] from the Store.
  ///
  /// Returns a list of [WinIapProduct] for every SKU that the Store
  /// recognises. Unknown SKUs are silently omitted.
  Future<List<WinIapProduct>> queryProducts(List<String> skuIds) {
    return FlutterWindowsIapPlatform.instance.queryProducts(skuIds);
  }

  /// Initiates a purchase for [skuId] by opening the native Store dialog.
  ///
  /// Resolves to a [WinIapPurchaseResult] when the dialog closes. Check
  /// [WinIapPurchaseResult.status] — a [WinIapPurchaseStatus.succeeded]
  /// result means the SKU is now owned; always call [restorePurchases] to
  /// confirm before granting entitlements in production.
  Future<WinIapPurchaseResult> purchase(String skuId) {
    return FlutterWindowsIapPlatform.instance.purchase(skuId);
  }

  /// Queries the Store for all previously purchased non-consumable SKUs
  /// associated with the current Microsoft account.
  ///
  /// Call this on app launch and after the user taps "Restore purchases".
  Future<List<String>> restorePurchases() {
    return FlutterWindowsIapPlatform.instance.restorePurchases();
  }
}
