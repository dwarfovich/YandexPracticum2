#pragma once

#include "scan_error_code.hpp"
#include "types.hpp"

#include <charconv>
#include <concepts>
#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace stdx::details {

using StringViewVector = std::vector<std::string_view>;

template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view, std::string_view) {
    return std::unexpected{scan_error{"Unknown type was given"}};
}

template <typename T>
concept numeric = std::integral<std::decay_t<T>> || std::floating_point<std::decay_t<T>>;

template <numeric T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view) {
    std::decay_t<T> value;
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), value);
    if (ec == std::errc()) {
        return value;
    } else if (ec == std::errc::invalid_argument) {
        return std::unexpected{scan_error{ScanErrorCode::InvalidArgument, "Invalid argument"}};
    } else if (ec == std::errc::result_out_of_range) {
        return std::unexpected{
            scan_error{ScanErrorCode::NumberIsOutOfRange, "The number is too large for provided type"}};
    }

    return std::unexpected{
        scan_error{ScanErrorCode::UnknownError ,"Unknown error happened while parsing an argument"}};
}

template <typename T>
    requires std::same_as<std::decay_t<T>, std::string> || std::same_as<std::decay_t<T>, std::string_view>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view) {
    return T{input};
}

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
template <typename... Ts>
std::expected<std::pair<StringViewVector, StringViewVector>, scan_error> parse_sources(std::string_view input,
                                                                                       std::string_view format) {
    std::vector<std::string_view> input_parts;
    std::vector<std::string_view> format_parts;  // Части формата между {}
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{ScanErrorCode::UnformattedTextMismatch,
                                                  "Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней '}'
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{ScanErrorCode::UnformattedTextMismatch,
                                              "Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{input_parts, format_parts};
}

}  // namespace stdx::details