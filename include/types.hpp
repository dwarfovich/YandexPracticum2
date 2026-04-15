#pragma once

#include "scan_error_code.hpp"

#include <expected>
#include <string>
#include <tuple>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования
struct scan_error {
    ScanErrorCode error_code = ScanErrorCode::NoError;
    std::string message;
};

template <typename... Ts>
concept PackWithoutReferences = (!std::is_reference_v<Ts> && ...);

// Шаблонный класс для хранения результатов успешного сканирования
template <PackWithoutReferences... Ts>
struct successful_scan_result {
    template <std::size_t N>
    const auto &get() const noexcept {
        return std::get<N>(values);
    }
    template <std::size_t N>
    auto &get() noexcept {
        return std::get<N>(values);
    }

    std::tuple<Ts...> values;
};

template <typename... Ts>
using scan_result = std::expected<successful_scan_result<Ts...>, scan_error>;

}  // namespace stdx::details
