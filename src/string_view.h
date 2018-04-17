#ifndef __STRING_VIEW_H__
#define __STRING_VIEW_H__

#include <leveldb/db.h>
#include <cstring>
#include <iostream>
#include <string>

// leveldb::Slice is very similar to C++17 string_view.
typedef leveldb::Slice string_view;

std::ostream &operator<<(std::ostream &os, const string_view &s);

#endif  // __STRING_VIEW_H__
