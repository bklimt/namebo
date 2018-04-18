
#include "word_counter.h"

#include <glog/logging.h>
#include <leveldb/write_batch.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <streambuf>

#define KEY1(w1) (w1)
#define KEY2(w1, w2) ((w1).ToString() + " " + (w2).ToString())
#define KEY3(w1, w2, w3) \
  ((w1).ToString() + " " + (w2).ToString() + " " + (w3).ToString())
#define GET_MACRO3(_1, _2, _3, NAME, ...) NAME
#define KEY(...) GET_MACRO3(__VA_ARGS__, KEY3, KEY2, KEY1)(__VA_ARGS__)

leveldb::DB *OpenDBOrDie(const std::string &path) {
  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db);
  CHECK(status.ok()) << "Unable to open " << path << ".";
  return db;
}

WordCounter::WordCounter(const std::string &path) : global_unsynced_(0) {
  global_path_ = path + "/global";

  // Read the global data.
  std::ifstream global_stream(global_path_.c_str());
  if (global_stream) {
    std::string str((std::istreambuf_iterator<char>(global_stream)),
                    std::istreambuf_iterator<char>());
    CHECK(global_.ParseFromString(str)) << "Unable to parse " << global_path_
                                        << ".";
  } else {
    std::ofstream out(global_path_.c_str());
    CHECK(out) << "Unable to open " << global_path_ << ".";
    out << global_.SerializeAsString();
    out.close();
  }

  // Load the phrase data.
  unigrams_.reset(OpenDBOrDie(path + "/unigrams"));
  bigrams_.reset(OpenDBOrDie(path + "/bigrams"));
  trigrams_.reset(OpenDBOrDie(path + "/trigrams"));
}

void WordCounter::Flush() {
  if (global_unsynced_ == 0) {
    return;
  }
  {
    std::ofstream out(global_path_.c_str());
    CHECK(out) << "Unable to open " << global_path_ << ".";
    out << global_.SerializeAsString();
    out.close();
  }
  global_unsynced_ = 0;
}

int32_t WordCounter::GetCount(string_view word) {
  PhraseData data = GetPhraseData(unigrams_.get(), KEY(word));
  return data.count();
}

int32_t WordCounter::GetCount(string_view word, string_view prev1) {
  PhraseData data = GetPhraseData(bigrams_.get(), KEY(prev1, word));
  return data.count();
}

int32_t WordCounter::GetCount(string_view word, string_view prev1,
                              string_view prev2) {
  PhraseData data = GetPhraseData(trigrams_.get(), KEY(prev2, prev1, word));
  return data.count();
}

PhraseData WordCounter::GetPhraseData(leveldb::DB *db, string_view phrase) {
  PhraseData data;
  std::string str;
  leveldb::Status status = db->Get(leveldb::ReadOptions(), phrase, &str);
  if (status.ok()) {
    CHECK(data.ParseFromString(str));
    return data;
  } else if (status.IsNotFound()) {
    return data;
  } else {
    LOG(FATAL) << "Unable to read " << phrase << " from leveldb.";
  }
}

void WordCounter::SetPhraseData(leveldb::DB *db, string_view phrase,
                                const PhraseData &data) {
  std::string str;
  CHECK(data.SerializeToString(&str));
  leveldb::Status status = db->Put(leveldb::WriteOptions(), phrase, str);
  CHECK(status.ok()) << "Unable to write " << phrase << " to leveldb.";
}

void WordCounter::Add(string_view word, string_view prev1, string_view prev2) {
  // Update unigram entry for word.
  std::string unigram = KEY(word).ToString();
  PhraseData unigram_data = GetPhraseData(unigrams_.get(), unigram);
  unigram_data.set_count(unigram_data.count() + 1);
  SetPhraseData(unigrams_.get(), unigram, unigram_data);

  // Update the bigram entry for "prev1 word".
  std::string bigram = KEY(prev1, word);
  PhraseData bigram_data = GetPhraseData(bigrams_.get(), bigram);
  bigram_data.set_count(bigram_data.count() + 1);
  SetPhraseData(bigrams_.get(), bigram, bigram_data);

  // Update the trigram entry for "prev2 prev1 word".
  std::string trigram = KEY(prev2, prev1, word);
  PhraseData trigram_data = GetPhraseData(trigrams_.get(), trigram);
  trigram_data.set_count(trigram_data.count() + 1);
  SetPhraseData(trigrams_.get(), trigram, trigram_data);

  // Update the global data.
  global_.set_total_count(global_.total_count() + 1);
  if (unigram_data.count() == 1) {
    global_.set_singleton_count(global_.singleton_count() + 1);
  } else if (unigram_data.count() == 2) {
    global_.set_singleton_count(global_.singleton_count() - 1);
  }

  global_unsynced_++;
  if (global_unsynced_ > 1000) {
    Flush();
  }
}

