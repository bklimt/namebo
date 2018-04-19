
#include <gtest/gtest.h>

#include "word_counter.h"

struct WordCounterTest : testing::Test {};

TEST_F(WordCounterTest, BasicTest) {
  srand(time(NULL));
  {
    WordCounter wc("data/test");

    // ^ foo bar baz baz foo bar qux $
    wc.Add("^", false);
    wc.Add("^", "^", false);
    wc.Add("foo", "^", "^", false);
    wc.Add("bar", "foo", "^", false);
    wc.Add("baz", "bar", "foo", false);
    wc.Add("baz", "baz", "bar", false);
    wc.Add("foo", "baz", "baz", false);
    wc.Add("bar", "foo", "baz", false);
    wc.Add("qux", "bar", "foo", false);
    wc.Add("$", "qux", "bar", false);

    // Test the total counts.
    EXPECT_EQ(8, wc.GetTotalCountWithoutCaret());
    EXPECT_EQ(10, wc.GetTotalCount());
    EXPECT_EQ(2, wc.GetSingletonCount());

    // Test the unigram counts.
    EXPECT_EQ(2, wc.GetCount("^"));
    EXPECT_EQ(2, wc.GetCount("foo"));
    EXPECT_EQ(2, wc.GetCount("bar"));
    EXPECT_EQ(2, wc.GetCount("baz"));
    EXPECT_EQ(1, wc.GetCount("qux"));
    EXPECT_EQ(1, wc.GetCount("$"));

    // Test the bigram counts.
    EXPECT_EQ(1, wc.GetCount("foo", "^"));
    EXPECT_EQ(2, wc.GetCount("bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("baz", "bar"));
    EXPECT_EQ(1, wc.GetCount("baz", "baz"));
    EXPECT_EQ(1, wc.GetCount("foo", "baz"));
    EXPECT_EQ(1, wc.GetCount("qux", "bar"));
    EXPECT_EQ(1, wc.GetCount("$", "qux"));

    // Test the trigram counts.
    EXPECT_EQ(1, wc.GetCount("foo", "^", "^"));
    EXPECT_EQ(1, wc.GetCount("bar", "foo", "^"));
    EXPECT_EQ(1, wc.GetCount("baz", "bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("baz", "baz", "bar"));
    EXPECT_EQ(1, wc.GetCount("foo", "baz", "baz"));
    EXPECT_EQ(1, wc.GetCount("bar", "foo", "baz"));
    EXPECT_EQ(1, wc.GetCount("qux", "bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("$", "qux", "bar"));
  }
  {
    WordCounter wc("data/test");

    // Test the total counts.
    EXPECT_EQ(8, wc.GetTotalCountWithoutCaret());
    EXPECT_EQ(10, wc.GetTotalCount());
    EXPECT_EQ(2, wc.GetSingletonCount());

    // Test the unigram counts.
    EXPECT_EQ(2, wc.GetCount("^"));
    EXPECT_EQ(2, wc.GetCount("foo"));
    EXPECT_EQ(2, wc.GetCount("bar"));
    EXPECT_EQ(2, wc.GetCount("baz"));
    EXPECT_EQ(1, wc.GetCount("qux"));
    EXPECT_EQ(1, wc.GetCount("$"));

    // Test the bigram counts.
    EXPECT_EQ(1, wc.GetCount("foo", "^"));
    EXPECT_EQ(2, wc.GetCount("bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("baz", "bar"));
    EXPECT_EQ(1, wc.GetCount("baz", "baz"));
    EXPECT_EQ(1, wc.GetCount("foo", "baz"));
    EXPECT_EQ(1, wc.GetCount("qux", "bar"));
    EXPECT_EQ(1, wc.GetCount("$", "qux"));

    // Test the trigram counts.
    EXPECT_EQ(1, wc.GetCount("foo", "^", "^"));
    EXPECT_EQ(1, wc.GetCount("bar", "foo", "^"));
    EXPECT_EQ(1, wc.GetCount("baz", "bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("baz", "baz", "bar"));
    EXPECT_EQ(1, wc.GetCount("foo", "baz", "baz"));
    EXPECT_EQ(1, wc.GetCount("bar", "foo", "baz"));
    EXPECT_EQ(1, wc.GetCount("qux", "bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("$", "qux", "bar"));
  }
}

TEST_F(WordCounterTest, LessTest) {
  EXPECT_TRUE(Less("abc", "def"));
  EXPECT_FALSE(Less("abc", "ab"));
  EXPECT_TRUE(Less("ab", "abc"));
  EXPECT_TRUE(Less("", "abc"));
  EXPECT_FALSE(Less("", ""));
}

TEST_F(WordCounterTest, HasPrefixTest) {
  EXPECT_TRUE(HasPrefix("foobar", "foo"));
  EXPECT_TRUE(HasPrefix("foo", "foo"));
  EXPECT_FALSE(HasPrefix("fo", "foo"));
  EXPECT_FALSE(HasPrefix("foo", "foobar"));
}
