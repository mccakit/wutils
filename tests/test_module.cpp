#include <gtest/gtest.h>
import std;
import wutils;

using namespace std::string_literals;

struct InputData {
  int width;
  std::u8string text;
};

std::array<InputData, 16> test_data{{{13, u8"Hello, World!"},
                                     {6, u8"Résumé"},
                                     {6, u8"😂😂😂"},
                                     {0, u8""},
                                     {2, u8"👩🏼‍🚀"},
                                     {4, u8"𐌀𐌍𐌓𐌀"},
                                     {11, u8"𝕄𝕒𝕥𝕙𝕖𝕞𝕒𝕥𝕚𝕔𝕤"},
                                     {6, u8"🌍🌎🌏"},
                                     {2, u8"👨‍👩‍👧‍👦"},
                                     {10, u8"𠔻𠕋𠖊𠖍𠖐"},
                                     {2, u8"𠮷"},
                                     {6, u8"𠀤𠀧𠁀"},
                                     {4, u8"𠊛好"},
                                     {6, u8"𪚥𪆷𪃹"},
                                     {6, u8"𪜈𪜋𪜌"},
                                     {7, u8"اَلْعَرَبِيَّةُ"}}};

TEST(WidthCalculation, AllTestCases) {
  for (const auto &[width, text] : test_data) {
    SCOPED_TRACE("Testing width of: " + std::string(text.begin(), text.end()));
    EXPECT_EQ(wutils::uswidth(text), width);
  }
}

TEST(StringConversions, AllTestCases) {
  for (const auto &[width, u8s] : test_data) {
    SCOPED_TRACE("Testing conversions for: " + std::string(u8s.begin(), u8s.end()));

    // u8 -> u16 -> u8
    auto u16s = wutils::u16s(u8s);
    ASSERT_TRUE(u16s);
    EXPECT_EQ(wutils::uswidth(*u16s), width);
    auto u8s_from_u16s = wutils::u8s(*u16s);
    ASSERT_TRUE(u8s_from_u16s);
    EXPECT_EQ(*u8s_from_u16s, u8s);

    // u8 -> u32 -> u8
    auto u32s = wutils::u32s(u8s);
    ASSERT_TRUE(u32s);
    EXPECT_EQ(wutils::uswidth(*u32s), width);
    auto u8s_from_u32s = wutils::u8s(*u32s);
    ASSERT_TRUE(u8s_from_u32s);
    EXPECT_EQ(*u8s_from_u32s, u8s);

    // u8 -> wstring -> u8
    auto ws = wutils::ws(u8s);
    ASSERT_TRUE(ws);
    EXPECT_EQ(wutils::wswidth(*ws), width);
    auto u8s_from_ws = wutils::u8s(*ws);
    ASSERT_TRUE(u8s_from_ws);
    EXPECT_EQ(*u8s_from_ws, u8s);

    // u8 -> string -> u8
    auto s = wutils::s(u8s);
    ASSERT_TRUE(s);
    auto u8s_from_s = wutils::u8s(*s);
    ASSERT_TRUE(u8s_from_s);
    EXPECT_EQ(*u8s_from_s, u8s);
  }
}

TEST(ErrorHandling, InvalidUTF8UseReplacementCharacter) {
  const char8_t invalid_u8_data[] = {
      's',  't', 'a', 'r', 't', '_', 0xC0, 0xAF, // Overlong '/' sequence
      '_',  'm', 'i', 'd', 'd', 'l', 'e',  '_',
      0xFF, // Invalid byte
      '_',  'e', 'n', 'd'};
  std::u8string invalid_u8(invalid_u8_data,
                           sizeof(invalid_u8_data) / sizeof(char8_t));

  auto res = wutils::u32s(invalid_u8,
                          wutils::ErrorPolicy::UseReplacementCharacter);
  EXPECT_FALSE(res.is_valid);
  const std::u32string expected =
      U"start_"s + wutils::detail::REPLACEMENT_CHAR_32 +
      wutils::detail::REPLACEMENT_CHAR_32 + U"_middle_" +
      wutils::detail::REPLACEMENT_CHAR_32 + U"_end";
  EXPECT_EQ(res.value, expected);
}

