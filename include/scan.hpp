#pragma once

#include "parse.hpp"
#include "types.hpp"

#include <optional>

namespace stdx {

namespace details {

template <typename T>
bool specifier_matches_type(const std::string_view &specifier) {
    if (specifier == "") {
        return true;  // Any type can be parsed with "{}" specifier
    };
    if (std::unsigned_integral<T>) {
        return specifier == "%u";
    } else if (std::integral<T>) {
        return specifier == "%d";
    } else if (std::floating_point<T>) {
        return specifier == "%f";
    } else if (std::same_as<std::decay_t<T>, std::string> || std::same_as<std::decay_t<T>, std::string_view>) {
        return specifier == "%s";
    }

    return false;
}

template <typename... Ts, std::size_t... Indexes>
std::optional<scan_error> specifiers_match_types(const details::StringViewVector &specifiers,
                                                 const std::index_sequence<Indexes...> &) {
    std::size_t invalid_index = 0;
    if (((specifier_matches_type<Ts>(specifiers[Indexes]) ? true : (invalid_index = Indexes, false)) && ...)) {
        return std::nullopt;
    } else {
        return scan_error{ScanErrorCode::FormatAndTypeMismatch,
                          "Format specifier " + std::to_string(invalid_index) + " do not match the provided type."};
    }
}

}  // namespace details

template <typename... Ts, size_t... Indexes>
details::scan_result<Ts...> scan_impl(const details::StringViewVector &inputs, const details::StringViewVector &formats,
                                      const std::index_sequence<Indexes...> &) {
    bool has_error = false;
    details::scan_error err;
    auto lambda = [&]<std::size_t Index, typename T>() {
        if(has_error){
            return T{};
        }
        auto value_parsing_result = details::parse_value_with_format<T>(inputs[Index], formats[Index]);
        if (value_parsing_result) {
            return std::move(*value_parsing_result);
        } else {
            err = std::move(value_parsing_result.error());
            has_error = true;
            return T{};
        }
    };

    details::scan_result<Ts...> result = details::scan_result<Ts...> {
        std::tuple<Ts...>{lambda.template operator()<Indexes, Ts>()...}
    };

    if (has_error){
        return std::unexpected(std::move(err));
    } else {
        return result;
    }
}

template <typename... Ts>
details::scan_result<Ts...> scan(std::string_view input, std::string_view format) {
    try {
        if (input.empty()) {
            return std::unexpected{details::scan_error{ScanErrorCode::NoInputProvided, "No input provided"}};
        }
        if (format.empty()) {
            return std::unexpected{details::scan_error{ScanErrorCode::NoFormatProvided, "No format provided"}};
        }

        const auto &parse_result = details::parse_sources(input, format);
        if (!parse_result) {
            return std::unexpected(parse_result.error());
        }

        if (sizeof...(Ts) != parse_result->second.size()) {
            return std::unexpected(
                details::scan_error{ScanErrorCode::InsufficientSpecifiers,
                                    "The number of format specifiers do not match the number of provided types."});
        }
        const auto specifiers_has_error =
            details::specifiers_match_types<Ts...>(parse_result->second, std::index_sequence_for<Ts...>{});
        if (specifiers_has_error) {
            return std::unexpected(*specifiers_has_error);
        }

        return scan_impl<Ts...>(parse_result->first, parse_result->second, std::index_sequence_for<Ts...>{});

    } catch (const std::exception &e) {
        return std::unexpected{details::scan_error{ScanErrorCode::UnknownError, e.what()}};
    }
}

}  // namespace stdx
