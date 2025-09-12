#include <string>
#include <sstream>
#include <string_view>
#include <vector>
#include <locale>
#include <ranges>

#include "wutils.hpp"
#include "test.hpp"

using namespace std::string_literals;

// Requires support for C++26 #embed
// // Clang doesn't have this flag even though they do support #embed :(
// static_assert(__cpp_pp_embed >= 202502L, "Requires #embed support"); 
static constexpr std::u8string test_data() {

    uint8_t raw_test_data[] = {
        #embed "test_data.txt"
    };

    return raw_test_data
        | std::ranges::views::transform([](auto i) {
            return static_cast<char8_t>(i);
        })
        | std::ranges::to<std::u8string>();
}

struct InputData {
    int width;
    std::u8string text;
};


std::vector<InputData> parse_test_data() {

    std::vector<InputData> inputs;

    bool readWidth = true;
    std::string width_s = "";
    int width_n = 0;
    std::u8string text = u8"";

    // Read text file, format: 1st line = expected width, 2nd line = text to read
    for (const auto &c: test_data()) {
        if (c == u8'\n') {
            if (readWidth) {
                try {
                    width_n = std::stoi(width_s);
                } catch (const std::invalid_argument &e) {
                    throw;
                }
            } else {
                inputs.emplace_back(width_n, text);
                width_s.clear();
                text.clear();
            }

            readWidth = !readWidth;
            continue;
        } else {
            if (readWidth) {
                width_s += static_cast<char>(c);
            } else {
                text += c;
            }
        }
    }

    return inputs;
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

void run_tests(const int width, const std::u8string u8s) {
    // Test successful width with UTF8 u8string
    ASSERT_EQ(width, wutils::uswidth(u8s));

    // Test successful conversion and width to UTF16 and UTF32
    auto u16s = wutils::u16s(u8s);
    ASSERT_TRUE(u16s);
    ASSERT_EQ(width, wutils::uswidth(*u16s));
    
    auto u32s = wutils::u32s(u8s);
    ASSERT_TRUE(u32s);
    ASSERT_EQ(width, wutils::uswidth(*u32s));

    // Convert all back to u8string, check equality
    auto u8s_from_u16s = wutils::u8s(*u16s);
    ASSERT_TRUE(u8s_from_u16s);
    ASSERT_EQ(u8s, *u8s_from_u16s);

    auto u8s_from_u32s = wutils::u8s(*u32s);
    ASSERT_TRUE(u8s_from_u32s);
    ASSERT_EQ(u8s, *u8s_from_u32s);
}

int main() {
    std::vector<InputData> data = parse_test_data();

    // Initialize locale
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());
    wutils::wprintln(L"Running tests...");

    // Test strings from test_data file
    for (const auto& [width, text] : data) {
        auto ws = wutils::ws(text);
        ASSERT_TRUE(ws);
        wutils::wprintln(*ws);
        run_tests(width, text);
    }

    // Test Error Handling
    {
        wutils::wprintln(L"Testing Error Handling...");

        // 1. Invalid UTF-8 sequences
        const unsigned char invalid_u8_data[] = {
            'v', 'a', 'l', 'i', 'd', '_', 's', 't', 'a', 'r', 't', '_',
            0xC0, 0xAF, // Overlong '/' sequence
            '_', 'i', 'n', 'v', 'a', 'l', 'i', 'd', '_', 'm', 'i', 'd', 'd', 'l', 'e', '_',
            0xFF,     // Invalid byte
            '_', 'e', 'n', 'd'
        };
        std::u8string invalid_u8(reinterpret_cast<const char8_t*>(invalid_u8_data), sizeof(invalid_u8_data));
        // Test: UseReplacementCharacter
        auto res_u8_rep = wutils::u32s(invalid_u8, wutils::ErrorPolicy::UseReplacementCharacter);
        ASSERT_FALSE(res_u8_rep.is_valid);
        const std::u32string expected_u32_rep = U"valid_start_"s + wutils::detail::REPLACEMENT_CHAR_32 + wutils::detail::REPLACEMENT_CHAR_32 + U"_invalid_middle_" + wutils::detail::REPLACEMENT_CHAR_32 + U"_end";
        ASSERT_EQ(res_u8_rep.value, expected_u32_rep);
        wutils::wprintln(L"Test 8.1 (Invalid UTF-8: UseReplacementCharacter): Passed");

        // Test: SkipInvalidValues
        auto res_u8_skip = wutils::u32s(invalid_u8, wutils::ErrorPolicy::SkipInvalidValues);
        ASSERT_FALSE(res_u8_skip.is_valid);
        ASSERT_EQ(res_u8_skip.value, U"valid_start__invalid_middle__end");
        wutils::wprintln(L"Test 8.2 (Invalid UTF-8: SkipInvalidValues): Passed");

        // Test: StopOnFirstError
        auto res_u8_stop = wutils::u32s(invalid_u8, wutils::ErrorPolicy::StopOnFirstError);
        ASSERT_FALSE(res_u8_stop.is_valid);
        ASSERT_EQ(res_u8_stop.value, U"valid_start_");
        wutils::wprintln(L"Test 8.3 (Invalid UTF-8: StopOnFirstError): Passed");

        // 2. Invalid UTF-16 sequences (unpaired surrogates)
        const char16_t invalid_u16_data[] = {
            u'v', u'a', u'l', u'i', u'd', u'_',
            0xD800, // Unpaired high surrogate
            u'_', u'i', u'n', u'v', u'a', u'l', u'i', u'd', u'_',
            0xDFFF, // Unpaired low surrogate
            u'_', u'e', u'n', u'd'
        };
        std::u16string invalid_u16(invalid_u16_data, sizeof(invalid_u16_data) / sizeof(char16_t));

        // Test: UseReplacementCharacter
        auto res_u16_rep = wutils::u8s(invalid_u16, wutils::ErrorPolicy::UseReplacementCharacter);
        ASSERT_FALSE(res_u16_rep.is_valid);
        const std::u8string expected_u8_rep = u8"valid_"s + std::u8string(wutils::detail::REPLACEMENT_CHAR_8) + u8"_invalid_" + std::u8string(wutils::detail::REPLACEMENT_CHAR_8) + u8"_end";
        ASSERT_EQ(res_u16_rep.value, expected_u8_rep);
        wutils::wprintln(L"Test 9.1 (Invalid UTF-16: UseReplacementCharacter): Passed");

        // Test: SkipInvalidValues
        auto res_u16_skip = wutils::u8s(invalid_u16, wutils::ErrorPolicy::SkipInvalidValues);
        ASSERT_FALSE(res_u16_skip.is_valid);
        ASSERT_EQ(res_u16_skip.value, u8"valid__invalid__end");
        wutils::wprintln(L"Test 9.2 (Invalid UTF-16: SkipInvalidValues): Passed");

        // Test: StopOnFirstError
        auto res_u16_stop = wutils::u8s(invalid_u16, wutils::ErrorPolicy::StopOnFirstError);
        ASSERT_FALSE(res_u16_stop.is_valid);
        ASSERT_EQ(res_u16_stop.value, u8"valid_");
        wutils::wprintln(L"Test 9.3 (Invalid UTF-16: StopOnFirstError): Passed");

        wutils::wprintln(L"All error handling tests passed!");
    }

    wutils::wprintln(L"All tests passed! 😄");

}
