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

void test_case(std::wstring ws, const int expected) {
    int width = wutils::wswidth(ws);
    std::wstringstream wss; wss << L"Length of \"" << ws << L"\": " << width;
    wutils::wprintln(wss.str());
    ASSERT_EQ(expected, width);
}

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
        test_case(L"Hello, World!", 13);
        wutils::wprintln(L"Test 1 (ASCII): Passed");
    }

    // Test Case 2: Unicode character within the Basic Multilingual Plane (BMP)
    // The character 'é' (LATIN SMALL LETTER E WITH ACUTE) has code point U+00E9
    {
        test_case(L"Résumé", 6);
        wutils::wprintln(L"Test 2 (Unicode BMP): Passed");
    }

    // Test Case 3: Character requiring a surrogate pair (if wchar_t is 16 bits)
    // The character '😂' (FACE WITH TEARS OF JOY) has code point U+1F602
    {
        test_case(L"😂😂😂", 6);
        wutils::wprintln(L"Test 3 (Simple Emoji Surrogate Pair): Passed");
    }
    // Test Case 4: Empty string
    {
        test_case(L"", 0);
        wutils::wprintln(L"Test 4 (Empty String): Passed");
    }
    // Test Case 5: Advanced Emoji Sequence
    {
        // This single emoji (👩🏼‍🚀) is a set of 4 codepoints [128105] [127996] [8205] [128640]
        test_case(L"👩🏼‍🚀", 2);
        wutils::wprintln(L"Test 5 (Advanced Emoji Sequence): Passed");
    }
    // Test Case 6: Characters outside the Basic Multilingual Plane (Plane 0)
    {
        /* ===PLANE 1 (Supplementary Multilingual Plane)=== */

        test_case(L"𐌀𐌍𐌓𐌀", 4); // Old Italic
        test_case(L"𝕄𝕒𝕥𝕙𝕖𝕞𝕒𝕥𝕚𝕔𝕤", 11); // Mathematical Alpanumeric
        test_case(L"🌍🌎🌏", 6); // Emoji 1
        test_case(L"👨‍👩‍👧‍👦", 2); // Emoji 2
        wutils::wprintln(L"Test 6.1 (Supplementary Multilingual Plane): Passed");
       
        /* ===PLANE 2 (Supplementary Ideographic Plane)=== */
        test_case(L"𠔻𠕋𠖊𠖍𠖐", 10); // Rare Chinese Characters
        test_case(L"𠮷", 2); // Rare Japanese Variant
        test_case(L"𠀤𠀧𠁀", 6); // Rare Chinese Variants
        test_case(L"𠊛好", 4); // Vietnamese Chữ Nôm (CJK Extensions)
        test_case(L"𪚥𪆷𪃹", 6); // Rare Japanese Kanji (CJK Extensions)
        test_case(L"𪜈𪜋𪜌", 6); // Rare Korean Hanja (CJK Extensions)
        wutils::wprintln(L"Test 6.2 (Supplementary Ideographic Plane): Passed");

    }

    wutils::wprintln(L"All tests completed successfully!");

    return 0;
}
