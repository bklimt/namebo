
#include "segmenter.h"

void Segmenter::SkipWhitespace() {
  while (pos_ < text_.size()) {
    if (text_[pos_] != ' ') {
      break;
    }
    pos_++;
  }
}

string_view Segmenter::next() {
  SkipWhitespace();
  int start_ = pos_;
  // If it's a symbol, just read the symbol.
  if (pos_ < text_.size() && !isalnum(text_[pos_])) {
    pos_++;
    SkipWhitespace();
    return string_view(text_.data() + start_, 1);
  }
  while (pos_ < text_.size()) {
    if (!isalnum(text_[pos_])) {
      break;
    }
    pos_++;
  }
  int end_ = pos_;
  // Skip to the next valid character.
  SkipWhitespace();
  return string_view(text_.data() + start_, end_ - start_);
}

