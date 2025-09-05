#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <typeinfo>
#include <locale>
#include <sstream>

#include "wutils.hpp"
#include "test.hpp"

template <typename U, typename V>
void assert_equal(const U& a, const V& b) {
    ASSERT_TRUE(a == b);
}

void test_width(std::wstring ws, const int expected) {
    int width = wutils::wswidth(ws);
    std::wstringstream wss; wss << L"Length of \"" << ws << L"\": " << width;
    wutils::wprintln(wss.str());

    wutils::wprintln(ws);
    
    // Print digits
    if (width >= 1) {
        std::wstringstream digits_lines;
        for (int i = 1; i <= width; i++) {
            digits_lines << (i % 10);
        }
        if (width >= 10) {
            digits_lines << L'\n';
            for (int i = 1; i <= width; i++) {
                digits_lines << ((i % 10) ? L" " : std::to_wstring(i / 10));
            }
        }
        wutils::wprintln(digits_lines.str() + L"\n");
    }
    ASSERT_EQ(expected, width);
}

template<typename From, typename To, typename Func>
void test_conversion(Func func, From in) {
    wutils::ConversionResult<To> result = func(in, wutils::ErrorPolicy::UseReplacementCharacter);
    ASSERT_TRUE(result);
    auto w_in = wutils::ws<From>(in);
    ASSERT_TRUE(w_in);
    auto w_out = wutils::ws<To>(*result);
    ASSERT_TRUE(w_out);

    ASSERT_EQ(*w_in, *w_out);
    std::wstringstream wss; wss << "Conversion successful: \"" << *w_in << L"\" == \"" << *w_out << L"\"";
    wutils::wprintln(wss.str());
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
        test_width(L"Hello, World!", 13);
        wutils::wprintln(L"Test 1 (ASCII): Passed\n-----");
    }

    // Test Case 2: Unicode character within the Basic Multilingual Plane (BMP)
    // The character 'é' (LATIN SMALL LETTER E WITH ACUTE) has code point U+00E9
    {
        test_width(L"Résumé", 6);
        wutils::wprintln(L"Test 2 (Unicode BMP): Passed\n-----");
    }

    // Test Case 3: Character requiring a surrogate pair (if wchar_t is 16 bits)
    // The character '😂' (FACE WITH TEARS OF JOY) has code point U+1F602
    {
        test_width(L"😂😂😂", 6);
        wutils::wprintln(L"Test 3 (Simple Emoji Surrogate Pair): Passed\n-----");
    }
    // Test Case 4: Empty string
    {
        test_width(L"", 0);
        wutils::wprintln(L"Test 4 (Empty String): Passed\n-----");
    }
    // Test Case 5: Advanced Emoji Sequence
    {
        // This single emoji (👩🏼‍🚀) is a set of 4 codepoints [128105] [127996] [8205] [128640]
        test_width(L"👩🏼‍🚀", 2);
        wutils::wprintln(L"Test 5 (Advanced Emoji Sequence): Passed\n-----");
    }
    // Test Case 6: Characters outside the Basic Multilingual Plane (Plane 0)
    {
        /* ===PLANE 1 (Supplementary Multilingual Plane)=== */

        test_width(L"𐌀𐌍𐌓𐌀", 4); // Old Italic
        test_width(L"𝕄𝕒𝕥𝕙𝕖𝕞𝕒𝕥𝕚𝕔𝕤", 11); // Mathematical Alpanumeric
        test_width(L"🌍🌎🌏", 6); // Emoji 1
        test_width(L"👨‍👩‍👧‍👦", 2); // Emoji 2
        wutils::wprintln(L"Test 6.1 (Supplementary Multilingual Plane): Passed\n-----");

        /* ===PLANE 2 (Supplementary Ideographic Plane)=== */
        test_width(L"𠔻𠕋𠖊𠖍𠖐", 10); // Rare Chinese Characters
        test_width(L"𠮷", 2); // Rare Japanese Variant
        test_width(L"𠀤𠀧𠁀", 6); // Rare Chinese Variants
        test_width(L"𠊛好", 4); // Vietnamese Chữ Nôm (CJK Extensions)
        test_width(L"𪚥𪆷𪃹", 6); // Rare Japanese Kanji (CJK Extensions)
        test_width(L"𪜈𪜋𪜌", 6); // Rare Korean Hanja (CJK Extensions)
        wutils::wprintln(L"Test 6.2 (Supplementary Ideographic Plane): Passed\n-----");

    }
    // Test case 7: Arabic
    {
        test_width(L"اَلْعَرَبِيَّةُ", 7);
        wutils::wprintln(L"Test 7 (Arabic): Passed\n-----");
    }

    // Test conversions
    {
        std::wstring w_in = L"Hello, World!";
        std::u8string u8_in = u8"Hello, World!";
        std::u16string u16_in = u"Hello, World!";
        std::u32string u32_in = U"Hello, World!";

        ustring u_out = wutils::ws_to_us(w_in);
        if constexpr (std::is_same_v<wutils::ustring, std::u8string>) {
            assert_equal(u8_in, u_out);
            wutils::wprintln(L"Assertion satisfied, u8_in == u_out");
        } else if constexpr (std::is_same_v<wutils::ustring, std::u16string>) {
            assert_equal(u16_in, u_out);
            wutils::wprintln(L"Assertion satisfied, u16_in == u_out");
        } else if constexpr (std::is_same_v<wutils::ustring, std::u32string>) {
            assert_equal(u32_in, u_out);
            wutils::wprintln(L"Assertion satisfied, u32_in == u_out");
        }

        // Wchar to ustrings
        test_conversion<std::wstring, std::u8string>(wutils::u8<std::wstring>, w_in);
        test_conversion<std::wstring, std::u16string>(wutils::u16<std::wstring>, w_in);
        test_conversion<std::wstring, std::u32string>(wutils::u32<std::wstring>, w_in);
        test_conversion<std::wstring, ustring>(wutils::us<std::wstring>, w_in);

        // Ustrings to wchar
        test_conversion<std::u8string, std::wstring>(wutils::ws<std::u8string>, u8_in);
        test_conversion<std::u16string, std::wstring>(wutils::ws<std::u16string>, u16_in);
        test_conversion<std::u32string, std::wstring>(wutils::ws<std::u32string>, u32_in);
        test_conversion<ustring, std::wstring>(wutils::ws<ustring>, u_out);

        // Between ustrings
        test_conversion<std::u8string, std::u8string>(wutils::u8<std::u8string>, u8_in);
        test_conversion<std::u16string, std::u8string>(wutils::u8<std::u16string>, u16_in);
        test_conversion<std::u32string, std::u8string>(wutils::u8<std::u32string>, u32_in);
        test_conversion<ustring, std::u8string>(wutils::u8<ustring>, u_out);

        test_conversion<std::u8string, std::u16string>(wutils::u16<std::u8string>, u8_in);
        test_conversion<std::u16string, std::u16string>(wutils::u16<std::u16string>, u16_in);
        test_conversion<std::u32string, std::u16string>(wutils::u16<std::u32string>, u32_in);
        test_conversion<ustring, std::u16string>(wutils::u16<ustring>, u_out);

        test_conversion<std::u8string, std::u32string>(wutils::u32<std::u8string>, u8_in);
        test_conversion<std::u16string, std::u32string>(wutils::u32<std::u16string>, u16_in);
        test_conversion<std::u32string, std::u32string>(wutils::u32<std::u32string>, u32_in);
        test_conversion<ustring, std::u32string>(wutils::u32<ustring>, u_out);

        test_conversion<std::u8string, ustring>(wutils::us<std::u8string>, u8_in);
        test_conversion<std::u16string, ustring>(wutils::us<std::u16string>, u16_in);
        test_conversion<std::u32string, ustring>(wutils::us<std::u32string>, u32_in);
        test_conversion<ustring, ustring>(wutils::us<ustring>, u_out);

        wutils::wprintln(L"All conversion tests passed!");

    }

    wutils::wprintln(L"All tests completed successfully!");

    return EXIT_SUCCESS;
}
