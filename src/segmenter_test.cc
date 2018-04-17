
#include <gtest/gtest.h>

#include "segmenter.h"

struct SegmenterTest : testing::Test {};

TEST_F(SegmenterTest, BasicTest) {
  Segmenter seg("  foo bar  \xF0\x9F\x92\xA9 baz.qux?   ");
  EXPECT_FALSE(seg.done());
  EXPECT_EQ(string_view("foo"), seg.next());
  EXPECT_FALSE(seg.done());
  EXPECT_EQ(string_view("bar"), seg.next());
  EXPECT_FALSE(seg.done());
  EXPECT_EQ(string_view("\xF0\x9F\x92\xA9"), seg.next());
  EXPECT_FALSE(seg.done());
  EXPECT_EQ(string_view("baz"), seg.next());
  EXPECT_FALSE(seg.done());
  EXPECT_EQ(string_view("."), seg.next());
  EXPECT_FALSE(seg.done());
  EXPECT_EQ(string_view("qux"), seg.next());
  EXPECT_FALSE(seg.done());
  EXPECT_EQ(string_view("?"), seg.next());
  EXPECT_TRUE(seg.done());
}

// This is used by build.py to know to link with gunit_main.
// GUNIT_MAIN