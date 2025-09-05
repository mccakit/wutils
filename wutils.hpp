#pragma once

#include <wchar.h>
#include <uchar.h>
#include <concepts>
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

template<typename T>
concept string_view_like = requires(const T& t) {
    typename T::value_type;
    typename T::const_iterator;

    { t.data() } -> std::same_as<const typename T::value_type*>;
    { t.size() } -> std::same_as<std::size_t>;
    { t.cbegin() } -> std::same_as<typename T::const_iterator>;
    { t.cend() } -> std::same_as<typename T::const_iterator>;
    { *t.cbegin() } -> std::same_as<const typename T::value_type&>;
};

template<typename T>
concept string_like = string_view_like<T> && requires(T t) {
    typename T::iterator;

    { t.begin() } -> std::same_as<typename T::iterator>;
    { t.end() } -> std::same_as<typename T::iterator>;
    { *t.begin() } -> std::same_as<typename T::value_type&>;
    { t.append(t) };
    { t.push_back(std::declval<typename T::value_type>()) };
};

 

// Determines course of action when encountered with an invalid sequence
enum class ErrorPolicy {
    UseReplacementCharacter, // Insert replacement character '�' on error
    SkipInvalidValues, // Skip invalid values and continue conversion to the best of its ability
    StopOnFirstError // Stop conversion on the first invalid value, return partial conversion
};

template<string_like T>
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

template<string_view_like From, string_like To>
struct specialized_conversion {
    static inline constexpr bool value = false;
};

template<>
struct specialized_conversion<std::u16string_view, std::u8string> {
    static inline constexpr bool value = true;
    inline ConversionResult<std::u8string> convert(const std::u16string_view u16s, ErrorPolicy errorPolicy) {
        return detail::u8(u16s, errorPolicy);
    }
};

template<>
struct specialized_conversion<std::u32string_view, std::u8string> {
    static inline constexpr bool value = true;
    inline ConversionResult<std::u8string> convert(const std::u32string_view u32s, ErrorPolicy errorPolicy) {
        return detail::u8(u32s, errorPolicy);
    }
};

template<>
struct specialized_conversion<std::u8string_view, std::u16string> {
    static inline constexpr bool value = true;
    inline ConversionResult<std::u16string> convert(const std::u8string_view u8s, ErrorPolicy errorPolicy) {
        return detail::u16(u8s, errorPolicy);
    }
};

template<>
struct specialized_conversion<std::u32string_view, std::u16string> {
    static inline constexpr bool value = true;
    inline ConversionResult<std::u16string> convert(const std::u32string_view u32s, ErrorPolicy errorPolicy) {
        return detail::u16(u32s, errorPolicy);
    }
};

template<>
struct specialized_conversion<std::u8string_view, std::u32string> {
    static inline constexpr bool value = true;
    inline ConversionResult<std::u32string> convert(const std::u8string_view u8s, ErrorPolicy errorPolicy) {
        return detail::u32(u8s, errorPolicy);
    }
};

template<>
struct specialized_conversion<std::u16string_view, std::u32string> {
    static inline constexpr bool value = true;
    inline ConversionResult<std::u32string> convert(const std::u16string_view u16s, ErrorPolicy errorPolicy) {
        return detail::u32(u16s, errorPolicy);
    }
};

template<string_view_like From, string_like To>
inline constexpr bool has_specialized_conversion = specialized_conversion<From, To>::value;

template<string_view_like From , string_like To>
requires is_implicitly_convertible<typename From::value_type, typename To::value_type>
To convert_implicitly(From from) {
    if constexpr (std::is_same_v<typename From::value_type, typename To::value_type>) {
        return To(from);
    } else {
#ifdef _MSC_VER // MSVC specific optimization
         return To(std::from_range, from);
#else // Generic conversion
        return from | std::ranges::views::transform([](From::value_type wc) {
            return static_cast<To::value_type>(wc);
        }) | std::ranges::to<To>();
#endif
    }
};

