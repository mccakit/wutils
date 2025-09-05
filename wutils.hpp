#pragma once

#include <wchar.h>
#include <uchar.h>
#include <concepts>
#include <string>
#include <string_view>
#include <ranges>
#include <expected>
#include <type_traits>
#include <utility>
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

// We assume char is char8_t, and treat std::string as UTF8
// Note: on windows using MSVC, make sure you compile with the `/utf8` flag

// Determine the underlying type for ustring at compile-time

using uchar_t = std::conditional_t<wchar_is_char8, char8_t,
                std::conditional_t<wchar_is_char16, char16_t, char32_t>>;
using ustring = std::basic_string<uchar_t>;
using ustring_view = std::basic_string_view<uchar_t>;


// Final sanity check
static_assert(sizeof(wchar_t) == sizeof(ustring::value_type) && sizeof(wchar_t) == sizeof(ustring_view::value_type), "Invalid wchar_t deduction");

// Windows sucks and can't properly print std::wcout to terminal so we use a wrapper
#ifdef _WIN32
void wcout(const std::wstring_view ws);
void wcerr(const std::wstring_view ws);
#else
inline void wcout(const std::wstring_view ws) {
    std::wcout << ws;
}
inline void wcerr(const std::wstring_view ws) {
    std::wcerr << ws << std::endl;
}
#endif

inline void wprint(const std::wstring_view ws) {
    wcout(ws);
}
inline void wprintln(const std::wstring_view ws) {
    wcout(ws); wcout(L"\n");
}

template<typename T>
concept string_impl = std::is_same_v<T, std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>;

template<typename T>
concept string_view_impl = string_impl<T> || (std::is_same_v<T, std::basic_string_view<typename T::value_type, typename T::traits_type>>);


template<typename T>
concept unicode_string_view = (string_view_impl<T> && (
        std::is_same_v<typename T::value_type, char8_t> ||
        std::is_same_v<typename T::value_type, char16_t> ||
        std::is_same_v<typename T::value_type, char32_t>
));

template<typename T>
concept unicode_string = (string_impl<T> && (
        std::is_same_v<typename T::value_type, char8_t> ||
        std::is_same_v<typename T::value_type, char16_t> ||
        std::is_same_v<typename T::value_type, char32_t>
));

// Determines course of action when encountered with an invalid sequence
enum class ErrorPolicy {
    UseReplacementCharacter, // Insert replacement character '�' on error
    SkipInvalidValues, // Skip invalid values and continue conversion to the best of its ability
    StopOnFirstError // Stop conversion on the first invalid value, return partial conversion
};

template<string_impl T>
struct ConversionResult {
    T value;
    bool is_valid;

    T& operator*() { return value; }
    T* operator->() { return &value; }
    const T* operator->() const { return &value; }
    explicit operator bool() const { return is_valid; }
};

namespace detail {

static constexpr inline const std::u8string_view REPLACEMENT_CHAR_8 = u8"�";
static constexpr inline const char16_t REPLACEMENT_CHAR_16 = u'�';
static constexpr inline const char32_t REPLACEMENT_CHAR_32 = U'�';

// ===== Implicit Conversions =====
template<typename FromChar, typename ToChar>
struct implicit_conversion : std::false_type{};
template<>
struct implicit_conversion<char, char8_t> : std::true_type{};

template<>
struct implicit_conversion<char8_t, char> : std::true_type{};

template<>
struct implicit_conversion<wchar_t, uchar_t> : std::true_type{};

template<>
struct implicit_conversion<uchar_t, wchar_t> : std::true_type{};

template<typename CharT>
struct implicit_conversion<CharT, CharT> : std::true_type{};

template<typename From, typename To>
inline constexpr bool is_implicitly_convertible = implicit_conversion<From, To>::value;

// ===== Specialized Conversions =====
inline ConversionResult<std::u8string> u8(const std::u8string_view u8s,
    [[maybe_unused]] const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return {std::u8string(u8s), true};
}
ConversionResult<std::u8string> u8(const std::u16string_view u16s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter);
ConversionResult<std::u8string> u8(const std::u32string_view u32s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter);

ConversionResult<std::u16string> u16(const std::u8string_view u8s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter);
inline ConversionResult<std::u16string> u16(const std::u16string_view u16s,
    [[maybe_unused]] const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return {std::u16string(u16s), true};
}
ConversionResult<std::u16string> u16(const std::u32string_view u32s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter);

ConversionResult<std::u32string> u32(const std::u8string_view u8s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter);
ConversionResult<std::u32string> u32(const std::u16string_view u16s, 
    const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter);
inline ConversionResult<std::u32string> u32(const std::u32string_view u32s,
    [[maybe_unused]] const ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return {std::u32string(u32s), true};
}

template<string_view_impl From , string_impl To>
requires is_implicitly_convertible<typename From::value_type, typename To::value_type>
To convert_implicitly(From from) {
    if constexpr (std::is_same_v<typename From::value_type, typename To::value_type>) {
        return To(from);
    } else {
#if defined(_MSC_VER) // would be nice to know the exact minimum _MSC_VER where this works
        return To(std::from_range, from);
    #elif __cpp_lib_ranges_to_container >= 202202L
        return from
            | std::ranges::views::transform([](typename From::value_type wc) {
                  return static_cast<typename To::value_type>(wc);
              })
            | std::ranges::to<To>();
    #else
        // C++20 fallback without ranges
        To out;
        out.reserve(from.size());
        for (auto it = from.cbegin(); it != from.cend(); ++it) {
            out.push_back(static_cast<typename To::value_type>(*it));
        }
        return out;
    #endif
    }
};


} // namespace detail

