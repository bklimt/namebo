
#include "word_counter.h"

#include <glog/logging.h>
#include <leveldb/write_batch.h>
#include <fstream>
#include <streambuf>

WordCounter::WordCounter(string_view path) : unsynced_(0) {
  global_path_ = path.ToString() + "/global";
  phrase_path_ = path.ToString() + "/phrase";

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
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, phrase_path_, &db);
  CHECK(status.ok()) << "Unable to open " << phrase_path_ << ".";
  phrases_.reset(db);

  phrase_batch_.reset(new leveldb::WriteBatch());
}

void WordCounter::Flush() {
  if (unsynced_ == 0) {
    return;
  }
  {
    std::ofstream out(global_path_.c_str());
    CHECK(out) << "Unable to open " << global_path_ << ".";
    out << global_.SerializeAsString();
    out.close();
  }
  {
    phrase_batch_.reset(new leveldb::WriteBatch());
    leveldb::WriteOptions write_options;
    write_options.sync = true;
    leveldb::Status status =
        phrases_->Write(write_options, phrase_batch_.get());
    CHECK(status.ok()) << "Unable to write to " << phrase_path_ << ".";

    phrase_cache_.clear();
  }
  unsynced_ = 0;
}

PhraseData WordCounter::GetPhraseData(string_view phrase) {
  if (!phrase_cache_.empty()) {
    auto iter = phrase_cache_.find(phrase.ToString());
    if (iter != phrase_cache_.end()) {
      return iter->second;
    }
  }

  PhraseData data;
  std::string str;
  leveldb::Status s = phrases_->Get(leveldb::ReadOptions(), phrase, &str);
  if (s.ok()) {
    CHECK(data.ParseFromString(str));
    return data;
  } else if (s.IsNotFound()) {
    return data;
  } else {
    LOG(FATAL) << "Unable to read " << phrase << " from " << phrase_path_
               << ".";
  }
}

void WordCounter::SetPhraseData(string_view phrase, const PhraseData& data) {
  std::string str;
  CHECK(data.SerializeToString(&str));
  phrase_batch_->Put(phrase, str);
  phrase_cache_[str] = data;
}

void WordCounter::Add(string_view word, string_view prev1, string_view prev2) {
  // Update unigram entry for word.
  std::string unigram = word.ToString();
  PhraseData unigram_data = GetPhraseData(unigram);
  unigram_data.set_count(unigram_data.count() + 1);
  SetPhraseData(unigram, unigram_data);

  // Update the bigram entry for "prev1 word".
  std::string bigram = prev1.ToString() + " " + unigram;
  PhraseData bigram_data = GetPhraseData(bigram);
  bigram_data.set_count(bigram_data.count() + 1);
  SetPhraseData(bigram, bigram_data);

  // Update the trigram entry for "prev2 prev1 word".
  std::string trigram = prev2.ToString() + " " + bigram;
  PhraseData trigram_data = GetPhraseData(trigram);
  trigram_data.set_count(trigram_data.count() + 1);
  SetPhraseData(trigram, trigram_data);

  // Update the global data.
  global_.set_total_count(global_.total_count());
  if (unigram_data.count() == 1) {
    global_.set_singleton_count(global_.singleton_count() + 1);
  } else if (unigram_data.count() == 2) {
    global_.set_singleton_count(global_.singleton_count() - 1);
  }

  unsynced_++;
  if (unsynced_ > 1000) {
    Flush();
  }
}
