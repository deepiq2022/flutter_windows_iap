import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_windows_iap/flutter_windows_iap.dart';
import 'package:flutter_windows_iap/flutter_windows_iap_platform_interface.dart';
import 'package:flutter_windows_iap/flutter_windows_iap_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class _MockPlatform
    with MockPlatformInterfaceMixin
    implements FlutterWindowsIapPlatform {
  @override
  Future<List<WinIapProduct>> queryProducts(List<String> skuIds) async {
    return [
      WinIapProduct(
        skuId: 'sku_1',
        title: 'Pro',
        description: 'Pro tier',
        formattedPrice: r'$3.00',
        currencyCode: 'USD',
      ),
    ];
  }

  @override
  Future<WinIapPurchaseResult> purchase(String skuId) async {
    return WinIapPurchaseResult(
      skuId: skuId,
      status: WinIapPurchaseStatus.succeeded,
    );
  }

  @override
  Future<List<String>> restorePurchases() async => ['sku_1'];
}

void main() {
  final FlutterWindowsIapPlatform initialPlatform =
      FlutterWindowsIapPlatform.instance;

  test('$MethodChannelFlutterWindowsIap is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelFlutterWindowsIap>());
  });

  group('FlutterWindowsIap delegates to platform', () {
    late FlutterWindowsIap iap;

    setUp(() {
      iap = FlutterWindowsIap.instance;
      FlutterWindowsIapPlatform.instance = _MockPlatform();
    });

    test('queryProducts returns list from platform', () async {
      final products = await iap.queryProducts(['sku_1']);
      expect(products.length, 1);
      expect(products.first.skuId, 'sku_1');
    });

    test('purchase returns result from platform', () async {
      final result = await iap.purchase('sku_1');
      expect(result.status, WinIapPurchaseStatus.succeeded);
    });

    test('restorePurchases returns list from platform', () async {
      final owned = await iap.restorePurchases();
      expect(owned, contains('sku_1'));
    });
  });
}
