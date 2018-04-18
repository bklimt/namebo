#ifndef __SEGMENTER_H__
#define __SEGMENTER_H__

#include <string>
#include "string_view.h"

struct Segment {
  string_view token;
  bool space_before;
};

class Segmenter {
 public:
  Segmenter(std::string &&text) : pos_(0), text_(text) { SkipWhitespace(); }

  bool Valid() { return pos_ < text_.size(); }

  Segment Next();

 private:
  void SkipWhitespace();

  int pos_;
  std::string text_;
  bool had_space_;
};

#endif  // __SEGMENTER_H__
