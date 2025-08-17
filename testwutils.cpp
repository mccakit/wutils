#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <print>
#include <typeinfo>
#include <format>
#include <locale>

#include "wutils.hpp"

template<typename T>
void assert_eq(const T expected, const T actual) {
    if (expected != actual) {
        std::wcout << L"Assertion failed: expected " << expected << ", got " << actual << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    using namespace wutils;

    // Initialize locale
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());

    std::wcout << L"Starting tests..." << std::endl;
    std::wcout << L"Detected wchar_t conversion type: " << typeid(uchar_t).name() << std::endl;

    // Test Case 1: Simple ASCII string
    {
        constexpr int EXPECTED = 13;
        std::wstring ws_ascii = L"Hello, World!";
        ustring us_ascii = ustring_from_wstring(ws_ascii);
        std::wstring ws_converted = wstring_from_ustring(us_ascii);
        std::wcout << L"Length of " << ws_ascii << L": " << wswidth(ws_ascii) << std::endl;
        assert(ws_converted == ws_ascii);
        assert_eq(EXPECTED, wswidth(ws_ascii));
        std::wcout << L"Test 1 (ASCII): Passed" << std::endl;
    }

    // Test Case 2: Unicode character within the Basic Multilingual Plane (BMP)
    // The character 'é' (LATIN SMALL LETTER E WITH ACUTE) has code point U+00E9
    {
        // Use a wide string literal with the unicode character
        constexpr int EXPECTED = 6;
        std::wstring ws_unicode = L"Résumé";
        ustring us_unicode = ustring_from_wstring(ws_unicode);
        std::wstring ws_converted = wstring_from_ustring(us_unicode);
        std::wcout << L"Length of " << ws_unicode << L": " << wswidth(ws_unicode) << std::endl;
        assert(ws_converted == ws_unicode);
        assert_eq(EXPECTED, wswidth(ws_unicode));
        std::wcout << "Test 2 (Unicode BMP): Passed" << std::endl;
    }

    // Test Case 3: Character requiring a surrogate pair (if wchar_t is 16 bits)
    // The character '😂' (FACE WITH TEARS OF JOY) has code point U+1F602
    {
        constexpr int EXPECTED = 3;
        std::wstring ws_surrogate = L"😂😂😂";
        ustring us_surrogate = ustring_from_wstring(ws_surrogate);
        std::wstring ws_converted = wstring_from_ustring(us_surrogate);
        std::wcout << L"Length of " << ws_surrogate << L": " << wswidth(ws_surrogate) << std::endl;
        if constexpr (wchar_is_char16) {
            // The original wstring and the converted one should match
            assert(ws_converted == ws_surrogate);
            // The length in code units should also match, as from_range handles it
            assert(us_surrogate.length() == ws_surrogate.length());
        }
        // The wswidth should also be correct
        assert_eq(EXPECTED, wswidth(ws_surrogate));
        std::wcout << "Test 3 (Surrogate Pairs): Passed" << std::endl;
    }
    // Test Case 4: Empty string
    {
        constexpr int EXPECTED = 0;
        std::wstring_view ws_empty = L"";
        ustring us_empty = ustring_from_wstring(ws_empty);
        std::wstring ws_converted = wstring_from_ustring(us_empty);
        std::wcout << L"Length of empty string: " << wswidth(ws_empty) << std::endl;
        assert(ws_converted.empty());
        assert(us_empty.empty());
        assert_eq(EXPECTED, wswidth(ws_empty));
        std::wcout << L"Test 4 (Empty String): Passed" << std::endl;
    }

    std::wcout << "All tests completed successfully!" << std::endl;

    return 0;
}
