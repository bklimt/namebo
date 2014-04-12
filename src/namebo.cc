
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <leveldb/db.h>

using namespace std;

DEFINE_string(input_file, "", "file of words to read in");
DEFINE_string(output_file, "", "leveldb to write map to");
DEFINE_int32(prefix_length, 3, "number of letters to consider");

void UpdateWord(leveldb::DB *db, const string &prefix, char letter) {
  printf("%s -> %c\n", prefix.c_str(), letter);

  //std::string value;
  //leveldb::Status s = db->Get(leveldb::ReadOptions(), prefix, &value);
  //if (s.ok()) {
  //  
  //}
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
  assert(status.ok());

  ifstream in(FLAGS_input_file);
  string line;
  getline(in, line);
  while (in) {
    string word = "^" + line + "$";
    for (int i = 1; i < word.size(); ++i) {
      char letter = word[i];
      for (int j = 1; j <= FLAGS_prefix_length; ++j) {
        if (i - j >= 0) {
          string prefix = word.substr(i - j, j);
          UpdateWord(db, prefix, letter);
        }
      }
    }
    getline(in, line);
  }

  delete db;

  return 0;
}

