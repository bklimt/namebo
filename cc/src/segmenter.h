#ifndef __SEGMENTER_H__
#define __SEGMENTER_H__

#include <string>

#include "string_view.h"

struct Segment {
  string_view token;
  std::string normalized_token;
  bool space_before;

  // Some space to use for the backing store of token, if needed.
  std::string buffer;
};

class Segmenter {
 public:
  Segmenter(std::string &&text, bool break_words);

  bool Valid() { return pos_ < text_.size(); }

  Segment Next();

 private:
  void Canonicalize();
  void SkipWhitespace();

  int pos_;
  std::string text_;
  bool had_space_;
  bool break_words_;
};

#endif  // __SEGMENTER_H__
