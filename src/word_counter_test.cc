
#include <gtest/gtest.h>

#include "word_counter.h"

struct WordCounterTest : testing::Test {};

TEST_F(WordCounterTest, BasicTest) { WordCounter wc("data/test"); }
