
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <leveldb/db.h>

#include "namebo.pb.h"

DEFINE_string(input_file, "", "file of words to read in");
DEFINE_string(output_file, "", "leveldb to write map to");
DEFINE_int32(prefix_length, 3, "number of letters to consider");

void UpdateWord(leveldb::DB *db, const std::string &prefix, char letter) {
  // printf("%s -> %c\n", prefix.c_str(), letter);

  PrefixData prefix_data;
  std::string value;
  leveldb::Status s = db->Get(leveldb::ReadOptions(), prefix, &value);
  if (s.ok()) {
    CHECK(prefix_data.ParseFromString(value));
    bool found = false;
    for (int i = 0; i < prefix_data.suffix_size(); ++i) {
      if (prefix_data.suffix(i).letter()[0] == letter) {
        found = true;
        SuffixData *suffix_data = prefix_data.mutable_suffix(i);
        suffix_data->set_count(suffix_data->count() + 1.0);
      }
    }
    if (!found) {
      SuffixData *suffix_data = prefix_data.add_suffix();
      suffix_data->set_letter(std::string(&letter, 1));
      suffix_data->set_count(1.0);
    }
  } else if (s.IsNotFound()) {
    prefix_data.set_prefix(prefix);
    SuffixData *suffix_data = prefix_data.add_suffix();
    suffix_data->set_letter(std::string(&letter, 1));
    suffix_data->set_count(1.0);
  } else {
    LOG(FATAL) << "Unable to read " << prefix << " from table.";
  }

  CHECK(prefix_data.SerializeToString(&value));

  s = db->Put(leveldb::WriteOptions(), prefix, value);
  CHECK(s.ok());
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_input_file.empty()) {
    LOG(FATAL) << "--input_file is required";
  }

  if (FLAGS_output_file.empty()) {
    LOG(FATAL) << "--output_file is required";
  }

  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, FLAGS_output_file, &db);
  CHECK(status.ok()) << "Unable to open " << FLAGS_output_file << ".";

  std::ifstream in(FLAGS_input_file);
  std::string line;
  getline(in, line);
  while (in) {
    std::string word = "^" + line + "$";
    for (int i = 1; i < word.size(); ++i) {
      char letter = word[i];
      for (int j = 1; j <= FLAGS_prefix_length; ++j) {
        if (i - j >= 0) {
          std::string prefix = word.substr(i - j, j);
          UpdateWord(db, prefix, letter);
        }
      }
    }
    getline(in, line);
  }

  delete db;

  return 0;
}
