
#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <leveldb/db.h>

#include "namebo.pb.h"

DEFINE_string(db_file, "", "leveldb file to dump");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_db_file.empty()) {
    LOG(FATAL) << "--db_file is required";
  }

  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, FLAGS_db_file, &db);
  CHECK(status.ok()) << "Unable to open " << FLAGS_db_file << ".";

  leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    PrefixData prefix_data;
    CHECK(prefix_data.ParseFromString(it->value().ToString()));
    std::cout << it->key().ToString()
              << ": "
              << prefix_data.ShortDebugString()
              << std::endl;
  }
  CHECK(it->status().ok());
  delete it; 

  delete db;

  return 0;
}

