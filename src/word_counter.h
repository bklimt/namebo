#ifndef __WORD_COUNTER_H__
#define __WORD_COUNTER_H__

#include <map>
#include <string>

#include "namebo.pb.h"
#include "string_view.h"

class WordCounter {
 public:
  void Add(string_view word, string_view prev1, string_view prev2);

 private:
  GlobalData global_;
  std::map<std::string, PhraseData> phrase_;
};

#endif  // __WORD_COUNTER_H__
