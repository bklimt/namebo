
#include <gtest/gtest.h>

#include "word_counter.h"

struct WordCounterTest : testing::Test {};

TEST_F(WordCounterTest, BasicTest) {
  WordCounter wc("data/test");

  wc.Add("foo", "^", "^");
  wc.Add("bar", "foo", "^");
  wc.Add("baz", "foo", "bar");

  // TODO(klimt): Test the actual counts.

  wc.Flush();

  // TODO(klimt): Test the actual counts again.
}
