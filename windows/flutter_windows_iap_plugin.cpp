#include "flutter_windows_iap_plugin.h"

#include <windows.h>
#include <ShObjIdl_core.h>  // IInitializeWithWindow

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

// Returns a StoreContext initialised for the given HWND so that purchase
// dialogs are properly parented. IInitializeWithWindow must be called on
// packaged Win32 desktop apps before RequestPurchaseAsync.
StoreContext GetStoreContext(HWND hwnd) {
  auto ctx = StoreContext::GetDefault();
  if (hwnd) {
    winrt::check_hresult(ctx.as<IInitializeWithWindow>()->Initialize(hwnd));
  }
  return ctx;
}

}  // namespace

// static
void FlutterWindowsIapPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<EncodableValue>>(
          registrar->messenger(), "flutter_windows_iap",
          &flutter::StandardMethodCodec::GetInstance());

  HWND hwnd = registrar->GetView()->GetNativeWindow();
  auto plugin = std::make_unique<FlutterWindowsIapPlugin>(hwnd);

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterWindowsIapPlugin::FlutterWindowsIapPlugin(HWND hwnd) : hwnd_(hwnd) {}

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

// ---------------------------------------------------------------------------
// Synchronous entry points — extract arguments, then hand off to coroutines.
// ---------------------------------------------------------------------------

void FlutterWindowsIapPlugin::HandleQueryProducts(
    const EncodableMap &args,
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
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

  std::vector<winrt::hstring> sku_ids;
  sku_ids.reserve(sku_list->size());
  for (const auto &sku : *sku_list) {
    const auto *s = std::get_if<std::string>(&sku);
    if (s) sku_ids.push_back(winrt::to_hstring(*s));
  }

  SharedResult shared(std::move(result));
  QueryProductsAsync(std::move(shared), std::move(sku_ids));
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

  SharedResult shared(std::move(result));
  PurchaseAsync(std::move(shared), winrt::to_hstring(*sku_id));
}

void FlutterWindowsIapPlugin::HandleRestorePurchases(
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  SharedResult shared(std::move(result));
  RestorePurchasesAsync(std::move(shared));
}

// ---------------------------------------------------------------------------
// WinRT coroutines
// ---------------------------------------------------------------------------

winrt::fire_and_forget FlutterWindowsIapPlugin::QueryProductsAsync(
    SharedResult result,
    std::vector<winrt::hstring> sku_ids) {
  try {
    auto ctx = GetStoreContext(hwnd_);

    auto product_kinds =
        winrt::single_threaded_vector<winrt::hstring>({L"Durable"});
    auto ids = winrt::single_threaded_vector(std::move(sku_ids));

    auto query_result =
        co_await ctx.GetStoreProductsAsync(product_kinds, ids);

    EncodableList products;
    for (auto &kv : query_result.Products()) {
      const auto &p = kv.Value();
      auto price = p.Price();
      EncodableMap m;
      m[EncodableValue("skuId")] = EncodableValue(to_string(p.StoreId()));
      m[EncodableValue("title")] = EncodableValue(to_string(p.Title()));
      m[EncodableValue("description")] =
          EncodableValue(to_string(p.Description()));
      m[EncodableValue("formattedPrice")] =
          EncodableValue(to_string(price.FormattedPrice()));
      m[EncodableValue("currencyCode")] =
          EncodableValue(to_string(price.CurrencyCode()));
      products.push_back(EncodableValue(m));
    }
    result->Success(EncodableValue(products));
  } catch (const winrt::hresult_error &e) {
    result->Error("STORE_ERROR", winrt::to_string(e.message()),
                  EncodableValue(static_cast<int>(e.code().value)));
  } catch (...) {
    result->Error("STORE_ERROR", "queryProducts failed with an unknown error");
  }
}

winrt::fire_and_forget FlutterWindowsIapPlugin::PurchaseAsync(
    SharedResult result,
    winrt::hstring sku_id) {
  try {
    auto ctx = GetStoreContext(hwnd_);
    auto purchase_result =
        co_await ctx.RequestPurchaseAsync(sku_id);

    EncodableMap m;
    m[EncodableValue("skuId")] = EncodableValue(to_string(sku_id));

    switch (purchase_result.Status()) {
      case StorePurchaseStatus::Succeeded:
        m[EncodableValue("status")] = EncodableValue(std::string("succeeded"));
        break;
      case StorePurchaseStatus::AlreadyPurchased:
        m[EncodableValue("status")] =
            EncodableValue(std::string("alreadyPurchased"));
        break;
      case StorePurchaseStatus::NotPurchased:
        // User cancelled the dialog.
        m[EncodableValue("status")] =
            EncodableValue(std::string("userCancelled"));
        break;
      default:
        m[EncodableValue("status")] = EncodableValue(std::string("failed"));
        m[EncodableValue("extendedError")] = EncodableValue(
            static_cast<int>(purchase_result.ExtendedError().value));
        break;
    }
    result->Success(EncodableValue(m));
  } catch (const winrt::hresult_error &e) {
    result->Error("STORE_ERROR", winrt::to_string(e.message()),
                  EncodableValue(static_cast<int>(e.code().value)));
  } catch (...) {
    result->Error("STORE_ERROR", "purchase failed with an unknown error");
  }
}

winrt::fire_and_forget FlutterWindowsIapPlugin::RestorePurchasesAsync(
    SharedResult result) {
  try {
    auto ctx = GetStoreContext(hwnd_);
    auto license = co_await ctx.GetAppLicenseAsync();

    EncodableList owned;
    for (auto &kv : license.AddOnLicenses()) {
      const auto &addon = kv.Value();
      if (addon.IsActive()) {
        owned.push_back(EncodableValue(to_string(addon.SkuStoreId())));
      }
    }
    result->Success(EncodableValue(owned));
  } catch (const winrt::hresult_error &e) {
    result->Error("STORE_ERROR", winrt::to_string(e.message()),
                  EncodableValue(static_cast<int>(e.code().value)));
  } catch (...) {
    result->Error("STORE_ERROR",
                  "restorePurchases failed with an unknown error");
  }
}

}  // namespace flutter_windows_iap
