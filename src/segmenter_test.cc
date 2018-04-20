
#include <gtest/gtest.h>

#include "segmenter.h"

struct SegmenterTest : testing::Test {};

// TODO(klimt): Add some space tests.

TEST_F(SegmenterTest, BasicTest) {
  Segmenter seg("  foo bar \xF0\x9F\x92\xA9 \U0001F4A9\U0001F4A9baz.qux?   ");
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("foo"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("bar"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("baz"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("."), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("qux"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("?"), seg.Next().token);
  EXPECT_FALSE(seg.Valid());
}

TEST_F(SegmenterTest, UnicodeTest) {
  Segmenter seg("\xF0\x9F\x92\xA9\U0001F4A9\U0001F4A9");
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9"), seg.Next().token);
  EXPECT_FALSE(seg.Valid());
}

TEST_F(SegmenterTest, QuoteTest) {
  Segmenter seg("\U0000201C\U00002018\U00002019\U0000201D");
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\""), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\'"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\'"), seg.Next().token);
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\""), seg.Next().token);
  EXPECT_FALSE(seg.Valid());
}
