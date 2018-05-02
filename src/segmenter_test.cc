
#include <gtest/gtest.h>

#include "segmenter.h"

struct SegmenterTest : testing::Test {};

void TestCase(const char *input, Segment expected[], int n) {
  Segmenter seg(input, false);
}

void TestCase(const char *input, bool break_words, Segment expected[], int n) {
  Segmenter seg(input, break_words);
  for (int i = 0; i < n; ++i) {
    EXPECT_TRUE(seg.Valid());
    Segment s = seg.Next();
    EXPECT_EQ(string_view(expected[i].token), s.token);
    EXPECT_EQ(expected[i].normalized_token, s.normalized_token);
    EXPECT_EQ(expected[i].space_before, s.space_before);
  }
  EXPECT_FALSE(seg.Valid());
}

TEST_F(SegmenterTest, BasicTest) {
  Segment expected[] = {
      {"foo", "foo", true},
      {"bar", "bar", true},
      {"\U0001F4A9", "\U0001F4A9", true},
      {"\U0001F4A9", "\U0001F4A9", true},
      {"\U0001F4A9", "\U0001F4A9", false},
      {"baz", "baz", false},
      {".", ".", false},
      {"qux", "qux", false},
      {"?", "?", false},
  };
  TestCase("  foo bar \xF0\x9F\x92\xA9 \U0001F4A9\U0001F4A9baz.qux?   ",
           expected, 9);
}

TEST_F(SegmenterTest, BreakWordsTest) {
  Segment expected[] = {
      {"f", "f", true},
      {"o", "o", false},
      {"o", "o", false},
      {"b", "b", true},
      {"a", "a", false},
      {"r", "r", false},
      {"\U0001F4A9", "\U0001F4A9", true},
      {"\U0001F4A9", "\U0001F4A9", true},
      {"\U0001F4A9", "\U0001F4A9", false},
      {"b", "b", false},
      {"a", "a", false},
      {"z", "z", false},
      {".", ".", false},
      {"q", "q", false},
      {"u", "u", false},
      {"x", "x", false},
      {"?", "?", false},
  };
  TestCase("  foo bar \xF0\x9F\x92\xA9 \U0001F4A9\U0001F4A9baz.qux?   ", true,
           expected, 17);
}

TEST_F(SegmenterTest, AlphaTest) {
  Segment expected[] = {
      {"Foo", "foo", false},
      {"BaR", "bar", true},
  };
  TestCase("Foo BaR", expected, 2);
}

TEST_F(SegmenterTest, UnicodeTest) {
  Segment expected[] = {
      {"\U0001F4A9", "\U0001F4A9", false},
      {"\U0001F4A9", "\U0001F4A9", false},
      {"\U0001F4A9", "\U0001F4A9", false},
  };
  TestCase("\xF0\x9F\x92\xA9\U0001F4A9\U0001F4A9", expected, 3);
}

TEST_F(SegmenterTest, QuoteTest) {
  Segment expected[]{
      {"\"", "\"", false},
      {"'", "'", false},
      {"'", "'", false},
      {"\"", "\"", false},
  };
  TestCase("\U0000201C\U00002018\U00002019\U0000201D", expected, 4);
}

TEST_F(SegmenterTest, ApostropheTest) {
  Segment expected[] = {{"can't", "can't", false}, {"stop", "stop", true},
                        {".", ".", false},         {"won't", "won't", true},
                        {"stop", "stop", true},    {".", ".", false}};
  TestCase("can't stop. won\U00002019t stop.", expected, 6);
}
