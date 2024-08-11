
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fstream>
#include <iostream>

#include "segmenter.h"
#include "word_counter.h"

DEFINE_string(input, "", "file of words to read in");
DEFINE_string(output, "", "leveldb to write map to");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (FLAGS_input.empty()) {
    LOG(FATAL) << "--input is required";
  }

  if (FLAGS_output.empty()) {
    LOG(FATAL) << "--output is required";
  }

  WordCounter wc(FLAGS_output);

  std::ifstream in(FLAGS_input.c_str());
  CHECK(in) << "Unable to open " << FLAGS_input << ".";

  std::string line;
  getline(in, line);
  while (in) {
    std::cout << line << std::endl;
    if (!line.empty()) {
      std::string prev1 = "^";
      std::string prev2 = "^";
      std::string text = " " + line + " $";
      Segmenter segmenter(std::move(text), false);
      while (segmenter.Valid()) {
        Segment segment = segmenter.Next();
        wc.Add(segment, prev1, prev2);
        prev2 = prev1;
        prev1 = segment.normalized_token;
      }
    }
    getline(in, line);
  }
  wc.CountSingletons();
  wc.PrintStats();

  return 0;
}
