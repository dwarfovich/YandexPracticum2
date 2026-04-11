#pragma once

#include "scan_error_code.hpp"

#include <string>
#include <tuple>
#include <expected>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    ScanErrorCode error_code = ScanErrorCode::NoError;
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования

template <typename... Ts>
struct successful_scan_result {
    template<std::size_t N>
    const auto& get() const noexcept {
        return std::get<N>(values);
    }
    std::tuple<Ts...> values;
};

template <typename... Ts>
using scan_result = std::expected<successful_scan_result<Ts...>, scan_error>;

} // namespace stdx::details