template<string_view_like From>
ConversionResult<ustring> convert_to_ustring(From from, ErrorPolicy errorPolicy) {
    using FromChar = From::value_type;
    if constexpr (std::is_same_v<FromChar, uchar_t>) {
        return {ustring(from), true};
    } else if constexpr (std::is_same_v<FromChar, wchar_t>) {
        return {detail::convert_implicitly<From, ustring>(from), true};
    } else if constexpr (std::is_same_v<uchar_t, char8_t>) {
        return detail::u8(from, errorPolicy);
    } else if constexpr (std::is_same_v<uchar_t, char16_t>) {
        return detail::u16(from, errorPolicy);
    } else if constexpr (std::is_same_v<uchar_t, char32_t>) {
        return detail::u32(from, errorPolicy);
    }
}

template<string_like To>
ConversionResult<To> convert_from_ustring(ustring_view from, ErrorPolicy errorPolicy) {
    using ToChar = To::value_type;
    if constexpr (std::is_same_v<uchar_t, ToChar>) {
        return {To(from), true};
    } else if constexpr (std::is_same_v<ToChar, wchar_t>) {
        return {detail::convert_implicitly<ustring_view, To>(from), true};
    } else if constexpr (std::is_same_v<ToChar, char8_t>) {
        return detail::u8(from, errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char16_t>) {
        return detail::u16(from, errorPolicy);
    } else if constexpr (std::is_same_v<ToChar, char32_t>) {
        return detail::u32(from, errorPolicy);
    } 
}

template<string_view_like From, string_like To>
ConversionResult<To> do_intermediate_conversion(From from, ErrorPolicy errorPolicy) {
    ConversionResult<ustring> inner = detail::convert_to_ustring<From>(from, errorPolicy);
    ConversionResult<To> outer = detail::convert_from_ustring<To>(inner.value, errorPolicy);
    return {outer.value, (inner.is_valid && outer.is_valid)};
}

} // namespace detail

// "Dispatch" our functions based on conversion type //

// General case: convert via intermediate: From -> ustring -> To
template<string_view_like From, string_like To>
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return detail::do_intermediate_conversion<From, To>(from, errorPolicy);
}

// Implicit case: convert via static_cast or copying, e.g: Nstring <-> Nstring, string <-> u8string, wstring <-> ustring
template<string_view_like From, string_like To>
requires detail::is_implicitly_convertible<From, To>
inline ConversionResult<To> convert(From from, [[maybe_unused]] ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return {detail::convert_implicitly<From, To>(from), true};
}

// Specialized conversion: conversions that are directly implemented: u8 <-> u16, u16 <-> u32, etc.
template<string_view_like From, string_like To>
requires detail::has_specialized_conversion<From, To>
inline ConversionResult<To> convert(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return detail::specialized_conversion<From, To>{}.convert(from, errorPolicy);
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
template<string_view_like From>
inline ConversionResult<std::u8string> u8(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::u8string>(from, errorPolicy);
}

template<string_view_like From>
inline ConversionResult<std::u16string> u16(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::u16string>(from, errorPolicy);
}

template<string_view_like From>
inline ConversionResult<std::u32string> u32(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::u32string>(from, errorPolicy);
}

template<string_view_like From>
inline ConversionResult<ustring> us(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, ustring>(from, errorPolicy);
}

template<string_view_like From>
inline ConversionResult<std::wstring> ws(From from, ErrorPolicy errorPolicy = ErrorPolicy::UseReplacementCharacter) {
    return convert<From, std::wstring>(from, errorPolicy);
}

int uswidth(const std::u8string_view u8s);
int uswidth(const std::u16string_view u16s);
int uswidth(const std::u32string_view u32s);

inline int wswidth(const std::wstring_view ws) {
    ustring u = wutils::ws_to_us(ws);
    return wutils::uswidth(u);
}


} // namespace wutils