int Compare(leveldb::Slice s1, leveldb::Slice s2) {
  int min_len = std::min(s1.size(), s2.size());
  int cmp = memcmp(s1.data(), s2.data(), min_len);
  if (cmp) {
    return cmp;
  }
  if (s1.size() < s2.size()) {
    return -1;
  }
  if (s1.size() > s2.size()) {
    return 1;
  }
  return 0;
}

bool Less(leveldb::Slice s1, leveldb::Slice s2) { return Compare(s1, s2) < 0; }

bool Equal(leveldb::Slice s1, leveldb::Slice s2) {
  return Compare(s1, s2) == 0;
}

bool HasPrefix(leveldb::Slice s, leveldb::Slice pre) {
  if (s.size() < pre.size()) {
    return false;
  }
  return memcmp(s.data(), pre.data(), std::min(s.size(), pre.size())) == 0;
}

std::string WordCounter::GetNext(string_view prev1, string_view prev2,
                                 double unigram_weight, double bigram_weight,
                                 double trigram_weight) {
  // Get the global data.
  int32_t total_count = global_.total_count();
  int32_t singleton_count = global_.singleton_count();
  double singleton_p = static_cast<double>(singleton_count) / total_count;
  // LOG(INFO) << "singleton p = " << singleton_p;

  double n = static_cast<double>(random()) / RAND_MAX;
  // LOG(INFO) << "n = " << n;

  // Get unigram entry for "prev1".
  PhraseData bigram_prefix_data = GetPhraseData(unigrams_.get(), KEY(prev1));

  // Get the bigram entry for "prev2 prev1".
  PhraseData trigram_prefix_data =
      GetPhraseData(bigrams_.get(), KEY(prev2, prev1));

  // Iterate over all terms.
  std::unique_ptr<leveldb::Iterator> it(
      unigrams_->NewIterator(leveldb::ReadOptions()));
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    // TODO(klimt): Is it possible to parse protos without copying?
    PhraseData unigram_data;
    leveldb::Slice bytes = it->value();
    CHECK(unigram_data.ParseFromString(bytes.ToString()))
        << "Unable to parse data for " << it->key();

    PhraseData bigram_data =
        GetPhraseData(bigrams_.get(), KEY(prev1, it->key()));
    PhraseData trigram_data =
        GetPhraseData(trigrams_.get(), KEY(prev2, prev1, it->key()));

    double unigram_p = static_cast<double>(unigram_data.count()) / total_count;
    double bigram_p =
        static_cast<double>(bigram_data.count()) / bigram_prefix_data.count();
    double trigram_p =
        static_cast<double>(trigram_data.count()) / trigram_prefix_data.count();
    double p = (unigram_weight * unigram_p + bigram_weight * bigram_p +
                trigram_weight * trigram_p) /
               (unigram_weight + bigram_weight + trigram_weight);

    /*
    LOG(INFO) << "word: " << it->key();
    LOG(INFO) << "unigram prob: " << unigram_data.count() << " / "
              << total_count;
    LOG(INFO) << " bigram prob: " << bigram_data.count() << " / "
              << bigram_prefix_data.count();
    LOG(INFO) << "trigram prob: " << trigram_data.count() << " / "
              << trigram_prefix_data.count();
    LOG(INFO) << "p = " << p;
    */
    n -= p;
    if (n < 0) {
      // LOG(INFO) << "CHOSE: " << it->key();
      return it->key().ToString();
    }
  }
  CHECK(it->status().ok()) << "Unable to iterate over unigrams.";
}