TEST(ErrorHandling, InvalidUTF8SkipInvalidValues) {
  const char8_t invalid_u8_data[] = {
      's',  't', 'a', 'r', 't', '_', 0xC0, 0xAF, // Overlong '/' sequence
      '_',  'm', 'i', 'd', 'd', 'l', 'e',  '_',
      0xFF, // Invalid byte
      '_',  'e', 'n', 'd'};
  std::u8string invalid_u8(invalid_u8_data,
                           sizeof(invalid_u8_data) / sizeof(char8_t));

  auto res = wutils::u32s(invalid_u8, wutils::ErrorPolicy::SkipInvalidValues);
  EXPECT_FALSE(res.is_valid);
  EXPECT_EQ(res.value, U"start__middle__end");
}

TEST(ErrorHandling, InvalidUTF8StopOnFirstError) {
  const char8_t invalid_u8_data[] = {
      's',  't', 'a', 'r', 't', '_', 0xC0, 0xAF, // Overlong '/' sequence
      '_',  'm', 'i', 'd', 'd', 'l', 'e',  '_',
      0xFF, // Invalid byte
      '_',  'e', 'n', 'd'};
  std::u8string invalid_u8(invalid_u8_data,
                           sizeof(invalid_u8_data) / sizeof(char8_t));

  auto res = wutils::u32s(invalid_u8, wutils::ErrorPolicy::StopOnFirstError);
  EXPECT_FALSE(res.is_valid);
  EXPECT_EQ(res.value, U"start_");
}

TEST(ErrorHandling, InvalidUTF16UseReplacementCharacter) {
  const char16_t invalid_u16_data[] = {u's',   u't', u'a', u'r', u't', u'_',
                                       0xD800, // Unpaired high surrogate
                                       u'_',   u'm', u'i', u'd', u'd', u'l',
                                       u'e',   u'_',
                                       0xDFFF, // Unpaired low surrogate
                                       u'_',   u'e', 'n',  'd'};
  std::u16string invalid_u16(invalid_u16_data,
                             sizeof(invalid_u16_data) / sizeof(char16_t));

  auto res = wutils::u8s(invalid_u16,
                         wutils::ErrorPolicy::UseReplacementCharacter);
  EXPECT_FALSE(res.is_valid);
  const std::u8string expected =
      u8"start_"s + std::u8string(wutils::detail::REPLACEMENT_CHAR_8) +
      u8"_middle_" + std::u8string(wutils::detail::REPLACEMENT_CHAR_8) +
      u8"_end";
  EXPECT_EQ(res.value, expected);
}

TEST(ErrorHandling, InvalidUTF16SkipInvalidValues) {
  const char16_t invalid_u16_data[] = {u's',   u't', u'a', u'r', u't', u'_',
                                       0xD800, // Unpaired high surrogate
                                       u'_',   u'm', u'i', u'd', u'd', u'l',
                                       u'e',   u'_',
                                       0xDFFF, // Unpaired low surrogate
                                       u'_',   u'e', 'n',  'd'};
  std::u16string invalid_u16(invalid_u16_data,
                             sizeof(invalid_u16_data) / sizeof(char16_t));

  auto res = wutils::u8s(invalid_u16, wutils::ErrorPolicy::SkipInvalidValues);
  EXPECT_FALSE(res.is_valid);
  EXPECT_EQ(res.value, u8"start__middle__end");
}

TEST(ErrorHandling, InvalidUTF16StopOnFirstError) {
  const char16_t invalid_u16_data[] = {u's',   u't', u'a', u'r', u't', u'_',
                                       0xD800, // Unpaired high surrogate
                                       u'_',   u'm', u'i', u'd', u'd', u'l',
                                       u'e',   u'_',
                                       0xDFFF, // Unpaired low surrogate
                                       u'_',   u'e', 'n',  'd'};
  std::u16string invalid_u16(invalid_u16_data,
                             sizeof(invalid_u16_data) / sizeof(char16_t));

  auto res = wutils::u8s(invalid_u16, wutils::ErrorPolicy::StopOnFirstError);
  EXPECT_FALSE(res.is_valid);
  EXPECT_EQ(res.value, u8"start_");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
