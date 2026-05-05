import 'package:flutter/material.dart';
import 'package:flutter_windows_iap/flutter_windows_iap.dart';

// Replace these with your real Microsoft Store product IDs.
const _testSkuIds = ['rm_ads', 'rs_2026_pack', 'pu_custom'];

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Windows IAP Example',
      theme: ThemeData(useMaterial3: true),
      home: const IapDemoPage(),
    );
  }
}

class IapDemoPage extends StatefulWidget {
  const IapDemoPage({super.key});

  @override
  State<IapDemoPage> createState() => _IapDemoPageState();
}

class _IapDemoPageState extends State<IapDemoPage> {
  final _iap = FlutterWindowsIap.instance;

  List<WinIapProduct> _products = [];
  List<String> _owned = [];
  String _log = '';
  bool _busy = false;

  void _appendLog(String msg) {
    setState(() => _log = '$msg\n$_log');
  }

  Future<void> _queryProducts() async {
    setState(() => _busy = true);
    try {
      final products = await _iap.queryProducts(_testSkuIds);
      setState(() => _products = products);
      _appendLog('queryProducts: ${products.length} result(s)');
    } catch (e) {
      _appendLog('queryProducts error: $e');
    } finally {
      setState(() => _busy = false);
    }
  }

  Future<void> _purchase(String skuId) async {
    setState(() => _busy = true);
    try {
      final result = await _iap.purchase(skuId);
      _appendLog('purchase($skuId): ${result.status.name}'
          '${result.extendedError != null ? ' [0x${result.extendedError!.toRadixString(16)}]' : ''}');
    } catch (e) {
      _appendLog('purchase error: $e');
    } finally {
      setState(() => _busy = false);
    }
  }

  Future<void> _restorePurchases() async {
    setState(() => _busy = true);
    try {
      final owned = await _iap.restorePurchases();
      setState(() => _owned = owned);
      _appendLog('restorePurchases: ${owned.isEmpty ? 'none' : owned.join(', ')}');
    } catch (e) {
      _appendLog('restorePurchases error: $e');
    } finally {
      setState(() => _busy = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Windows IAP Example')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            // Actions
            Wrap(
              spacing: 8,
              children: [
                FilledButton(
                  onPressed: _busy ? null : _queryProducts,
                  child: const Text('Query products'),
                ),
                OutlinedButton(
                  onPressed: _busy ? null : _restorePurchases,
                  child: const Text('Restore purchases'),
                ),
              ],
            ),
            const SizedBox(height: 16),

            // Product list
            if (_products.isNotEmpty) ...[
              Text('Products', style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 8),
              ...(_products.map((p) => Card(
                    child: ListTile(
                      title: Text(p.title.isEmpty ? p.skuId : p.title),
                      subtitle: Text(p.description.isEmpty
                          ? p.skuId
                          : '${p.description}\n${p.formattedPrice}'),
                      isThreeLine: p.description.isNotEmpty,
                      trailing: FilledButton.tonal(
                        onPressed: _busy ? null : () => _purchase(p.skuId),
                        child: const Text('Buy'),
                      ),
                    ),
                  ))),
              const SizedBox(height: 8),
            ],

            // Owned
            if (_owned.isNotEmpty) ...[
              Text('Owned SKUs',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 4),
              Wrap(
                spacing: 6,
                children: _owned.map((s) => Chip(label: Text(s))).toList(),
              ),
              const SizedBox(height: 8),
            ],

            // Log
            Text('Log', style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 4),
            Expanded(
              child: Container(
                padding: const EdgeInsets.all(8),
                decoration: BoxDecoration(
                  color: Colors.black87,
                  borderRadius: BorderRadius.circular(4),
                ),
                child: SingleChildScrollView(
                  child: Text(
                    _log.isEmpty ? '(no activity yet)' : _log,
                    style: const TextStyle(
                        fontFamily: 'monospace',
                        fontSize: 12,
                        color: Colors.greenAccent),
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
