#include <cassert>
#include <iostream>
#include <string>
#include <print>

#include "wutils.hpp"

int main() {
    using namespace wutils;

    std::cout << "Starting tests..." << std::endl;
    std::cout << "Detected wchar_t size: " << sizeof(wchar_t) << " bytes" << std::endl;

    std::cout << std::boolalpha << "Is wchar_t same as char16_t? " << std::is_same_v<wchar_t, char16_t> << std::endl;
    std::cout << std::boolalpha << "Is wchar_t same as char32_t? " << std::is_same_v<wchar_t, char32_t> << std::endl;

    // Test Case 1: Simple ASCII string
    {
        std::wstring_view ws_ascii = L"Hello, World!";
        ustring us_ascii = ustring_from_wstring(ws_ascii);
        std::wstring ws_converted = wstring_from_ustring(us_ascii);
        assert(ws_converted == ws_ascii);
        assert(wswidth(ws_ascii) == ws_ascii.length());
        std::cout << "Test 1 (ASCII): Passed" << std::endl;
    }

    // Test Case 2: Unicode character within the Basic Multilingual Plane (BMP)
    // The character 'é' (LATIN SMALL LETTER E WITH ACUTE) has code point U+00E9
    {
        // Use a wide string literal with the unicode character
        std::wstring_view ws_unicode = L"Résumé";
        ustring us_unicode = ustring_from_wstring(ws_unicode);
        std::wstring ws_converted = wstring_from_ustring(us_unicode);
        assert(ws_converted == ws_unicode);
        assert(wswidth(ws_unicode) == ws_unicode.length());
        std::cout << "Test 2 (Unicode BMP): Passed" << std::endl;
    }

    // Test Case 3: Character requiring a surrogate pair (if wchar_t is 16 bits)
    // The character '😂' (FACE WITH TEARS OF JOY) has code point U+1F602
    if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
        std::cout << "Testing surrogate pair handling..." << std::endl;
        std::wstring_view ws_surrogate = L"Emoji: 😂";
        ustring us_surrogate = ustring_from_wstring(ws_surrogate);
        std::wstring ws_converted = wstring_from_ustring(us_surrogate);
        // The original wstring and the converted one should match
        assert(ws_converted == ws_surrogate);
        // The length in code units should also match, as from_range handles it
        assert(us_surrogate.length() == ws_surrogate.length());
        // The wswidth should also be correct
        assert(wswidth(ws_surrogate) == ws_surrogate.length());
        std::cout << "Test 3 (Surrogate Pairs): Passed" << std::endl;
    } else {
        std::cout << "Test 3 (Surrogate Pairs): Skipped (wchar_t is not 16-bit)" << std::endl;
    }

    // Test Case 4: Empty string
    {
        std::wstring_view ws_empty = L"";
        ustring us_empty = ustring_from_wstring(ws_empty);
        std::wstring ws_converted = wstring_from_ustring(us_empty);
        assert(ws_converted.empty());
        assert(us_empty.empty());
        assert(wswidth(ws_empty) == 0);
        std::cout << "Test 4 (Empty String): Passed" << std::endl;
    }

    std::cout << "All tests completed successfully!" << std::endl;

    return 0;
}
