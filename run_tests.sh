set -e
python3 ./build.py >Makefile
make format
make all
bin/segmenter_test
bin/word_counter_test
