#include "scan.hpp"

#include <gtest/gtest.h>

#include <iomanip>
#include <limits>
#include <string>

 TEST(ScanTest, EmptyInputAndEmptyFormat) {
     auto result = stdx::scan<std::string>("", "");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::NoInputProvided);
 }

 TEST(ScanTest, EmptyInput) {
     auto result = stdx::scan<std::string>("", "{%d}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::NoInputProvided);
 }

 TEST(ScanTest, EmptyFormat) {
     auto result = stdx::scan<std::string>("Hello, world!", "");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::NoFormatProvided);
 }

 TEST(ScanTest, InvalidFormatOpeningBrace) {
     auto result = stdx::scan<std::string>("Hello, world!", "{");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::UnformattedTextMismatch);
 }

 TEST(ScanTest, InvalidFormatClosingBrace) {
     auto result = stdx::scan<std::string>("Hello, world!", "}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::UnformattedTextMismatch);
 }
 TEST(ScanTest, InvalidFormatIncompleteSpecifier) {
     auto result = stdx::scan<std::string>("Hello, world!", "{%}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::FormatAndTypeMismatch);
 }

 TEST(ScanTest, InvalidFormatSpecifier) {
     auto result = stdx::scan<std::string>("Hello, world!", "{%x}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::FormatAndTypeMismatch);
 }

 TEST(ScanTest, TestUint8_0) {
     auto result = stdx::scan<std::uint8_t>("0", "{%u}");
     ASSERT_EQ(result.value().get<0>(), 0);
 }

 TEST(ScanTest, TestUint8_128) {
     auto result = stdx::scan<std::uint8_t>("128", "{%u}");
     ASSERT_EQ(result.value().get<0>(), 128);
 }

 TEST(ScanTest, TestUint8_255) {
     auto result = stdx::scan<std::uint8_t>("255", "{%u}");
     ASSERT_EQ(result.value().get<0>(), 255);
 }

 TEST(ScanTest, TestUint8_OutOfRange) {
     auto result = stdx::scan<std::uint8_t>("256", "{%u}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::NumberIsOutOfRange);
 }

 TEST(ScanTest, TestUint8_OutOfRange2) {
     auto result = stdx::scan<std::uint8_t>("4000", "{%u}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::NumberIsOutOfRange);
 }

 TEST(ScanTest, Test2Uint8) {
     auto result = stdx::scan<std::uint8_t, std::uint8_t>("255 0", "{%u} {%u}");
     ASSERT_EQ(result.value().get<0>(), 255);
     ASSERT_EQ(result.value().get<1>(), 0);
 }

 TEST(ScanTest, Test2Uint8_UnsufficientSpecifiers) {
     auto result = stdx::scan<std::uint8_t, std::uint8_t>("255 0", "{%u}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::InsufficientSpecifiers);
 }

 TEST(ScanTest, TestWrongSignedIntSpecifier) {
     auto result = stdx::scan<int>("0", "{%u}");
     ASSERT_EQ(result.error().error_code, stdx::ScanErrorCode::FormatAndTypeMismatch);
 }

 template <typename T>
 void TestMinLimits(const std::string &specifier) {
     std::ostringstream oss;
     oss << std::setprecision(std::numeric_limits<T>::max_digits10) << std::numeric_limits<T>::min();
     auto result = stdx::scan<T>(oss.str(), specifier);
     if constexpr (std::is_same_v<T, float>) {
         EXPECT_FLOAT_EQ(result.value().get<0>(), std::numeric_limits<T>::min());
     } else if (std::is_same_v<T, double>) {
         EXPECT_DOUBLE_EQ(result.value().get<0>(), std::numeric_limits<T>::min());
     } else {
         ASSERT_EQ(result.value().get<0>(), std::numeric_limits<T>::min());
     }
 }

 template <typename T>
 void TestMaxLimits(const std::string &specifier) {
     auto result = stdx::scan<T>(std::to_string(std::numeric_limits<T>::max()), specifier);
     if constexpr (std::is_same<T, float>()) {
         EXPECT_FLOAT_EQ(result.value().get<0>(), std::numeric_limits<T>::max());
     } else if (std::is_same<T, double>()) {
         EXPECT_DOUBLE_EQ(result.value().get<0>(), std::numeric_limits<T>::max());
     } else {
         ASSERT_EQ(result.value().get<0>(), std::numeric_limits<T>::max());
     }
 }

 template <typename T>
 void TestLowestLimits(const std::string &specifier) {
     auto result = stdx::scan<T>(std::to_string(std::numeric_limits<T>::lowest()), specifier);
     if constexpr (std::is_same<T, float>()) {
         EXPECT_FLOAT_EQ(result.value().get<0>(), std::numeric_limits<T>::lowest());
     } else if (std::is_same<T, double>()) {
         EXPECT_DOUBLE_EQ(result.value().get<0>(), std::numeric_limits<T>::lowest());
     } else {
         ASSERT_EQ(result.value().get<0>(), std::numeric_limits<T>::lowest());
     }
 }

 template <typename T>
 void TestLimits(const std::string &specifier) {
     if constexpr (std::floating_point<T>) {
         TestMinLimits<T>(specifier);
     }
     TestMaxLimits<T>(specifier);
     TestLowestLimits<T>(specifier);
 }

 TEST(ScanTest, TestInt16_Limits) { TestLimits<std::int16_t>("{%d}"); }

 TEST(ScanTest, TestInt_Limits) { TestLimits<int>("{%d}"); }

 TEST(ScanTest, TestInt64_Limits) { TestLimits<std::int64_t>("{%d}"); }

 TEST(ScanTest, TestUint64_Limits) { TestLimits<std::uint64_t>("{%u}"); }

 TEST(ScanTest, TestFloat_Limits) { TestLimits<float>("{%f}"); }

 TEST(ScanTest, TestDouble_Limits) { TestLimits<double>("{%f}"); }

 TEST(ScanTest, TestMixed) {
     auto result = stdx::scan<std::uint8_t, double>("255 2.5", "{%u} {%f}");
     ASSERT_EQ(result.value().get<0>(), 255);
     ASSERT_DOUBLE_EQ(result.value().get<1>(), 2.5);
 }

TEST(ScanTest, TestStringView) {
    auto result = stdx::scan<const std::string_view>("hello", "{%s}");
    ASSERT_EQ(result.value().get<0>(), "hello");
}

 TEST(ScanTest, TestConstType) {
     auto result = stdx::scan<const int>("100", "{%d}");
     ASSERT_EQ(result.value().get<0>(), 100);
 }

 TEST(ScanTest, TestConstType2) {
     auto result = stdx::scan<const std::uint8_t>("42", "{%u}");
     ASSERT_EQ(result.value().get<0>(), 42);
 }

TEST(ScanTest, TestMixedConstTypes) {
    auto result =
        stdx::scan<const double, const std::string_view, const std::uint8_t>("2.7 hello 42", "{%f} {%s} {%u}");
    ASSERT_DOUBLE_EQ(result.value().get<0>(), 2.7);
    ASSERT_EQ(result.value().get<1>(), "hello");
    ASSERT_EQ(result.value().get<2>(), 42);
    static_assert(!stdx::details::PackWithoutReferences<std::string&>);
}

TEST(ScanTest, TestReferencesNotCompiled) {
    static_assert(!stdx::details::PackWithoutReferences<std::string &>);
    static_assert(!stdx::details::PackWithoutReferences<const int&>);
    static_assert(!stdx::details::PackWithoutReferences<const char &, double&>);
}

TEST(ScanTest, TestEmptySpecifier) {
    auto result = stdx::scan<float>("hello 42.42 hello", "hello {} hello");
    ASSERT_FLOAT_EQ(result.value().get<0>(), 42.42);
}

TEST(ScanTest, TestEmptySpecifiers) {
    auto result = stdx::scan<const std::uint8_t, std::string>("42 hello", "{} {}");
    ASSERT_EQ(result.value().get<0>(), 42);
    ASSERT_EQ(result.value().get<1>(), "hello");
}