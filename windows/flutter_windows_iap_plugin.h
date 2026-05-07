#ifndef FLUTTER_PLUGIN_FLUTTER_WINDOWS_IAP_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_WINDOWS_IAP_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Services.Store.h>

#include <memory>
#include <string>
#include <vector>

namespace flutter_windows_iap {

class FlutterWindowsIapPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  explicit FlutterWindowsIapPlugin(HWND hwnd);
  virtual ~FlutterWindowsIapPlugin();

  FlutterWindowsIapPlugin(const FlutterWindowsIapPlugin&) = delete;
  FlutterWindowsIapPlugin& operator=(const FlutterWindowsIapPlugin&) = delete;

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

 private:
  HWND hwnd_;

  // Shared-ptr wrapper so result can be captured in fire_and_forget coroutines.
  using SharedResult =
      std::shared_ptr<flutter::MethodResult<flutter::EncodableValue>>;

  void HandleQueryProducts(
      const flutter::EncodableMap &args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  void HandlePurchase(
      const flutter::EncodableMap &args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  void HandleRestorePurchases(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  winrt::fire_and_forget QueryProductsAsync(
      SharedResult result,
      std::vector<winrt::hstring> sku_ids);

  winrt::fire_and_forget PurchaseAsync(
      SharedResult result,
      winrt::hstring sku_id);

  winrt::fire_and_forget RestorePurchasesAsync(SharedResult result);
};

}  // namespace flutter_windows_iap

#endif  // FLUTTER_PLUGIN_FLUTTER_WINDOWS_IAP_PLUGIN_H_
