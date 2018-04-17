#ifndef __WORD_COUNTER_H__
#define __WORD_COUNTER_H__

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <map>
#include <memory>
#include <string>

#include "namebo.pb.h"
#include "string_view.h"

class WordCounter {
 public:
  WordCounter(string_view path);
  ~WordCounter() { Flush(); }

  void Flush();
  void Add(string_view word, string_view prev1, string_view prev2);

 private:
  PhraseData GetPhraseData(string_view phrase);
  void SetPhraseData(string_view phrase, const PhraseData& data);

 private:
  int32_t unsynced_;

  GlobalData global_;
  std::string global_path_;

  std::string phrase_path_;
  std::unique_ptr<leveldb::DB> phrases_;
  std::unique_ptr<leveldb::WriteBatch> phrase_batch_;
  std::map<std::string, PhraseData> phrase_cache_;
};

#endif  // __WORD_COUNTER_H__
