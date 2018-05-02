Namebo
======

Some utilities for generating random English-like words.

To install dependencies on Mac OS X:

    brew install clang-format
    brew install glog
    brew install gflags
    brew install protobuf
    brew install leveldb
    brew install --HEAD https://gist.githubusercontent.com/Kronuz/96ac10fbd8472eb1e7566d740c4034f8/raw/gtest.rb

To install dependencies on Ubuntu 16:

    sudo apt-get install libgflags-dev libgoogle-glog-dev \
         libprotobuf-dev libleveldb-dev protobuf-compiler \
         libgtest-dev clang-format-3.8

Build [gtest](https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/).

To build the binaries:

    python3 build.py >Makefile
    make

To run tests:

    ./run_tests.sh

To build the prefix database:

    ./bin/count --input=data/words --output=data/map

To generate a random word:

    ./bin/generate --prefix_db=data/map

