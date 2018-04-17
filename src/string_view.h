#ifndef __STRING_VIEW_H__
#define __STRING_VIEW_H__

#include <cstring>
#include <iostream>
#include <string>

// As close as possible to C++ 17 string_view.
class string_view {
 public:
  string_view(const std::string &s) : data_(s.data()), size_(s.size()) {}
  string_view(const char *d, size_t s) : data_(d), size_(s) {}
  string_view(const char *d) : data_(d), size_(strlen(d)) {}

  const char &operator[](size_t i) const { return data_[i]; }
  size_t size() const { return size_; }

  bool operator==(const string_view &other) const {
    if (size_ != other.size_) {
      return false;
    }
    return memcmp(data_, other.data_, size_) == 0;
  }

  bool operator!=(const string_view &other) const { return (*this == other); }

  const std::string string() const { return std::string(data_, size_); }

 private:
  const char *data_;
  size_t size_;
};

std::ostream &operator<<(std::ostream &os, const string_view &s);

#endif  // __STRING_VIEW_H__
