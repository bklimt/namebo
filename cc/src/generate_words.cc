
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fstream>
#include <iostream>

#include "segmenter.h"
#include "word_counter.h"

DEFINE_string(input, "", "leveldb to read words from");
DEFINE_double(unigram_weight, 0.1, "unigram weight");
DEFINE_double(bigram_weight, 0.3, "bigram weight");
DEFINE_double(trigram_weight, 0.6, "trigram weight");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (FLAGS_input.empty()) {
    LOG(FATAL) << "--input is required";
  }

  WordCounter wc(FLAGS_input);

  srand(time(NULL));
  int32_t word_count = 0;
  std::string prev1 = "^";
  std::string prev2 = "^";
  while (true) {
    Segment segment = wc.GetNext(prev1, prev2, FLAGS_unigram_weight,
                                 FLAGS_bigram_weight, FLAGS_trigram_weight);
    prev2 = prev1;
    prev1 = segment.normalized_token;
    ++word_count;

    if (segment.normalized_token == "$" || word_count > 200) {
      printf("\n");
      break;
    }

    printf("%s%s", segment.space_before ? " " : "", segment.token.data());
  }

  return 0;
}
