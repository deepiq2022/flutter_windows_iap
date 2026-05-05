import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'flutter_windows_iap_method_channel.dart';
import 'src/models.dart';

abstract class FlutterWindowsIapPlatform extends PlatformInterface {
  FlutterWindowsIapPlatform() : super(token: _token);

  static final Object _token = Object();

  static FlutterWindowsIapPlatform _instance = MethodChannelFlutterWindowsIap();

  static FlutterWindowsIapPlatform get instance => _instance;

  static set instance(FlutterWindowsIapPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<List<WinIapProduct>> queryProducts(List<String> skuIds) {
    throw UnimplementedError('queryProducts() has not been implemented.');
  }

  Future<WinIapPurchaseResult> purchase(String skuId) {
    throw UnimplementedError('purchase() has not been implemented.');
  }

  Future<List<String>> restorePurchases() {
    throw UnimplementedError('restorePurchases() has not been implemented.');
  }
}
