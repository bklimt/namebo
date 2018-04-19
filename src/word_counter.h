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
  WordCounter(const std::string &path);
  ~WordCounter() { Flush(); }

  void Flush();
  void Add(string_view word, bool space_before);
  void Add(string_view word, string_view prev1, bool space_before);
  void Add(string_view word, string_view prev1, string_view prev2,
           bool space_before);

  // Randomly picks a next word based on the probabilities in the table.
  std::string GetNext(string_view prev1, string_view prev2,
                      double unigram_weight, double bigram_weight,
                      double trigram_weight, bool *space_before);

  int32_t GetTotalCount() { return global_.total_count(); }
  int32_t GetSingletonCount() { return global_.singleton_count(); }

  int32_t GetCount(string_view word);
  int32_t GetCount(string_view word, string_view prev1);
  int32_t GetCount(string_view word, string_view prev1, string_view prev2);

 private:
  PhraseData GetPhraseData(leveldb::DB *db, string_view phrase);
  void SetPhraseData(leveldb::DB *db, string_view phrase,
                     const PhraseData &data);

 private:
  std::string global_path_;
  GlobalData global_;
  int32_t global_unsynced_;

  std::unique_ptr<leveldb::DB> unigrams_;
  std::unique_ptr<leveldb::DB> bigrams_;
  std::unique_ptr<leveldb::DB> trigrams_;
};

bool Less(leveldb::Slice s1, leveldb::Slice s2);
bool Equal(leveldb::Slice s1, leveldb::Slice s2);
bool HasPrefix(leveldb::Slice s, leveldb::Slice pre);

#endif  // __WORD_COUNTER_H__
