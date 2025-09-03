#pragma once

#include <wchar.h>
#include <uchar.h>
#include <string>
#include <string_view>
#include <ranges>
#include <expected>
#ifndef _WIN32
#include <iostream>
#endif

namespace wutils {

static constexpr bool wchar_is_char8 = sizeof(wchar_t) == sizeof(char8_t); // not used anywhere afaik but still here for completion
static constexpr bool wchar_is_char16 = sizeof(wchar_t) == sizeof(char16_t); // used on Windows
static constexpr bool wchar_is_char32 = sizeof(wchar_t) == sizeof(char32_t); // used on Linux, MacOS, and most UNIX systems

// A sanity check to ensure the size matches an expected type
static_assert(wchar_is_char8 || wchar_is_char16 || wchar_is_char32,
              "Unsupported wchar_t width, expecting 8, 16 or 32 bits");

// Only one type matches
static_assert(
    (wchar_is_char8 + wchar_is_char16 + wchar_is_char32) == 1,
    "Exactly one wchar_t type must match"
);

// Assume char is char8_t
static_assert(sizeof(char) == sizeof(char8_t), "Invalid char type assumption");

// Determine the underlying type for ustring at compile-time

using uchar_t = std::conditional_t<wchar_is_char8, char8_t,
                std::conditional_t<wchar_is_char16, char16_t, char32_t>>;
using ustring = std::basic_string<uchar_t>;
using ustring_view = std::basic_string_view<uchar_t>;


// Final sanity check
static_assert(sizeof(wchar_t) == sizeof(ustring::value_type) && sizeof(wchar_t) == sizeof(ustring_view::value_type), "Invalid wchar_t deduction");

// Windows sucks and can't properly print std::wcout to terminal so we use a wrapper
#ifdef _WIN32
void wprint(const std::wstring_view ws);
void wprintln(const std::wstring_view ws);
#else
inline void wprint(const std::wstring_view ws) {
    std::wcout << ws;
}
inline void wprintln(const std::wstring_view ws) {
    std::wcout << ws << std::endl;
}
#endif

// Determines course of action when encountered with an invalid sequence
enum class ErrorPolicy {
    SkipInvalidValues, // Skip invalid values and continue conversion to the best of its ability
    StopOnFirstError // Stop conversion on the first invalid value, return partial conversion
};

template<typename T>
struct ConversionFailure {
    // Either the "best effort" result skipping invalid characters, 
    // or the partially converted sequence up to the point of failure, depending on error policy
    T partial_result; 
};

template<typename T>
using ConversionResult = std::expected<T, ConversionFailure<T>>;

namespace detail {

template<typename FromChar, typename ToChar>
struct IsImplicitlyConvertible {
    static constexpr bool value = false;
};

template<>
struct IsImplicitlyConvertible<char, char8_t> {
    static constexpr bool value = true;
};

template<>
struct IsImplicitlyConvertible<char8_t, char> {
    static constexpr bool value = true;
};

template<>
struct IsImplicitlyConvertible<wchar_t, uchar_t> {
    static constexpr bool value = true;
};

template<>
struct IsImplicitlyConvertible<uchar_t, wchar_t> {
    static constexpr bool value = true;
};

inline ConversionResult<std::u8string> u8(const std::u8string_view u8s,
    [[maybe_unused]] const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return std::u8string(u8s);
}
ConversionResult<std::u8string> u8(const std::u16string_view u16s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues);
ConversionResult<std::u8string> u8(const std::u32string_view u32s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues);

ConversionResult<std::u16string> u16(const std::u8string_view u8s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues);
inline ConversionResult<std::u16string> u16(const std::u16string_view u16s,
    [[maybe_unused]] const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return std::u16string(u16s);
}
ConversionResult<std::u16string> u16(const std::u32string_view u32s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues);

ConversionResult<std::u32string> u32(const std::u8string_view u8s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues);
ConversionResult<std::u32string> u32(const std::u16string_view u16s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues);
inline ConversionResult<std::u32string> u32(const std::u32string_view u32s,
    [[maybe_unused]] const ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return std::u32string(u32s);
}

template<typename FromChar, typename ToChar>
requires IsImplicitlyConvertible<FromChar, ToChar>::value
std::basic_string<ToChar> convertImplicitly(std::basic_string_view<FromChar> from) {
#ifdef _MSC_VER // MSVC specific optimization
         return std::basic_string<ToChar>(std::from_range, from);
#else // Generic conversion
        return from | std::ranges::views::transform([](FromChar wc) {
            return static_cast<ToChar>(wc);
        }) | std::ranges::to<std::basic_string<ToChar>>();
#endif
};

template<typename FromChar>
ConversionResult<ustring> convert_to_ustring(std::basic_string_view<FromChar> from, ErrorPolicy errorPolicy) {
    if constexpr (std::is_same_v<uchar_t, char8_t>) {
        return u8(from, errorPolicy);
    } else if constexpr (std::is_same_v<uchar_t, char16_t>) {
        return u16(from, errorPolicy);
    } else if constexpr (std::is_same_v<uchar_t, char32_t>) {
        return u32(from, errorPolicy);
    }
}

template<typename ToChar>
ConversionResult<std::basic_string<ToChar>> convert_from_ustring(ustring_view from, ErrorPolicy errorPolicy) {
    if constexpr (std::is_same_v<ToChar, char8_t>) {
        return u8(from, errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char16_t>) {
        return u16(from, errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char32_t>) {
        return u32(from, errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, wchar_t>) {
        return convertImplicitly<uchar_t, wchar_t>(from);
    }
}

template<typename ToChar>
ConversionResult<std::basic_string<ToChar>> propagateError(ConversionResult<ustring> inner, ErrorPolicy errorPolicy) {
    if (inner.has_value()) {
        return convert_from_ustring<ToChar>(*inner, errorPolicy);
    } else {
        ConversionResult<std::basic_string<ToChar>> last = convert_from_ustring<ToChar>(inner.error().partial_result, errorPolicy);
        std::basic_string failedError = ( (last) ? (*last) : (last.error().partial_result));
        return std::unexpected(ConversionFailure<std::basic_string<ToChar>>(failedError));
    }
}


} // namespace detail
// Main "dispatcher"-like convert template
// It works like this because it needs to handle cases where we need an intermediate conversion to uchar_t
// E.g to convert wchar_t to char32_t on a Windows system (where uchar_t = char16_t), we need to:
// 1. Convert wchar_t to char16_t using convertImplicitly
// 2. Convert char16_t to char32_t using detail::u32(u16string_view)
// Same thing happens in the opposite direction (char32_t to wchar_t)
template<typename FromChar, typename ToChar>
ConversionResult<std::basic_string<ToChar>> convert(std::basic_string_view<FromChar> from, 
        ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    if constexpr (std::is_same_v<FromChar, ToChar>) {
        return std::basic_string<ToChar>(from);
    }
    else if constexpr (detail::IsImplicitlyConvertible<FromChar, ToChar>::value) {
        return detail::convertImplicitly<FromChar, ToChar>(from);
    } else if constexpr (std::is_same_v<ToChar, char8_t>) {
        return detail::propagateError<char8_t>(detail::convert_to_ustring<FromChar>(from, errorPolicy), errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char16_t>) {
        return detail::propagateError<char16_t>(detail::convert_to_ustring<FromChar>(from, errorPolicy), errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char32_t>) {
        return detail::propagateError<char32_t>(detail::convert_to_ustring<FromChar>(from, errorPolicy), errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, wchar_t>) {
        return detail::propagateError<wchar_t>(detail::convert_to_ustring<FromChar>(from, errorPolicy), errorPolicy);
    }
}

// Simple conversions to avoid ConversionResult
inline ustring ws_to_us(std::wstring_view from) {
    return detail::convertImplicitly<wchar_t, uchar_t>(from);
}

inline std::wstring us_to_ws(ustring_view from) {
    return detail::convertImplicitly<uchar_t, wchar_t>(from);
}

inline std::u8string s_to_u8s(std::string_view from) {
    return detail::convertImplicitly<char, char8_t>(from);
}

inline std::string u8s_to_s(std::u8string_view from) {
    return detail::convertImplicitly<char8_t, char>(from);
}

// "Advanced" conversions that require ConversionResult
template<typename FromChar>
inline ConversionResult<std::u8string> u8(std::basic_string_view<FromChar> from, ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return convert<FromChar, char8_t>(from, errorPolicy);
}

template<typename FromChar>
inline ConversionResult<std::u16string> u16(std::basic_string_view<FromChar> from, ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return convert<FromChar, char16_t>(from, errorPolicy);
}

template<typename FromChar>
inline ConversionResult<std::u32string> u32(std::basic_string_view<FromChar> from, ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return convert<FromChar, char32_t>(from, errorPolicy);
}

template<typename FromChar>
inline ConversionResult<ustring> us(std::basic_string_view<FromChar> from, ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return convert<FromChar, uchar_t>(from, errorPolicy);
}

template<typename FromChar>
inline ConversionResult<std::wstring> ws(std::basic_string_view<FromChar> from, ErrorPolicy errorPolicy = ErrorPolicy::SkipInvalidValues) {
    return convert<FromChar, wchar_t>(from, errorPolicy);
}

int uswidth(const std::u8string_view u8s);
int uswidth(const std::u16string_view u16s);
int uswidth(const std::u32string_view u32s);

inline int wswidth(const std::wstring_view ws) {
    ustring u = wutils::ws_to_us(ws);
    return wutils::uswidth(u);
}


} // namespace wutils

