
#include "segmenter.h"

void Segmenter::SkipWhitespace() {
  had_space_ = false;
  while (pos_ < text_.size()) {
    if (text_[pos_] != ' ') {
      break;
    }
    pos_++;
    had_space_ = true;
  }
}

Segment Segmenter::Next() {
  // TODO(klimt): Make make this handle contractions better?
  // TODO(klimt): Segment unicode sequences better.
  int start = pos_;
  if (pos_ < text_.size() && !isalnum(text_[pos_])) {
    // It's not alphanumeric.
    if ((text_[pos_] & 0x80) == 0) {
      // It's an ascii character.
      pos_++;
    } else {
      // It's a series of UTF-8 runes.
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
  Segment segment;
  segment.token = string_view(text_.data() + start, end - start);
  segment.space_before = had_space_;
  // Skip to the next valid character.
  SkipWhitespace();
  return segment;
}
