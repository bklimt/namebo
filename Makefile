all: bin/convert bin/count bin/count_words bin/dump bin/generate bin/generate_words bin/segmenter_test bin/word_counter_test

clean:
	rm -rf bin || true
	rm -rf obj || true
	rm -rf gen || true

format:
	clang-format-3.8 -i src/*

.PRECIOUS: obj/%.o

bin/convert: obj/convert.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/convert.o: ./src/convert.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/convert.cc

bin/count: obj/count.o obj/segmenter.o obj/string_view.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lprotobuf -lgflags -lglog -lleveldb

obj/count.o: ./src/count.cc ./src/segmenter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/count.cc

bin/count_words: obj/count_words.o obj/segmenter.o obj/string_view.o obj/word_counter.o obj/string_view.o obj/segmenter.o obj/string_view.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lleveldb -lleveldb -lprotobuf -lleveldb -lleveldb -lglog -lleveldb -lgflags -lglog

obj/count_words.o: ./src/count_words.cc ./src/segmenter.h ./src/string_view.h ./src/word_counter.h ./src/string_view.h ./src/segmenter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/count_words.cc

bin/dump: obj/dump.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/dump.o: ./src/dump.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/dump.cc

bin/generate: obj/generate.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/generate.o: ./src/generate.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/generate.cc

bin/generate_words: obj/generate_words.o obj/segmenter.o obj/string_view.o obj/word_counter.o obj/string_view.o obj/segmenter.o obj/string_view.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lleveldb -lleveldb -lprotobuf -lleveldb -lleveldb -lglog -lleveldb -lgflags -lglog

obj/generate_words.o: ./src/generate_words.cc ./src/segmenter.h ./src/string_view.h ./src/word_counter.h ./src/string_view.h ./src/segmenter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/generate_words.cc

obj/namebo.o: gen/namebo.pb.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c gen/namebo.pb.cc

gen/namebo.pb.cc gen/namebo.pb.h: src/namebo.proto
	mkdir -p gen && protoc --proto_path=src --cpp_out=gen $^

obj/segmenter.o: ./src/segmenter.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/segmenter.cc

bin/segmenter_test: obj/segmenter_test.o obj/segmenter.o obj/string_view.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lgtest -lpthread -lgtest_main

obj/segmenter_test.o: ./src/segmenter_test.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/segmenter_test.cc

obj/string_view.o: ./src/string_view.cc ./src/string_view.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/string_view.cc

obj/word_counter.o: ./src/word_counter.cc ./src/word_counter.h ./src/string_view.h ./src/segmenter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/word_counter.cc

bin/word_counter_test: obj/word_counter_test.o obj/segmenter.o obj/string_view.o obj/word_counter.o obj/string_view.o obj/segmenter.o obj/string_view.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lleveldb -lleveldb -lprotobuf -lleveldb -lleveldb -lglog -lleveldb -lgtest -lpthread -lgtest_main

obj/word_counter_test.o: ./src/word_counter_test.cc ./src/segmenter.h ./src/string_view.h ./src/word_counter.h ./src/string_view.h ./src/segmenter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/word_counter_test.cc

