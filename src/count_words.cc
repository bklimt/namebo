
#include <fstream>
#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

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
      std::string text = " " + line + "$";
      Segmenter segmenter(std::move(text));
      while (segmenter.Valid()) {
        Segment segment = segmenter.Next();
        string_view word = segment.token;
        wc.Add(word, prev1, prev2, segment.space_before);
        prev2 = prev1;
        prev1 = word.ToString();
      }
    }
    getline(in, line);
  }

  return 0;
}
