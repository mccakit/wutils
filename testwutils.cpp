#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <print>
#include <typeinfo>
#include <locale>
#include <sstream>

#include "wutils.hpp"
#include "test.hpp"

int main() {
    using wutils::uchar_t, wutils::ustring, wutils::ustring_view;
    // Initialize locale
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());

    wutils::wprintln(L"Starting tests...");
    std::wstringstream wss1; wss1 << L"Detected wchar_t conversion type: " << typeid(uchar_t).name();
    wutils::wprintln(wss1.str());

    // Test Case 1: Simple ASCII string
    {
        constexpr int EXPECTED = 13;
        std::wstring ws_ascii = L"Hello, World!";
        ustring us_ascii = wutils::us(ws_ascii);
        std::wstring ws_converted = wutils::ws(us_ascii);
        std::wstringstream wss; wss << L"Length of " << ws_ascii << L": " << wutils::wswidth(ws_ascii);
        wutils::wprintln(wss.str());
        ASSERT_EQ(ws_converted, ws_ascii);
        ASSERT_EQ(EXPECTED, wutils::wswidth(ws_ascii));
        wutils::wprintln(L"Test 1 (ASCII): Passed");
    }

    // Test Case 2: Unicode character within the Basic Multilingual Plane (BMP)
    // The character 'é' (LATIN SMALL LETTER E WITH ACUTE) has code point U+00E9
    {
        // Use a wide string literal with the unicode character
        constexpr int EXPECTED = 6;
        std::wstring ws_unicode = L"Résumé";
        ustring us_unicode = wutils::us(ws_unicode);
        std::wstring ws_converted = wutils::ws(us_unicode);
        std::wstringstream wss; wss << L"Length of " << ws_unicode << L": " << wutils::wswidth(ws_unicode);
        wutils::wprintln(wss.str());
        ASSERT_EQ(ws_converted, ws_unicode);
        ASSERT_EQ(EXPECTED, wutils::wswidth(ws_unicode));
        wutils::wprintln(L"Test 2 (Unicode BMP): Passed");
    }

    // Test Case 3: Character requiring a surrogate pair (if wchar_t is 16 bits)
    // The character '😂' (FACE WITH TEARS OF JOY) has code point U+1F602
    {
        constexpr int EXPECTED = 6;
        std::wstring ws_surrogate = L"😂😂😂";
        ustring us_surrogate = wutils::us(ws_surrogate);
        std::wstring ws_converted = wutils::ws(us_surrogate);
        std::wstringstream wss; wss << L"Length of " << ws_surrogate << L": " << wutils::wswidth(ws_surrogate);
        wutils::wprintln(wss.str());
        if constexpr (wutils::wchar_is_char16) {
            // The original wstring and the converted one should match
            ASSERT_EQ(ws_converted, ws_surrogate);
            // The length in code units should also match, as from_range handles it
            ASSERT_EQ(us_surrogate.length(), ws_surrogate.length());
        }
        // The wswidth should also be correct
        ASSERT_EQ(EXPECTED, wutils::wswidth(ws_surrogate));
        wutils::wprintln(L"Test 3 (Surrogate Pairs): Passed");
    }
    // Test Case 4: Empty string
    {
        constexpr int EXPECTED = 0;
        std::wstring_view ws_empty = L"";
        ustring us_empty = wutils::us(ws_empty);
        std::wstring ws_converted = wutils::ws(us_empty);
        std::wstringstream wss; wss << L"Length of empty string: " << wutils::wswidth(ws_empty);
        wutils::wprintln(wss.str());
        ASSERT_TRUE(ws_converted.empty());
        ASSERT_TRUE(us_empty.empty());
        ASSERT_EQ(EXPECTED, wutils::wswidth(ws_empty));
        wutils::wprintln(L"Test 4 (Empty String): Passed");
    }

    wutils::wprintln(L"All tests completed successfully!");

    return 0;
}
