
#include <gtest/gtest.h>

#include "segmenter.h"

struct SegmenterTest : testing::Test {};

TEST_F(SegmenterTest, BasicTest) {
  Segmenter seg("  foo bar \xF0\x9F\x92\xA9 \U0001F4A9\U0001F4A9baz.qux?   ");
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("foo"), seg.Next());
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("bar"), seg.Next());
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9"), seg.Next());
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("\U0001F4A9\U0001F4A9"), seg.Next());
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("baz"), seg.Next());
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("."), seg.Next());
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("qux"), seg.Next());
  EXPECT_TRUE(seg.Valid());
  EXPECT_EQ(string_view("?"), seg.Next());
  EXPECT_FALSE(seg.Valid());
}
