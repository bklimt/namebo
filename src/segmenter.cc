
#include "segmenter.h"

Segmenter::Segmenter(std::string &&text) : pos_(0), text_(text) {
  Canonicalize();
  SkipWhitespace();
}

void Segmenter::Canonicalize() {
  // Replace any unicode with ascii, if possible.
  int src = 0;
  int dest = 0;
  while (src < text_.size()) {
    if (src + 2 < text_.size()) {
      if (memcmp(text_.data() + src, "\U00002018", 3) == 0 ||
          memcmp(text_.data() + src, "\U00002019", 3) == 0) {
        text_[dest++] = '\'';
        src += 3;
      } else if (memcmp(text_.data() + src, "\U0000201C", 3) == 0 ||
                 memcmp(text_.data() + src, "\U0000201D", 3) == 0) {
        text_[dest++] = '\"';
        src += 3;
      } else {
        text_[dest++] = text_[src++];
      }
    } else {
      text_[dest++] = text_[src++];
    }
  }
  if (dest < text_.size()) {
    text_.resize(dest);
  }
}

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

std::string Normalize(string_view s) {
  std::string n;
  n.reserve(s.size());
  for (int i = 0; i < s.size(); ++i) {
    n += static_cast<char>(tolower(s[i]));
  }
  return n;
}

Segment Segmenter::Next() {
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
      if (isalnum(text_[pos_]) || text_[pos_] == '\'') {
        pos_++;
      } else {
        break;
      }
    }
  }
  int end = pos_;
  Segment segment;
  segment.token = string_view(text_.data() + start, end - start);
  segment.normalized_token = Normalize(segment.token);
  segment.space_before = had_space_;
  // Skip to the next valid character.
  SkipWhitespace();
  return segment;
}
