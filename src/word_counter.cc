
#include "word_counter.h"

#include <glog/logging.h>
#include <leveldb/write_batch.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <streambuf>

#include "segmenter.h"

std::string ToString(const std::string& s) { return s; }

std::string ToString(const string_view& s) { return s.ToString(); }

template <typename T1>
std::string Key(const T1& s) {
  return ToString(s);
}

template <typename T1, typename T2>
std::string Key(const T1& s1, const T2& s2) {
  return ToString(s1) + " " + ToString(s2);
}

template <typename T1, typename T2, typename T3>
std::string Key(const T1& s1, const T2& s2, const T3& s3) {
  return ToString(s1) + " " + ToString(s2) + " " + ToString(s3);
}

leveldb::DB* OpenDBOrDie(const std::string& path) {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db);
  CHECK(status.ok()) << "Unable to open " << path << ".";
  return db;
}

double WordCounter::Random() { return dist_(gen_); }

WordCounter::WordCounter(const std::string& path)
    : global_unsynced_(0), rd_(), gen_(rd_()), dist_(0.0, 1.0) {
  global_path_ = path + "/global";

  // Read the global data.
  std::ifstream global_stream(global_path_.c_str());
  if (global_stream) {
    std::string str((std::istreambuf_iterator<char>(global_stream)),
                    std::istreambuf_iterator<char>());
    CHECK(global_.ParseFromString(str))
        << "Unable to parse " << global_path_ << ".";
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
  PhraseData data = GetPhraseData(unigrams_.get(), Key(word));
  return data.count();
}

int32_t WordCounter::GetCount(string_view word, string_view prev1) {
  PhraseData data = GetPhraseData(bigrams_.get(), Key(prev1, word));
  return data.count();
}

int32_t WordCounter::GetCount(string_view word, string_view prev1,
                              string_view prev2) {
  PhraseData data = GetPhraseData(trigrams_.get(), Key(prev2, prev1, word));
  return data.count();
}

int32_t WordCounter::GetPrefixCount(string_view word) {
  PhraseData data = GetPhraseData(unigrams_.get(), Key(word));
  return data.prefix_count();
}

int32_t WordCounter::GetPrefixCount(string_view word, string_view prev1) {
  PhraseData data = GetPhraseData(bigrams_.get(), Key(prev1, word));
  return data.prefix_count();
}

PhraseData WordCounter::GetPhraseData(leveldb::DB* db, string_view phrase) {
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

void WordCounter::SetPhraseData(leveldb::DB* db, string_view phrase,
                                const PhraseData& data) {
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

void WordCounter::Add(const Segment& word, string_view prev1,
                      string_view prev2) {
  INCREMENT(trigrams, Key(prev2, prev1, word.normalized_token), count);
  INCREMENT(bigrams, Key(prev2, prev1), prefix_count);

  INCREMENT(bigrams, Key(prev1, word.normalized_token), count);
  INCREMENT(unigrams, Key(prev1), prefix_count);

  /*
  if (Key(prev1) == "^") {
    LOG(INFO) << "Prefix for " << word.normalized_token << " is ^.";
    PhraseData to_test = GetPhraseData(unigrams_.get(), "^");
    LOG(INFO) << "Count: " << to_test.prefix_count();
  }
  */

  // Update unigram entry for word.
  std::string key = Key(word.normalized_token);
  PhraseData data = GetPhraseData(unigrams_.get(), key);
  data.set_count(data.count() + 1);
  if (!word.space_before) {
    data.set_no_space_count(data.no_space_count() + 1);
  }
  // Update the usages.
  bool found = false;
  for (int i = 0; i < data.usages_size(); ++i) {
    if (data.usages(i).text().size() != word.token.size()) {
      continue;
    }
    size_t size = word.token.size();
    if (memcmp(data.usages(i).text().c_str(), word.token.data(), size) == 0) {
      data.mutable_usages(i)->set_count(data.usages(i).count() + 1);
      found = true;
      break;
    }
  }
  if (!found) {
    Usage* usage = data.add_usages();
    usage->set_text(word.token.ToString());
    usage->set_count(1);
    // LOG(INFO) << "Adding usage: " << word.token.ToString();
  }
  SetPhraseData(unigrams_.get(), key, data);

  // Update the global data.
  global_.set_total_count(global_.total_count() + 1);
  // TODO(klimt): Count up the singletons in a final pass.
  global_unsynced_++;
  if (global_unsynced_ > 1000) {
    Flush();
  }
}

void WordCounter::CountSingletons() {
  // Iterate over all terms.
  int singletons = 0;
  int unique = 0;
  std::unique_ptr<leveldb::Iterator> it(
      unigrams_->NewIterator(leveldb::ReadOptions()));
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    // TODO(klimt): Is it possible to parse protos without copying?
    PhraseData unigram_data;
    leveldb::Slice bytes = it->value();
    CHECK(unigram_data.ParseFromString(bytes.ToString()))
        << "Unable to parse data for " << it->key();

    if (unigram_data.count() > 0) {
      ++unique;
    }
    if (unigram_data.count() == 1) {
      ++singletons;
    }
  }
  global_.set_singleton_count(singletons);
  global_.set_unique_count(unique);
  global_unsynced_++;
}

const std::string& WordCounter::PickUsage(const PhraseData& unigram_data) {
  int n = Random() * unigram_data.count();
  for (int i = 0; i < unigram_data.usages_size(); ++i) {
    n -= unigram_data.usages(i).count();
    if (n <= 0) {
      return unigram_data.usages(i).text();
    }
  }
  return unigram_data.usages(0).text();
}

Segment WordCounter::GetNext(string_view prev1, string_view prev2,
                             double unigram_weight, double bigram_weight,
                             double trigram_weight) {
  // Get the global data.
  int32_t total_count = global_.total_count();
  int32_t singleton_count = global_.singleton_count();
  double singleton_p = static_cast<double>(singleton_count) / total_count;
  LOG(INFO) << "singleton p = " << singleton_p;

  double n = Random();
  LOG(INFO) << "n = " << n;

  // Get unigram entry for "prev1".
  PhraseData bigram_prefix_data = GetPhraseData(unigrams_.get(), Key(prev1));

  // Get the bigram entry for "prev2 prev1".
  PhraseData trigram_prefix_data =
      GetPhraseData(bigrams_.get(), Key(prev2, prev1));

  // If the trigram doesn't exist, fall back to unigram and bigram data.
  if (trigram_prefix_data.prefix_count() == 0) {
    unigram_weight += trigram_weight / 2.0;
    bigram_weight += trigram_weight / 2.0;
    trigram_weight = 0.0;
  }
  // If the bigram doesn't exist, fall back to unigram data.
  if (bigram_prefix_data.prefix_count() == 0) {
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
        GetPhraseData(bigrams_.get(), Key(prev1, it->key()));
    PhraseData trigram_data =
        GetPhraseData(trigrams_.get(), Key(prev2, prev1, it->key()));

    double unigram_p = static_cast<double>(unigram_data.count()) / total_count;
    double bigram_p = (bigram_prefix_data.prefix_count() == 0)
                          ? 0.0
                          : static_cast<double>(bigram_data.count()) /
                                bigram_prefix_data.prefix_count();
    double trigram_p = (trigram_prefix_data.prefix_count() == 0)
                           ? 0.0
                           : static_cast<double>(trigram_data.count()) /
                                 trigram_prefix_data.prefix_count();
    double p = (unigram_weight * unigram_p + bigram_weight * bigram_p +
                trigram_weight * trigram_p) /
               (unigram_weight + bigram_weight + trigram_weight);

    if (isnan(p)) {
      LOG(ERROR) << "Found a NaN probability.";
      LOG(ERROR) << it->key();
      LOG(ERROR) << unigram_data.DebugString();
      LOG(ERROR) << bigram_data.DebugString();
      LOG(ERROR) << trigram_data.DebugString();
      LOG(FATAL) << "Cannot subtract NaN probability.";
    }

    n -= p;
    if (n < 0) {
      Segment segment;
      std::string word = it->key().ToString();
      segment.buffer = PickUsage(unigram_data);
      segment.token = segment.buffer;
      segment.normalized_token = word;
      // TODO(klimt): Make this random?
      segment.space_before =
          (unigram_data.no_space_count() <= unigram_data.count() / 2);

      LOG(INFO) << "word: " << word;
      LOG(INFO) << "bigram prefix: " << Key(prev1);
      LOG(INFO) << "trigram prefix: " << Key(prev2, prev1);
      LOG(INFO) << "unigram prob: " << unigram_data.count() << " / "
                << total_count;
      LOG(INFO) << " bigram prob: " << bigram_data.count() << " / "
                << bigram_prefix_data.prefix_count();
      LOG(INFO) << "trigram prob: " << trigram_data.count() << " / "
                << trigram_prefix_data.prefix_count();
      LOG(INFO) << "p = " << p;

      return segment;
    }
  }
  CHECK(it->status().ok()) << "Unable to iterate over unigrams.";
  LOG(FATAL) << "Reached end of unigram list with remaining n = " << n;
}

void WordCounter::PrintStats() {
  std::cout << " Statistics" << std::endl;
  std::cout << "============" << std::endl;
  std::cout << global_.DebugString() << std::endl;
  /*
  std::cout << "unigrams: " << std::endl;
  std::string stats;
  CHECK(unigrams_->GetProperty("leveldb.stats", &stats)) << "Unable to read
  stats.";
  std::cout << stats << std::endl;
  */
}
