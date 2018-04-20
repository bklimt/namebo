
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

int32_t WordCounter::GetPrefixCount(string_view word) {
  PhraseData data = GetPhraseData(unigrams_.get(), KEY(word));
  return data.prefix_count();
}

int32_t WordCounter::GetPrefixCount(string_view word, string_view prev1) {
  PhraseData data = GetPhraseData(bigrams_.get(), KEY(prev1, word));
  return data.prefix_count();
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

#define INCREMENT(table, key, field)                      \
  do {                                                    \
    PhraseData data = GetPhraseData(table##_.get(), key); \
    data.set_##field(data.field() + 1);                   \
    SetPhraseData(table##_.get(), key, data);             \
  } while (0)

void WordCounter::Add(string_view word, string_view prev1, string_view prev2,
                      bool space_before) {
  INCREMENT(trigrams, KEY(prev2, prev1, word), count);
  INCREMENT(bigrams, KEY(prev2, prev1), prefix_count);

  INCREMENT(bigrams, KEY(prev1, word), count);
  INCREMENT(unigrams, KEY(prev1), prefix_count);

  // Update unigram entry for word.
  std::string key = KEY(word).ToString();
  INCREMENT(unigrams, key, count);
  if (!space_before) {
    INCREMENT(unigrams, key, no_space_count);
  }

  // Update the global data.
  global_.set_total_count(global_.total_count() + 1);
  // TODO(klimt): Count up the singletons in a final pass.
  global_unsynced_++;
  if (global_unsynced_ > 1000) {
    Flush();
  }
}

std::string WordCounter::GetNext(string_view prev1, string_view prev2,
                                 double unigram_weight, double bigram_weight,
                                 double trigram_weight, bool *space_before) {
  // Get the global data.
  int32_t total_count = global_.total_count();
  int32_t singleton_count = global_.singleton_count();
  double singleton_p = static_cast<double>(singleton_count) / total_count;
  LOG(INFO) << "singleton p = " << singleton_p;

  // TODO(klimt): This is all wrong, because the probabilities for the
  // suffixes of a novel ngram will add up to 0, not 1.
  double n = static_cast<double>(random()) / RAND_MAX;
  LOG(INFO) << "n = " << n;

  // Get unigram entry for "prev1".
  PhraseData bigram_prefix_data = GetPhraseData(unigrams_.get(), KEY(prev1));

  // Get the bigram entry for "prev2 prev1".
  PhraseData trigram_prefix_data =
      GetPhraseData(bigrams_.get(), KEY(prev2, prev1));

  // If the trigram doesn't exist, fall back to unigram and bigram data.
  if (trigram_prefix_data.count() == 0) {
    unigram_weight += trigram_weight / 2.0;
    bigram_weight += trigram_weight / 2.0;
    trigram_weight = 0.0;
  }
  // If the bigram doesn't exist, fall back to unigram data.
  if (bigram_prefix_data.count() == 0) {
    unigram_weight += bigram_weight;
    bigram_weight = 0.0;
  }

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
    double bigram_p = (bigram_prefix_data.count() == 0)
                          ? 0.0
                          : static_cast<double>(bigram_data.count()) /
                                bigram_prefix_data.prefix_count();
    double trigram_p = (trigram_prefix_data.count() == 0)
                           ? 0.0
                           : static_cast<double>(trigram_data.count()) /
                                 trigram_prefix_data.prefix_count();
    double p = (unigram_weight * unigram_p + bigram_weight * bigram_p +
                trigram_weight * trigram_p) /
               (unigram_weight + bigram_weight + trigram_weight);

    n -= p;
    if (n < 0) {
      std::string word = it->key().ToString();
      // TODO(klimt): Make this random?
      *space_before =
          (unigram_data.no_space_count() <= unigram_data.count() / 2);

      LOG(INFO) << "word: " << word;
      LOG(INFO) << "bigram prefix: " << KEY(prev1);
      LOG(INFO) << "trigram prefix: " << KEY(prev2, prev1);
      LOG(INFO) << "unigram prob: " << unigram_data.count() << " / "
                << total_count;
      LOG(INFO) << " bigram prob: " << bigram_data.count() << " / "
                << bigram_prefix_data.count();
      LOG(INFO) << "trigram prob: " << trigram_data.count() << " / "
                << trigram_prefix_data.count();
      LOG(INFO) << "p = " << p;

      return word;
    }
  }
  CHECK(it->status().ok()) << "Unable to iterate over unigrams.";
  LOG(FATAL) << "Reached end of unigram list with remaining n = " << n;
}