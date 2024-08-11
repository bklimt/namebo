
#include "word_counter.h"

#include <gtest/gtest.h>

#include "segmenter.h"

struct WordCounterTest : testing::Test {};

TEST_F(WordCounterTest, BasicTest) {
  srand(time(NULL));
  {
    WordCounter wc("data/test");

    // ^ foo bar baz baz foo bar qux $
    wc.Add({"Foo", "foo", false}, "^", "^");
    wc.Add({"BAR", "bar", false}, "foo", "^");
    wc.Add({"baz", "baz", false}, "bar", "foo");
    wc.Add({"baz", "baz", false}, "baz", "bar");
    wc.Add({"foo", "foo", false}, "baz", "baz");
    wc.Add({"bar", "bar", false}, "foo", "baz");
    wc.Add({"qux", "qux", false}, "bar", "foo");
    wc.Add({"$", "$", false}, "qux", "bar");

    // Test the total counts.
    EXPECT_EQ(8, wc.GetTotalCount());
    EXPECT_EQ(0, wc.GetSingletonCount());
    EXPECT_EQ(0, wc.GetUniqueCount());
    wc.CountSingletons();
    EXPECT_EQ(2, wc.GetSingletonCount());
    EXPECT_EQ(5, wc.GetUniqueCount());

    // Test the unigram counts.
    EXPECT_EQ(0, wc.GetCount("^"));
    EXPECT_EQ(2, wc.GetCount("foo"));
    EXPECT_EQ(2, wc.GetCount("bar"));
    EXPECT_EQ(2, wc.GetCount("baz"));
    EXPECT_EQ(1, wc.GetCount("qux"));
    EXPECT_EQ(1, wc.GetCount("$"));
    EXPECT_EQ(1, wc.GetPrefixCount("^"));
    EXPECT_EQ(2, wc.GetPrefixCount("foo"));
    EXPECT_EQ(2, wc.GetPrefixCount("bar"));
    EXPECT_EQ(2, wc.GetPrefixCount("baz"));
    EXPECT_EQ(1, wc.GetPrefixCount("qux"));
    EXPECT_EQ(0, wc.GetPrefixCount("$"));

    // Test the bigram counts.
    EXPECT_EQ(1, wc.GetCount("foo", "^"));
    EXPECT_EQ(2, wc.GetCount("bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("baz", "bar"));
    EXPECT_EQ(1, wc.GetCount("baz", "baz"));
    EXPECT_EQ(1, wc.GetCount("foo", "baz"));
    EXPECT_EQ(1, wc.GetCount("qux", "bar"));
    EXPECT_EQ(1, wc.GetCount("$", "qux"));
    EXPECT_EQ(1, wc.GetPrefixCount("^", "^"));
    EXPECT_EQ(1, wc.GetPrefixCount("foo", "^"));
    EXPECT_EQ(2, wc.GetPrefixCount("bar", "foo"));
    EXPECT_EQ(1, wc.GetPrefixCount("baz", "bar"));
    EXPECT_EQ(1, wc.GetPrefixCount("baz", "baz"));
    EXPECT_EQ(1, wc.GetPrefixCount("foo", "baz"));
    EXPECT_EQ(1, wc.GetPrefixCount("qux", "bar"));
    EXPECT_EQ(0, wc.GetPrefixCount("$", "qux"));

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
    EXPECT_EQ(8, wc.GetTotalCount());
    EXPECT_EQ(2, wc.GetSingletonCount());
    EXPECT_EQ(5, wc.GetUniqueCount());

    // Test the unigram counts.
    EXPECT_EQ(0, wc.GetCount("^"));
    EXPECT_EQ(2, wc.GetCount("foo"));
    EXPECT_EQ(2, wc.GetCount("bar"));
    EXPECT_EQ(2, wc.GetCount("baz"));
    EXPECT_EQ(1, wc.GetCount("qux"));
    EXPECT_EQ(1, wc.GetCount("$"));
    EXPECT_EQ(1, wc.GetPrefixCount("^"));
    EXPECT_EQ(2, wc.GetPrefixCount("foo"));
    EXPECT_EQ(2, wc.GetPrefixCount("bar"));
    EXPECT_EQ(2, wc.GetPrefixCount("baz"));
    EXPECT_EQ(1, wc.GetPrefixCount("qux"));
    EXPECT_EQ(0, wc.GetPrefixCount("$"));

    // Test the bigram counts.
    EXPECT_EQ(1, wc.GetCount("foo", "^"));
    EXPECT_EQ(2, wc.GetCount("bar", "foo"));
    EXPECT_EQ(1, wc.GetCount("baz", "bar"));
    EXPECT_EQ(1, wc.GetCount("baz", "baz"));
    EXPECT_EQ(1, wc.GetCount("foo", "baz"));
    EXPECT_EQ(1, wc.GetCount("qux", "bar"));
    EXPECT_EQ(1, wc.GetCount("$", "qux"));
    EXPECT_EQ(1, wc.GetPrefixCount("^", "^"));
    EXPECT_EQ(1, wc.GetPrefixCount("foo", "^"));
    EXPECT_EQ(2, wc.GetPrefixCount("bar", "foo"));
    EXPECT_EQ(1, wc.GetPrefixCount("baz", "bar"));
    EXPECT_EQ(1, wc.GetPrefixCount("baz", "baz"));
    EXPECT_EQ(1, wc.GetPrefixCount("foo", "baz"));
    EXPECT_EQ(1, wc.GetPrefixCount("qux", "bar"));
    EXPECT_EQ(0, wc.GetPrefixCount("$", "qux"));

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
