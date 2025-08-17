#include <cassert>
#include <iostream>
#include <string>
#include <print>
#include <typeinfo>

#include "wutils.hpp"

int main() {
    using namespace wutils;

    std::println("Starting tests...");
    std::println("Detected wchar_t conversion type: {}", typeid(uchar_t).name());

    // Test Case 1: Simple ASCII string
    {
        constexpr size_t EXPECTED = 13;
        std::wstring_view ws_ascii = L"Hello, World!";
        ustring us_ascii = ustring_from_wstring(ws_ascii);
        std::wstring ws_converted = wstring_from_ustring(us_ascii);
        assert(ws_converted == ws_ascii);
        assert(wswidth(ws_ascii) == EXPECTED);
        std::println("Test 1 (ASCII): Passed");
    }

    // Test Case 2: Unicode character within the Basic Multilingual Plane (BMP)
    // The character 'é' (LATIN SMALL LETTER E WITH ACUTE) has code point U+00E9
    {
        // Use a wide string literal with the unicode character
        constexpr size_t EXPECTED = 6;
        std::wstring_view ws_unicode = L"Résumé";
        ustring us_unicode = ustring_from_wstring(ws_unicode);
        std::wstring ws_converted = wstring_from_ustring(us_unicode);
        assert(ws_converted == ws_unicode);
        assert(wswidth(ws_unicode) == EXPECTED);
        std::println("Test 2 (Unicode BMP): Passed");
    }

    // Test Case 3: Character requiring a surrogate pair (if wchar_t is 16 bits)
    // The character '😂' (FACE WITH TEARS OF JOY) has code point U+1F602
    {
        constexpr size_t EXPECTED = 3;
        std::wstring_view ws_surrogate = L"😂😂😂";
        ustring us_surrogate = ustring_from_wstring(ws_surrogate);
        std::wstring ws_converted = wstring_from_ustring(us_surrogate);
        if constexpr (wchar_is_char16) {
            // The original wstring and the converted one should match
            assert(ws_converted == ws_surrogate);
            // The length in code units should also match, as from_range handles it
            assert(us_surrogate.length() == ws_surrogate.length());
        }
        // The wswidth should also be correct
        assert(wswidth(ws_surrogate) == EXPECTED);
        std::println("Test 3 (Surrogate Pairs): Passed");
    }
    // Test Case 4: Empty string
    {
        std::wstring_view ws_empty = L"";
        ustring us_empty = ustring_from_wstring(ws_empty);
        std::wstring ws_converted = wstring_from_ustring(us_empty);
        assert(ws_converted.empty());
        assert(us_empty.empty());
        assert(wswidth(ws_empty) == 0);
        std::println("Test 4 (Empty String): Passed");
    }

    std::println("All tests completed successfully!");

    return 0;
}
