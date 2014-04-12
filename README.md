Namebo
======

Some utilities for generating random English-like words.

To install dependencies on Mac OS X:

    brew install glog
    brew install gflags
    brew install protobuf
    brew install leveldb

To build the binaries:

    make

To build the prefix database:

    ./bin/count --input=data/words --output=data/map

To generate a random word:

    ./bin/generate --prefix_db=data/map

