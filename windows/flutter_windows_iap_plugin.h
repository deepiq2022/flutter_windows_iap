#ifndef FLUTTER_PLUGIN_FLUTTER_WINDOWS_IAP_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_WINDOWS_IAP_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace flutter_windows_iap {

class FlutterWindowsIapPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterWindowsIapPlugin();
  virtual ~FlutterWindowsIapPlugin();

  FlutterWindowsIapPlugin(const FlutterWindowsIapPlugin&) = delete;
  FlutterWindowsIapPlugin& operator=(const FlutterWindowsIapPlugin&) = delete;

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

 private:
  // Fetches product info for one or more SKU IDs.
  // Dart method: queryProducts({skuIds: List<String>})
  // Returns: List<Map> matching WinIapProduct.fromMap
  void HandleQueryProducts(
      const flutter::EncodableMap &args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Opens the native Store purchase dialog for a single SKU.
  // Dart method: purchase({skuId: String})
  // Returns: Map matching WinIapPurchaseResult.fromMap
  void HandlePurchase(
      const flutter::EncodableMap &args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Queries the Store for all owned non-consumable SKUs.
  // Dart method: restorePurchases()
  // Returns: List<String> of owned SKU IDs
  void HandleRestorePurchases(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace flutter_windows_iap

#endif  // FLUTTER_PLUGIN_FLUTTER_WINDOWS_IAP_PLUGIN_H_
