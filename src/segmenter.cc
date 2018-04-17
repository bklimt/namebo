
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
  int start = pos_;
  // If it's a symbol, just read the symbol.
  if (pos_ < text_.size() && !isalnum(text_[pos_])) {
    if ((text_[pos_] & 0x80) == 0) {
      // It's an ascii character.
      pos_++;
    } else {
      // It's a UTF-8 rune.
      while ((text_[pos_] & 0x80) != 0) {
        pos_++;
      }
    }
  } else {
    // It's a word made of ascii alphanumeric characters.
    while (pos_ < text_.size()) {
      if (!isalnum(text_[pos_])) {
        break;
      }
      pos_++;
    }
  }
  int end = pos_;
  // Skip to the next valid character.
  SkipWhitespace();
  return string_view(text_.data() + start, end - start);
}
