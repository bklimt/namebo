
PROTOC=$(shell brew --prefix protobuf)/bin/protoc

LIBS=\
  -lglog -L$(shell brew --prefix glog)/lib \
  -lgflags -L$(shell brew --prefix gflags)/lib \
  -lleveldb -L$(shell brew --prefix leveldb)/lib \
  -lprotobuf -L$(shell brew --prefix protobuf)/lib

INCLUDES=\
  -Igen \
  -I$(shell brew --prefix glog)/include \
  -I$(shell brew --prefix gflags)/include \
  -I$(shell brew --prefix leveldb)/include \
  -I$(shell brew --prefix protobuf)/include

all: bin/count bin/dump bin/generate

clean:
	rm -rf bin || true
	rm -rf obj || true
	rm -rf gen || true

bin/%: obj/%.o obj/namebo.pb.o
	mkdir -p bin && g++ $(LIBS) -o $@ $^

obj/%.o: src/%.cc gen/namebo.pb.h
	mkdir -p obj && g++ $(INCLUDES) -o $@ -c $<

obj/namebo.pb.o: gen/namebo.pb.cc
	mkdir -p obj && g++ $(INCLUDES) -o $@ -c $<

gen/namebo.pb.cc gen/namebo.pb.h: src/namebo.proto
	mkdir -p gen && $(PROTOC) --proto_path=src --cpp_out=gen src/namebo.proto

