import 'package:flutter/services.dart';

import 'flutter_windows_iap_platform_interface.dart';
import 'src/models.dart';

class MethodChannelFlutterWindowsIap extends FlutterWindowsIapPlatform {
  final _channel = const MethodChannel('flutter_windows_iap');

  @override
  Future<List<WinIapProduct>> queryProducts(List<String> skuIds) async {
    final result = await _channel.invokeListMethod<Map<Object?, Object?>>(
      'queryProducts',
      {'skuIds': skuIds},
    );
    return (result ?? []).map(WinIapProduct.fromMap).toList();
  }

  @override
  Future<WinIapPurchaseResult> purchase(String skuId) async {
    final result = await _channel.invokeMapMethod<Object?, Object?>(
      'purchase',
      {'skuId': skuId},
    );
    if (result == null) {
      return WinIapPurchaseResult(
        skuId: skuId,
        status: WinIapPurchaseStatus.failed,
      );
    }
    return WinIapPurchaseResult.fromMap(result);
  }

  @override
  Future<List<String>> restorePurchases() async {
    final result = await _channel.invokeListMethod<String>('restorePurchases');
    return result ?? [];
  }
}
