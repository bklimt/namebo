
#include <string>

class Segmenter {
 public:
  Segmenter(std::string&& text): pos_(0), text_(text) {
  }

  bool done() {
    return pos_ >= text_.size();
  }

  std::string_view next() {
    int start_ = pos_;
    // Go to the next break character.
    while (pos_ < text_.size()) {
      if (text_[pos_] == ' ') {
        break;
      }
      pos_++;
    }
    // Skip to the next valid character.
    while (pos_ < text_.size()) {
      if (text_[pos_] != ' ') {
        break;
      }
      pos_++;
    }
    return std::string_view(text_.data() + start_, pos_ - start_);
  }

 private:
  int pos_;
  std::string text_;
};