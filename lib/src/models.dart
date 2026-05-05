/// Metadata for a single Microsoft Store product.
class WinIapProduct {
  const WinIapProduct({
    required this.skuId,
    required this.title,
    required this.description,
    required this.formattedPrice,
    required this.currencyCode,
  });

  final String skuId;
  final String title;
  final String description;

  /// Localised price string as returned by the Store (e.g. "$0.99").
  final String formattedPrice;

  /// ISO 4217 currency code (e.g. "USD").
  final String currencyCode;

  factory WinIapProduct.fromMap(Map<Object?, Object?> map) {
    return WinIapProduct(
      skuId: map['skuId'] as String,
      title: map['title'] as String,
      description: map['description'] as String,
      formattedPrice: map['formattedPrice'] as String,
      currencyCode: map['currencyCode'] as String,
    );
  }
}

/// Outcome of a [FlutterWindowsIap.purchase] call.
enum WinIapPurchaseStatus {
  /// The user completed the purchase and the SKU is now owned.
  succeeded,

  /// The user already owns this SKU (non-consumable re-purchase attempt).
  alreadyPurchased,

  /// The user cancelled the Store dialog.
  userCancelled,

  /// The Store returned an error. See [WinIapPurchaseResult.extendedError].
  failed,
}

class WinIapPurchaseResult {
  const WinIapPurchaseResult({
    required this.skuId,
    required this.status,
    this.extendedError,
  });

  final String skuId;
  final WinIapPurchaseStatus status;

  /// HRESULT error code from [StoreContext.RequestPurchaseAsync], if [status]
  /// is [WinIapPurchaseStatus.failed].
  final int? extendedError;

  factory WinIapPurchaseResult.fromMap(Map<Object?, Object?> map) {
    final statusStr = map['status'] as String;
    final status = switch (statusStr) {
      'succeeded' => WinIapPurchaseStatus.succeeded,
      'alreadyPurchased' => WinIapPurchaseStatus.alreadyPurchased,
      'userCancelled' => WinIapPurchaseStatus.userCancelled,
      _ => WinIapPurchaseStatus.failed,
    };
    return WinIapPurchaseResult(
      skuId: map['skuId'] as String,
      status: status,
      extendedError: map['extendedError'] as int?,
    );
  }
}
