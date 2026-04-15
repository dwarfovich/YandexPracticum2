#pragma once

namespace stdx {

enum class ScanErrorCode {
    NoError,
    UnknownError,
    InvalidArgument,
    NumberIsOutOfRange,
    UnformattedTextMismatch,
    FormatAndTypeMismatch,
    NoInputProvided,
    NoFormatProvided,
    InsufficientSpecifiers
};

}