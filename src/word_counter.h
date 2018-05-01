#ifndef __WORD_COUNTER_H__
#define __WORD_COUNTER_H__

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <map>
#include <memory>
#include <random>
#include <string>

#include "namebo.pb.h"
#include "string_view.h"

class Segment;

class WordCounter {
 public:
  WordCounter(const std::string &path);
  ~WordCounter() { Flush(); }

  void Flush();
  void Add(const Segment &word, string_view prev1, string_view prev2);

  // Randomly picks a next word based on the probabilities in the table.
  Segment GetNext(string_view prev1, string_view prev2, double unigram_weight,
                  double bigram_weight, double trigram_weight);

  int32_t GetTotalCount() { return global_.total_count(); }
  int32_t GetSingletonCount() { return global_.singleton_count(); }
  int32_t GetUniqueCount() { return global_.unique_count(); }

  int32_t GetCount(string_view word);
  int32_t GetCount(string_view word, string_view prev1);
  int32_t GetCount(string_view word, string_view prev1, string_view prev2);
  int32_t GetPrefixCount(string_view word);
  int32_t GetPrefixCount(string_view word, string_view prev1);

  // Counts the number of terms that occur exactly once.
  void CountSingletons();

  void PrintStats();

 private:
  PhraseData GetPhraseData(leveldb::DB *db, string_view phrase);
  void SetPhraseData(leveldb::DB *db, string_view phrase,
                     const PhraseData &data);

  double Random();
  const std::string &PickUsage(const PhraseData &unigram_data);

 private:
  std::string global_path_;
  GlobalData global_;
  int32_t global_unsynced_;

  std::unique_ptr<leveldb::DB> unigrams_;
  std::unique_ptr<leveldb::DB> bigrams_;
  std::unique_ptr<leveldb::DB> trigrams_;

  std::random_device rd_;
  std::mt19937 gen_;
  std::uniform_real_distribution<> dist_;
};

#endif  // __WORD_COUNTER_H__
