// Integration tests run against the real plugin host (C++/WinRT layer).
// These tests require a Microsoft Store-packaged build and a real Microsoft
// account; they are not expected to pass in CI without proper signing and
// Store configuration.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:flutter_windows_iap/flutter_windows_iap.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  final iap = FlutterWindowsIap.instance;

  testWidgets('queryProducts returns a list (may be empty without real SKUs)',
      (tester) async {
    final products = await iap.queryProducts(['fake_sku_for_testing']);
    // An empty list is fine — fake SKUs are silently omitted by the Store.
    expect(products, isA<List<WinIapProduct>>());
  });

  testWidgets('restorePurchases returns a list', (tester) async {
    final owned = await iap.restorePurchases();
    expect(owned, isA<List<String>>());
  });
}