// "Dispatch" our functions based on conversion type //


// Implicit case: convert via static_cast or copying, e.g: Nstring <-> Nstring, string <-> u8string, wstring <-> ustring
template<string_view_impl From, string_impl To>
requires (detail::is_implicitly_convertible<From, To>)
inline ConversionResult<To> convert(From from, [[maybe_unused]] ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return {detail::convert_implicitly<From, To>(from), true};
}

// "Specialized" cases: both are unicode strings, use directly implemented methods
template<unicode_string_view From, unicode_string To>
requires (!detail::is_implicitly_convertible<From, To>)
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    using ToChar = typename To::value_type;
    if constexpr (std::is_same_v<ToChar, char8_t>) {
        return detail::u8(from, errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char16_t>) {
        return detail::u16(from, errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char32_t>) {
        return detail::u32(from, errorPolicy);
    }
}

// Convert wstring/string to unicode types first


// From wstring to string
template<string_view_impl From, string_impl To>
requires (!detail::is_implicitly_convertible<From, To> && std::is_same_v<typename From::value_type, wchar_t> && std::is_same_v<typename To::value_type, char>)
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return detail::u8(detail::convert_implicitly<From, ustring>(from), errorPolicy);
}

// From string to wstring
template<string_view_impl From, string_impl To>
requires (!detail::is_implicitly_convertible<From, To> && std::is_same_v<typename From::value_type, char> && std::is_same_v<typename To::value_type, wchar_t>)
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<ustring, To>(detail::convert_implicitly<From, std::u8string>(from), errorPolicy);
}

// From wstring to X
template<string_view_impl From, string_impl To>
requires (!detail::is_implicitly_convertible<From, To> && std::is_same_v<typename From::value_type, wchar_t> && !std::is_same_v<typename To::value_type, char>)
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<ustring, To>(detail::convert_implicitly<From, ustring>(from), errorPolicy);
}

// From string to X
template<string_view_impl From, string_impl To>
requires (!detail::is_implicitly_convertible<From, To> && std::is_same_v<typename From::value_type, char> && !std::is_same_v<typename To::value_type, wchar_t>)
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<std::u8string, To>(detail::convert_implicitly<From, std::u8string>(from), errorPolicy);
}

// From X to wstring
template<string_view_impl From, string_impl To>
requires (!detail::is_implicitly_convertible<From, To> && std::is_same_v<typename To::value_type, wchar_t>)
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    ConversionResult<ustring> inner = convert<From, ustring>(from, errorPolicy);
    return {detail::convert_implicitly<ustring, To>(inner.value), inner.is_valid};
}

// From X to string
template<string_view_impl From, string_impl To>
requires (!detail::is_implicitly_convertible<From, To> && std::is_same_v<typename To::value_type, char>)
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    ConversionResult<ustring> intermediate = convert<From, std::u8string>(from, errorPolicy);
    return {detail::convert_implicitly<std::u8string, To>(intermediate.value), intermediate.is_valid};
}

// Simple conversions to avoid ConversionResult
inline ustring ws_to_us(std::wstring_view from) {
    return detail::convert_implicitly<std::wstring_view, ustring>(from);
}

inline std::wstring us_to_ws(ustring_view from) {
    return detail::convert_implicitly<ustring_view, std::wstring>(from);
}

inline std::u8string s_to_u8s(std::string_view from) {
    return detail::convert_implicitly<std::string_view, std::u8string>(from);
}

inline std::string u8s_to_s(std::u8string_view from) {
    return detail::convert_implicitly<std::u8string_view, std::string>(from);
}

// "Advanced" conversions that require ConversionResult
template<string_view_impl From>
inline ConversionResult<std::u8string> u8s(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::u8string>(from, errorPolicy);
}

template<string_view_impl From>
inline ConversionResult<std::u16string> u16s(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::u16string>(from, errorPolicy);
}

template<string_view_impl From>
inline ConversionResult<std::u32string> u32s(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::u32string>(from, errorPolicy);
}

template<string_view_impl From>
inline ConversionResult<ustring> us(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, ustring>(from, errorPolicy);
}

template<string_view_impl From>
inline ConversionResult<std::wstring> ws(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::wstring>(from, errorPolicy);
}

template<string_view_impl From>
inline ConversionResult<std::string> s(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::string>(from, errorPolicy);
}

int uswidth(const std::u8string_view u8s);
int uswidth(const std::u16string_view u16s);
int uswidth(const std::u32string_view u32s);

inline int wswidth(const std::wstring_view ws) {
    ustring u = wutils::ws_to_us(ws);
    return wutils::uswidth(u);
}


} // namespace wutils

