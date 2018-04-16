
#include <string>
#include "string_view.h"

class Segmenter {
 public:
  Segmenter(std::string &&text) : pos_(0), text_(text) {}

  bool done() { return pos_ >= text_.size(); }

  string_view next();

 private:
  void SkipWhitespace();

  int pos_;
  std::string text_;
};