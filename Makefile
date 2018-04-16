all: bin/count bin/segmenter_test bin/generate bin/dump bin/convert

clean:
	rm -rf bin || true
	rm -rf obj || true
	rm -rf gen || true

format:
	clang-format-3.8 -i src/*

.PRECIOUS: obj/%.o

obj/segmenter_test.o: ./src/segmenter_test.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/segmenter_test.cc

bin/count: obj/count.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

bin/segmenter_test: obj/segmenter_test.o obj/segmenter.o obj/string_view.o
	mkdir -p bin && g++ -o $@ $^ -lgtest -lpthread -lgtest_main

obj/count.o: ./src/count.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/count.cc

bin/generate: obj/generate.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/convert.o: ./src/convert.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/convert.cc

obj/dump.o: ./src/dump.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/dump.cc

bin/dump: obj/dump.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/namebo.o: gen/namebo.pb.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c gen/namebo.pb.cc

gen/namebo.pb.cc gen/namebo.pb.h: src/namebo.proto
	mkdir -p gen && protoc --proto_path=src --cpp_out=gen $^

obj/string_view.o: ./src/string_view.cc ./src/string_view.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/string_view.cc

bin/convert: obj/convert.o obj/namebo.o
	mkdir -p bin && g++ -o $@ $^ -lprotobuf -lgflags -lglog -lleveldb

obj/segmenter.o: ./src/segmenter.cc ./src/segmenter.h ./src/string_view.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/segmenter.cc

obj/generate.o: ./src/generate.cc gen/namebo.pb.h
	mkdir -p obj && g++ -std=c++11 -Igen -o $@ -c ./src/generate.cc

