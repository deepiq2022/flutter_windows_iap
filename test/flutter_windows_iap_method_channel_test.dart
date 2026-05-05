import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_windows_iap/flutter_windows_iap_method_channel.dart';
import 'package:flutter_windows_iap/src/models.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  final platform = MethodChannelFlutterWindowsIap();
  const channel = MethodChannel('flutter_windows_iap');

  group('queryProducts', () {
    setUp(() {
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
          .setMockMethodCallHandler(channel, (call) async {
        if (call.method == 'queryProducts') {
          return [
            {
              'skuId': 'sku_1',
              'title': 'Pro',
              'description': 'Pro tier',
              'formattedPrice': r'$3.00',
              'currencyCode': 'USD',
            }
          ];
        }
        return null;
      });
    });

    tearDown(() {
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
          .setMockMethodCallHandler(channel, null);
    });

    test('returns parsed WinIapProduct list', () async {
      final result = await platform.queryProducts(['sku_1']);
      expect(result.length, 1);
      expect(result.first.skuId, 'sku_1');
      expect(result.first.currencyCode, 'USD');
    });
  });

  group('purchase', () {
    setUp(() {
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
          .setMockMethodCallHandler(channel, (call) async {
        if (call.method == 'purchase') {
          return {'skuId': call.arguments['skuId'], 'status': 'succeeded'};
        }
        return null;
      });
    });

    tearDown(() {
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
          .setMockMethodCallHandler(channel, null);
    });

    test('returns succeeded result', () async {
      final result = await platform.purchase('sku_1');
      expect(result.status, WinIapPurchaseStatus.succeeded);
      expect(result.skuId, 'sku_1');
    });

    test('returns failed result on null response', () async {
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
          .setMockMethodCallHandler(channel, (call) async => null);
      final result = await platform.purchase('sku_x');
      expect(result.status, WinIapPurchaseStatus.failed);
    });
  });

  group('restorePurchases', () {
    setUp(() {
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
          .setMockMethodCallHandler(channel, (call) async {
        if (call.method == 'restorePurchases') {
          return ['sku_1', 'sku_2'];
        }
        return null;
      });
    });

    tearDown(() {
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
          .setMockMethodCallHandler(channel, null);
    });

    test('returns list of owned SKU IDs', () async {
      final owned = await platform.restorePurchases();
      expect(owned, ['sku_1', 'sku_2']);
    });
  });
}
