
#include "segmenter.h"

const char canonical[] = "\"\'";

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

string_view Canonicalize(string_view c) {
  if (c == "\U0000201C" || c == "\U0000201D") {
    return string_view(canonical, 1);
  } else if (c == "\U00002018" || c == "\U00002019") {
    return string_view(canonical + 1, 1);
  }
  return c;
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
      if ((text_[pos_] & 0xC0) != 0xC0) {
        // It's a bogus character.
        text_[pos_] = ' ';
        pos_++;
      } else {
        pos_++;
        // Read the rest of the rune.
        while (pos_ < text_.size() && (text_[pos_] & 0xC0) == 0x80) {
          pos_++;
        }
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
  segment.token = Canonicalize(string_view(text_.data() + start, end - start));
  segment.space_before = had_space_;
  // Skip to the next valid character.
  SkipWhitespace();
  return segment;
}
