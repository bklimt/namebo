#include "word_counter.h"

void WordCounter::Add(string_view word, string_view prev1, string_view prev2) {
  // Update unigram entry for word.
  // Update the bigram entry for "prev1 word".
  // Update the bigram suffix entry for "prev1".
  // Update the trigram entry for "prev2 prev1 word".
  // Update the trigram suffix entry for "prev2 prev1".
}
