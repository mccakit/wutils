#pragma once

#include <wchar.h>
#include <uchar.h>
#include <string>
#include <ranges>

namespace wutils {

size_t uswidth(const std::u32string_view u32s);
size_t uswidth(const std::u16string_view u16s);

static constexpr bool wchar_is_char16 = sizeof(wchar_t) == sizeof(char16_t);
static constexpr bool wchar_is_char32 = sizeof(wchar_t) == sizeof(char32_t);

// A sanity check to ensure the size matches an expected type
static_assert(wchar_is_char16 || wchar_is_char32,
              "Unsupported wchar_t width, expecting 16 or 32 bits");

// Determine the underlying type for ustring at compile-time

using uchar_t = std::conditional_t<wchar_is_char16, char16_t, char32_t>;
using ustring = std::basic_string<uchar_t>;
using ustring_view = std::basic_string_view<uchar_t>;


// Final sanity check
static_assert(sizeof(wchar_t) == sizeof(ustring::value_type) && sizeof(wchar_t) == sizeof(ustring_view::value_type), "Invalid wchar_t deduction");

#ifdef _MSC_VER // MSVC-only optimization
inline ustring ustring_from_wstring(const std::wstring_view ws) {
    return ustring(std::from_range, ws);
}

inline std::wstring wstring_from_ustring(const ustring_view us) {
    return std::wstring(std::from_range, us);
}
#else
inline ustring ustring_from_wstring(std::wstring_view ws) {
    return ws | std::ranges::views::transform([](wchar_t wc) {
        return static_cast<char32_t>(wc);
    }) | std::ranges::to<ustring>();
}

inline std::wstring wstring_from_ustring(ustring_view us) {

    return us | std::ranges::views::transform([](uchar_t uc) {
        return static_cast<wchar_t>(uc);
    }) | std::ranges::to<std::wstring>();
} 

#endif

inline size_t wswidth(const std::wstring_view ws) {
    ustring us = ustring_from_wstring(ws);
    return uswidth(us);
}

} // namespace wutils
