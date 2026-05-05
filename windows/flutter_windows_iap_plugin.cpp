#include "flutter_windows_iap_plugin.h"

#include <windows.h>

// C++/WinRT runtime — requires /std:c++20 and linking windowsapp.lib
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Services.Store.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>
#include <vector>

namespace flutter_windows_iap {

namespace {

using namespace winrt::Windows::Services::Store;
using flutter::EncodableList;
using flutter::EncodableMap;
using flutter::EncodableValue;

std::string to_string(const winrt::hstring &h) {
  return winrt::to_string(h);
}

// Posts a result back from a background thread. The channel result object
// is not thread-safe; wrapping the call in a lambda posted to the Flutter
// task runner is the correct pattern. For now, results are posted directly
// since the async work is not yet implemented.
//
// TODO(windows-iap): route result callbacks through the UI thread runner
// once real WinRT async calls are in place.

}  // namespace

// static
void FlutterWindowsIapPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<EncodableValue>>(
          registrar->messenger(), "flutter_windows_iap",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterWindowsIapPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterWindowsIapPlugin::FlutterWindowsIapPlugin() {}

FlutterWindowsIapPlugin::~FlutterWindowsIapPlugin() {}

void FlutterWindowsIapPlugin::HandleMethodCall(
    const flutter::MethodCall<EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  const auto &name = method_call.method_name();
  const auto *args = std::get_if<EncodableMap>(method_call.arguments());

  if (name == "queryProducts") {
    if (!args) {
      result->Error("BAD_ARGS", "queryProducts requires a map argument");
      return;
    }
    HandleQueryProducts(*args, std::move(result));
  } else if (name == "purchase") {
    if (!args) {
      result->Error("BAD_ARGS", "purchase requires a map argument");
      return;
    }
    HandlePurchase(*args, std::move(result));
  } else if (name == "restorePurchases") {
    HandleRestorePurchases(std::move(result));
  } else {
    result->NotImplemented();
  }
}

void FlutterWindowsIapPlugin::HandleQueryProducts(
    const EncodableMap &args,
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  // Extract skuIds from args.
  auto it = args.find(EncodableValue("skuIds"));
  if (it == args.end()) {
    result->Error("BAD_ARGS", "missing skuIds");
    return;
  }
  const auto *sku_list = std::get_if<EncodableList>(&it->second);
  if (!sku_list) {
    result->Error("BAD_ARGS", "skuIds must be a list");
    return;
  }

  // TODO(windows-iap): call StoreContext::GetDefault() and
  // GetStoreProductsAsync with the extracted SKU IDs.  For now return an
  // empty list so the Dart layer doesn't hang.
  //
  // Intended implementation sketch:
  //   auto ctx = StoreContext::GetDefault();
  //   winrt::Windows::Foundation::Collections::IVector<winrt::hstring> ids =
  //       winrt::single_threaded_vector<winrt::hstring>();
  //   for (auto &sku : *sku_list) {
  //     auto *s = std::get_if<std::string>(&sku);
  //     if (s) ids.Append(winrt::to_hstring(*s));
  //   }
  //   auto query_result = co_await ctx.GetStoreProductsAsync(
  //       {L"Durable"}, ids);
  //   EncodableList products;
  //   for (auto &kv : query_result.Products()) {
  //     auto &p = kv.Value();
  //     EncodableMap m;
  //     m[EncodableValue("skuId")] = EncodableValue(to_string(p.StoreId()));
  //     m[EncodableValue("title")] = EncodableValue(to_string(p.Title()));
  //     m[EncodableValue("description")] =
  //         EncodableValue(to_string(p.Description()));
  //     auto price = p.Price();
  //     m[EncodableValue("formattedPrice")] =
  //         EncodableValue(to_string(price.FormattedPrice()));
  //     m[EncodableValue("currencyCode")] =
  //         EncodableValue(to_string(price.CurrencyCode()));
  //     products.push_back(EncodableValue(m));
  //   }
  //   result->Success(EncodableValue(products));

  result->Success(EncodableValue(EncodableList{}));
}

void FlutterWindowsIapPlugin::HandlePurchase(
    const EncodableMap &args,
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  auto it = args.find(EncodableValue("skuId"));
  if (it == args.end()) {
    result->Error("BAD_ARGS", "missing skuId");
    return;
  }
  const auto *sku_id = std::get_if<std::string>(&it->second);
  if (!sku_id) {
    result->Error("BAD_ARGS", "skuId must be a string");
    return;
  }

  // TODO(windows-iap): call StoreContext::GetDefault() and
  // RequestPurchaseAsync with the SKU ID.
  //
  // Intended implementation sketch:
  //   auto ctx = StoreContext::GetDefault();
  //   auto purchase_result = co_await ctx.RequestPurchaseAsync(
  //       winrt::to_hstring(*sku_id));
  //   EncodableMap m;
  //   m[EncodableValue("skuId")] = EncodableValue(*sku_id);
  //   switch (purchase_result.Status()) {
  //     case StorePurchaseStatus::Succeeded:
  //       m[EncodableValue("status")] = EncodableValue("succeeded");
  //       break;
  //     case StorePurchaseStatus::AlreadyPurchased:
  //       m[EncodableValue("status")] = EncodableValue("alreadyPurchased");
  //       break;
  //     case StorePurchaseStatus::NotPurchased:
  //       m[EncodableValue("status")] = EncodableValue("userCancelled");
  //       break;
  //     default:
  //       m[EncodableValue("status")] = EncodableValue("failed");
  //       m[EncodableValue("extendedError")] = EncodableValue(
  //           static_cast<int>(purchase_result.ExtendedError().value));
  //       break;
  //   }
  //   result->Success(EncodableValue(m));

  EncodableMap m;
  m[EncodableValue("skuId")] = EncodableValue(*sku_id);
  m[EncodableValue("status")] = EncodableValue(std::string("failed"));
  result->Success(EncodableValue(m));
}

void FlutterWindowsIapPlugin::HandleRestorePurchases(
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  // TODO(windows-iap): call StoreContext::GetDefault() and
  // GetAppLicenseAsync to enumerate owned non-consumable SKUs.
  //
  // Intended implementation sketch:
  //   auto ctx = StoreContext::GetDefault();
  //   auto license = co_await ctx.GetAppLicenseAsync();
  //   EncodableList owned;
  //   for (auto &kv : license.AddOnLicenses()) {
  //     if (kv.Value().IsActive()) {
  //       owned.push_back(
  //           EncodableValue(to_string(kv.Value().SkuStoreId())));
  //     }
  //   }
  //   result->Success(EncodableValue(owned));

  result->Success(EncodableValue(EncodableList{}));
}

}  // namespace flutter_windows_iap
