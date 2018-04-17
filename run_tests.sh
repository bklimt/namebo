set -e
python3 ./build.py >Makefile
make
bin/segmenter_test
bin/word_counter_test
