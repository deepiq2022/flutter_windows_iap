#include <flutter/method_call.h>
#include <flutter/method_result_functions.h>
#include <flutter/standard_method_codec.h>
#include <gtest/gtest.h>
#include <windows.h>

#include <memory>
#include <string>
#include <variant>

#include "flutter_windows_iap_plugin.h"

namespace flutter_windows_iap {
namespace test {

namespace {

using flutter::EncodableList;
using flutter::EncodableMap;
using flutter::EncodableValue;
using flutter::MethodCall;
using flutter::MethodResultFunctions;

// Helpers to capture success / error results synchronously.
// The WinRT async methods are not testable here without a real Store context,
// but the synchronous argument-validation paths are.

struct CapturedResult {
  bool success = false;
  bool not_implemented = false;
  std::string error_code;
  std::string error_message;
};

std::unique_ptr<MethodResultFunctions<>> MakeCapture(CapturedResult &out) {
  return std::make_unique<MethodResultFunctions<>>(
      // success
      [&out](const EncodableValue *) { out.success = true; },
      // error
      [&out](const std::string &code, const std::string &msg,
             const EncodableValue *) {
        out.error_code = code;
        out.error_message = msg;
      },
      // not_implemented
      [&out]() { out.not_implemented = true; });
}

}  // namespace

class FlutterWindowsIapPluginTest : public ::testing::Test {
 protected:
  // Use a null HWND — fine for argument-validation tests that never reach the
  // WinRT layer.
  FlutterWindowsIapPlugin plugin_{nullptr};
};

TEST_F(FlutterWindowsIapPluginTest, UnknownMethodReturnsNotImplemented) {
  CapturedResult out;
  plugin_.HandleMethodCall(
      MethodCall("unknownMethod", std::make_unique<EncodableValue>()),
      MakeCapture(out));
  EXPECT_TRUE(out.not_implemented);
}

TEST_F(FlutterWindowsIapPluginTest, QueryProductsMissingArgsReturnsError) {
  CapturedResult out;
  // Pass nullptr args (no map).
  plugin_.HandleMethodCall(
      MethodCall("queryProducts", std::make_unique<EncodableValue>()),
      MakeCapture(out));
  EXPECT_EQ(out.error_code, "BAD_ARGS");
}

TEST_F(FlutterWindowsIapPluginTest, QueryProductsMissingSkuIdsReturnsError) {
  CapturedResult out;
  // Pass a map but without the "skuIds" key.
  auto args = std::make_unique<EncodableValue>(EncodableMap{});
  plugin_.HandleMethodCall(MethodCall("queryProducts", std::move(args)),
                           MakeCapture(out));
  EXPECT_EQ(out.error_code, "BAD_ARGS");
}

TEST_F(FlutterWindowsIapPluginTest, PurchaseMissingArgsReturnsError) {
  CapturedResult out;
  plugin_.HandleMethodCall(
      MethodCall("purchase", std::make_unique<EncodableValue>()),
      MakeCapture(out));
  EXPECT_EQ(out.error_code, "BAD_ARGS");
}

TEST_F(FlutterWindowsIapPluginTest, PurchaseMissingSkuIdReturnsError) {
  CapturedResult out;
  auto args = std::make_unique<EncodableValue>(EncodableMap{});
  plugin_.HandleMethodCall(MethodCall("purchase", std::move(args)),
                           MakeCapture(out));
  EXPECT_EQ(out.error_code, "BAD_ARGS");
}

}  // namespace test
}  // namespace flutter_windows_iap
