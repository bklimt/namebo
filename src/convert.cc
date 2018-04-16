/*
 * Creates a JSON representation of the weighted frequency map.
 */

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <leveldb/db.h>

#include "namebo.pb.h"

DEFINE_string(prefix_db, "", "leveldb with prefix data");

DEFINE_bool(module, false, "if true, export as a node module");

DEFINE_double(bigram_weight, 1.0, "weight considering 1 letter prefix");
DEFINE_double(trigram_weight, 100.0, "weight considering 2 letter prefix");
DEFINE_double(quadgram_weight, 10000.0, "weight considering 3 letter prefix");

void PrintSuffixJSON(const SuffixData &suffix_data) {
  std::cout << "\"" << suffix_data.letter() << "\":" << suffix_data.count();
}

void PrintPrefixJSON(const PrefixData &prefix_data) {
  std::cout << "\"" << prefix_data.prefix() << "\":{";
  for (int i = 0; i < prefix_data.suffix_size(); ++i) {
    if (i) {
      std::cout << ",";
    }
    PrintSuffixJSON(prefix_data.suffix(i));
  }
  std::cout << "}" << std::endl;
}

void GetPrefixData(leveldb::DB *db, const std::string &prefix,
                   PrefixData *prefix_data) {
  std::string value;
  leveldb::Status s = db->Get(leveldb::ReadOptions(), prefix, &value);
  if (s.ok()) {
    CHECK(prefix_data->ParseFromString(value));
  } else if (s.IsNotFound()) {
    prefix_data->Clear();
  } else {
    LOG(FATAL) << "Unable to read " << prefix << " from table.";
  }
}

void MergePrefixData(const PrefixData &in_data, double weight,
                     PrefixData *out_data) {
  for (int i = 0; i < in_data.suffix_size(); ++i) {
    std::string letter = in_data.suffix(i).letter();
    double count = in_data.suffix(i).count() * weight;

    bool found = false;
    for (int j = 0; j < out_data->suffix_size(); ++j) {
      SuffixData *suffix_data = out_data->mutable_suffix(j);
      if (suffix_data->letter() == letter) {
        suffix_data->set_count(suffix_data->count() + count);
        found = true;
      }
    }
    if (!found) {
      SuffixData *suffix_data = out_data->add_suffix();
      suffix_data->set_letter(letter);
      suffix_data->set_count(count);
    }
  }
}

void GetData(leveldb::DB *db, const std::string &word,
             PrefixData *prefix_data) {
  prefix_data->Clear();
  if (word.size() > 0) {
    std::string bigram = word.substr(word.size() - 1, 1);
    PrefixData bigram_data;
    GetPrefixData(db, bigram, &bigram_data);
    MergePrefixData(bigram_data, FLAGS_bigram_weight, prefix_data);
  }
  if (word.size() > 1) {
    std::string trigram = word.substr(word.size() - 2, 2);
    PrefixData trigram_data;
    GetPrefixData(db, trigram, &trigram_data);
    MergePrefixData(trigram_data, FLAGS_trigram_weight, prefix_data);
  }
  if (word.size() > 2) {
    std::string quadgram = word.substr(word.size() - 3, 3);
    PrefixData quadgram_data;
    GetPrefixData(db, quadgram, &quadgram_data);
    MergePrefixData(quadgram_data, FLAGS_quadgram_weight, prefix_data);
  }
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (FLAGS_prefix_db.empty()) {
    LOG(FATAL) << "--prefix_db is required";
  }

  srand(time(NULL));

  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, FLAGS_prefix_db, &db);
  CHECK(status.ok()) << "Unable to open " << FLAGS_prefix_db << ".";

  if (FLAGS_module) {
    std::cout << "module.exports = ";
  }

  std::cout << "{" << std::endl;

  bool first = true;
  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    if (!first) {
      std::cout << ",";
    }
    first = false;

    PrefixData prefix_data;
    GetData(db, it->key().ToString(), &prefix_data);
    prefix_data.set_prefix(it->key().ToString());

    PrintPrefixJSON(prefix_data);
  }
  CHECK(it->status().ok());
  delete it;

  std::cout << "}";

  if (FLAGS_module) {
    std::cout << ";";
  }

  std::cout << std::endl;

  delete db;
  return 0;
}
