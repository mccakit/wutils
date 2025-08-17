#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <print>
#include <typeinfo>
#include <format>
#include <locale>
#include <sstream>

// Windows-specific headers for console I/O
#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

#include "wutils.hpp"

template<typename T>
void assert_eq(const T expected, const T actual) {
    if (expected != actual) {
        std::wstringstream wss;
        wss << L"Assertion failed: expected " << expected << L", got " << actual;
        wutils::wprintln(wss.str());
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    using namespace wutils;
#if defined(_WIN32)
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#else
    // Initialize locale
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());
#endif

    wprintln(L"Starting tests...");
    // std::wcout << L"Detected wchar_t conversion type: " << typeid(uchar_t).name() << std::endl;

    // Test Case 1: Simple ASCII string
    {
        constexpr int EXPECTED = 13;
        std::wstring ws_ascii = L"Hello, World!";
        ustring us_ascii = ustring_from_wstring(ws_ascii);
        std::wstring ws_converted = wstring_from_ustring(us_ascii);
        std::wstringstream wss; wss << L"Length of " << ws_ascii << L": " << wswidth(ws_ascii);
        wprintln(wss.str());
        assert(ws_converted == ws_ascii);
        assert_eq(EXPECTED, wswidth(ws_ascii));
        std::wstringstream wss2; wss2 << L"Test 1 (ASCII): Passed";
        wprintln(wss2.str());
    }

    // Test Case 2: Unicode character within the Basic Multilingual Plane (BMP)
    // The character 'é' (LATIN SMALL LETTER E WITH ACUTE) has code point U+00E9
    {
        // Use a wide string literal with the unicode character
        constexpr int EXPECTED = 6;
        std::wstring ws_unicode = L"Résumé";
        ustring us_unicode = ustring_from_wstring(ws_unicode);
        std::wstring ws_converted = wstring_from_ustring(us_unicode);
        std::wstringstream wss; wss << L"Length of " << ws_unicode << L": " << wswidth(ws_unicode);
        wprintln(wss.str());
        assert(ws_converted == ws_unicode);
        assert_eq(EXPECTED, wswidth(ws_unicode));
        std::wstringstream wss2; wss2 << "Test 2 (Unicode BMP): Passed";
        wprintln(wss2.str());
    }

    // Test Case 3: Character requiring a surrogate pair (if wchar_t is 16 bits)
    // The character '😂' (FACE WITH TEARS OF JOY) has code point U+1F602
    {
        constexpr int EXPECTED = 3;
        std::wstring ws_surrogate = L"😂😂😂";
        ustring us_surrogate = ustring_from_wstring(ws_surrogate);
        std::wstring ws_converted = wstring_from_ustring(us_surrogate);
        std::wstringstream wss; wss << L"Length of " << ws_surrogate << L": " << wswidth(ws_surrogate);
        wprintln(wss.str());
        if constexpr (wchar_is_char16) {
            // The original wstring and the converted one should match
            assert(ws_converted == ws_surrogate);
            // The length in code units should also match, as from_range handles it
            assert(us_surrogate.length() == ws_surrogate.length());
        }
        // The wswidth should also be correct
        assert_eq(EXPECTED, wswidth(ws_surrogate));
        std::wstringstream wss2; wss2 << "Test 3 (Surrogate Pairs): Passed";
        wprintln(wss2.str());
    }
    // Test Case 4: Empty string
    {
        constexpr int EXPECTED = 0;
        std::wstring_view ws_empty = L"";
        ustring us_empty = ustring_from_wstring(ws_empty);
        std::wstring ws_converted = wstring_from_ustring(us_empty);
        std::wstringstream wss; wss << L"Length of empty string: " << wswidth(ws_empty);
        wprintln(wss.str());
        assert(ws_converted.empty());
        assert(us_empty.empty());
        assert_eq(EXPECTED, wswidth(ws_empty));
        std::wstringstream wss2; wss2 << L"Test 4 (Empty String): Passed";
        wprintln(wss2.str());
    }

    std::wstringstream wss; wss << "All tests completed successfully!";
    wprintln(wss.str());

    return 0;
}
