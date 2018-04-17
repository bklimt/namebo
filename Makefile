all: bin/convert bin/segmenter_test bin/dump bin/generate bin/count

clean:
	rm -rf bin || true
	rm -rf obj || true
	rm -rf gen || true

format:
	clang-format-3.8 -i src/*

.PRECIOUS: obj/%.o

obj/segmenter.o: ./src/segmenter.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/segmenter.cc

bin/convert: obj/convert.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

bin/segmenter_test: obj/segmenter_test.o obj/segmenter.o obj/string_view.o
	mkdir -p bin && g++ -o $@ $^ -lgtest -lpthread -lgtest_main

obj/namebo.o: gen/namebo.pb.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c gen/namebo.pb.cc

obj/segmenter_test.o: ./src/segmenter_test.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/segmenter_test.cc

gen/namebo.pb.cc gen/namebo.pb.h: src/namebo.proto
	mkdir -p gen && protoc --proto_path=src --cpp_out=gen $^

bin/dump: obj/dump.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/generate.o: ./src/generate.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/generate.cc

obj/convert.o: ./src/convert.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/convert.cc

obj/string_view.o: ./src/string_view.cc ./src/string_view.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/string_view.cc

obj/count.o: ./src/count.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/count.cc

obj/dump.o: ./src/dump.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/dump.cc

obj/word_counter.o: ./src/word_counter.cc ./src/word_counter.h ./src/string_view.h gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/word_counter.cc

bin/generate: obj/generate.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

bin/count: obj/count.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

