all: bin/generate_words bin/convert bin/segmenter_test bin/dump bin/generate bin/word_counter_test bin/count bin/count_words

clean:
	rm -rf bin || true
	rm -rf obj || true
	rm -rf gen || true

format:
	clang-format-3.8 -i src/*

.PRECIOUS: obj/%.o

bin/generate_words: obj/generate_words.o obj/word_counter.o obj/string_view.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lprotobuf -lleveldb -lleveldb -lglog -lleveldb -lgflags -lglog

obj/namebo.o: gen/namebo.pb.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c gen/namebo.pb.cc

obj/generate_words.o: ./src/generate_words.cc ./src/word_counter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/generate_words.cc

bin/convert: obj/convert.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/convert.o: ./src/convert.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/convert.cc

bin/segmenter_test: obj/segmenter_test.o obj/segmenter.o obj/string_view.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lgtest -lpthread -lgtest_main

obj/generate.o: ./src/generate.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/generate.cc

bin/dump: obj/dump.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

bin/generate: obj/generate.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

bin/word_counter_test: obj/word_counter_test.o obj/word_counter.o obj/string_view.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lprotobuf -lleveldb -lleveldb -lglog -lleveldb -lgtest -lpthread -lgtest_main

obj/dump.o: ./src/dump.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/dump.cc

gen/namebo.pb.cc gen/namebo.pb.h: src/namebo.proto
	mkdir -p gen && protoc --proto_path=src --cpp_out=gen $^

bin/count: obj/count.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

bin/count_words: obj/count_words.o obj/segmenter.o obj/string_view.o obj/word_counter.o obj/string_view.o obj/namebo.o
	mkdir -p bin && g++ -g -o $@ $^ -lleveldb -lleveldb -lprotobuf -lleveldb -lleveldb -lglog -lleveldb -lgflags -lglog

obj/count_words.o: ./src/count_words.cc ./src/segmenter.h ./src/string_view.h ./src/word_counter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/count_words.cc

obj/count.o: ./src/count.cc gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/count.cc

obj/word_counter.o: ./src/word_counter.cc ./src/word_counter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/word_counter.cc

obj/string_view.o: ./src/string_view.cc ./src/string_view.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/string_view.cc

obj/segmenter.o: ./src/segmenter.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/segmenter.cc

obj/word_counter_test.o: ./src/word_counter_test.cc ./src/word_counter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/word_counter_test.cc

obj/segmenter_test.o: ./src/segmenter_test.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -g -std=c++11 -Igen -o $@ -c ./src/segmenter_test.cc

